/** @file_id `plugins.c`
 *  @brief This file_id implements the gbox clang plugins for gbclang and gbclang++
 *         preprocessing macros.
 */

#include "gbox/proc/action/plugins.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/LiteralSupport.h>
#include <clang/Lex/Token.h>
#include <llvm/ADT/RewriteBuffer.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

#include <unordered_map>
#include <vector>

#include "clang/Basic/SourceManager.h"
#include "gbox/proc/syn/parse.hpp"
#include "gbox/proc/syn/tokens.hpp"

using namespace gbox::plugins;
using namespace gbox;

// TODO: Demonstrate full s2s translation in-memory using the system described prior.
tokens::TokenStream proc_macro_executor(tokens::TokenStream &input) {
    return tokens::TokenStream();
}

void HandleFuncDecl::run(const clang::ast_matchers::MatchFinder::MatchResult &res) {
    const clang::FunctionDecl *func_decl =
        res.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl");
    if (!func_decl) {
        return;
    }

    const clang::FunctionDecl *fdef = func_decl->getDefinition();
    if (!fdef) {
        return;
    }

    std::vector<llvm::StringRef> annotations;

    for (auto attr : fdef->getAttrs()) {
        const auto *annotated_attr = dyn_cast<clang::AnnotateAttr>(attr);
        if (annotated_attr) {
            annotations.push_back(annotated_attr->getAnnotation());
        }
    }

    // Skip functions that have no annotations
    if (annotations.size() == 0) {
        return;
    }

    clang::ASTContext *ctx = res.Context;
    clang::SourceManager &sm = ctx->getSourceManager();

    clang::SourceRange src_range = fdef->getSourceRange();
    clang::SourceLocation src_begin = src_range.getBegin();
    clang::SourceLocation src_end = src_range.getEnd();

    auto file_id = sm.getFileID(src_begin);
    unsigned start = sm.getFileOffset(src_begin);
    unsigned end = sm.getFileOffset(src_end);

    // Lexing stage

    const size_t length = end - start + 1;
    std::string slice = sm.getBufferData(file_id).substr(start, length).str();
    auto buffer = llvm::MemoryBufferRef(slice, "<scratch>");

    // Anchor the scratch lex at src_begin so every token's SourceLocation maps
    // back into the real SourceManager (required for Rewriter::ReplaceText).
    const char *buf_start = slice.data();
    const char *buf_end = buf_start + slice.size();
    clang::Token tok;
    std::vector<clang::Token> tokens;
    auto lexer = clang::Lexer(
        src_begin, res.Context->getLangOpts(), buf_start, buf_start, buf_end
    );
    while (!lexer.LexFromRawLexer(tok)) {
        tokens.push_back(tok);
    }

    if (!tok.is(clang::tok::eof)) {
        tokens.push_back(tok);
    }

    // Parsing stage

    parse::Parser parser = parse::Parser(
        parse::ClangCtx{tokens, sm, res, res.Context->getDiagnostics()},
        parse::FileInfo{file_id, buffer, start, length}
    );

    tokens::TokenStream stream;
    if (!parser.parse(stream)) {
        llvm::outs() << "Failed to parse tokens into tokens::TokenStream\n";
        return;
    }

    // Invoke tokens::TokenStream proc macro plugins

    // FIXME: Exactly what's on the tin
    // for (size_t idx = 0; idx < annotations.size(); idx += 1) {
    //     if (annotations[idx] == "executor") {
    //         proc_macro_executor(stream);
    //     }
    // }

    // Match against each token, and generate C++ code from them

    std::string code = stream.toString();
    std::string test_code =
        "static inline int32_t hello_msg(void *args) { printf(\"TEST: It Worked!\"); }";
    const clang::SourceRange range = fdef->getSourceRange();
    this->rewriter_.ReplaceText(range, test_code);
}

void ProcMacroConsumer::HandleTranslationUnit(clang::ASTContext &ctx) {
    HandleFuncDecl handler(this->rewriter_);
    clang::ast_matchers::MatchFinder finder;

    // Match against function definitions
    finder.addMatcher(clang::ast_matchers::functionDecl().bind("funcDecl"), &handler);
    finder.matchAST(ctx);
}

bool ProcMacroAction::BeginSourceFileAction(clang::CompilerInstance &ci) {
    // We always run as ParseSyntaxOnly — object emission is delegated to the
    // real clang invocation.  Accept any program action.
    return true;
}

void ProcMacroAction::EndSourceFileAction() {
    auto file_id = getCompilerInstance().getSourceManager().getMainFileID();
    const llvm::RewriteBuffer *buf = this->rewriter_.getRewriteBufferFor(file_id);
    if (buf) {
        this->rewritten_[std::string(this->infile_)] =
            std::string(buf->begin(), buf->end());
    }
}
