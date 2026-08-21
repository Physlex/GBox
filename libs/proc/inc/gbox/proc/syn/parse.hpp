#ifndef GBOX_MACROS_SYN_PARSE_HPP_
#define GBOX_MACROS_SYN_PARSE_HPP_

/**
 * @brief This module implements the gbox proc macro parser, used to convert raw
 *        clang lexed tokens into a simple proc macro IR.
 */

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Token.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBufferRef.h>

#include <utility>

#include "gbox/proc/syn/tokens.hpp"

namespace gbox::parse {

/// @brief Plain old data type, aggregating clang context specifics
struct ClangCtx {
    std::vector<clang::Token> raw_tokens;
    clang::SourceManager &sm;
    const clang::ast_matchers::MatchFinder::MatchResult &res;
    clang::DiagnosticsEngine &diagnostics;
};

struct FileInfo {
    clang::FileID id;
    llvm::MemoryBufferRef buffer;
    size_t start;
    size_t length;
};

/// Implements the parsing interface for converting clang tokens to proc macro IR.
class Parser {
  public:
    Parser(ClangCtx clang, FileInfo file) : clang_(std::move(clang)), file_(file) {}

    /// @brief Parsing implementation for the proc macro AST.
    bool parse(tokens::TokenStream &in) {
        const size_t raw_tokens_cnt = this->clang_.raw_tokens.size();
        while ((this->idx_ < raw_tokens_cnt) && this->parse_aux(in)) {
            ;
        }
        return this->idx_ >= raw_tokens_cnt;
    }

  private:
    // TODO: DOCS
    bool parse(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef symbol,
        tokens::Span span
    );

    /**
     *  @defgroup parsing_steps Parsing Steps
     *  @brief Parsing step for literals, converts a clang token into a token tree and
     * appends it to the stream.
     *  {@
     */

    bool parse_literals(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef symbol,
        tokens::Span span
    );

    bool parse_identifiers(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef symbol,
        tokens::Span span
    );

    bool parse_numerics(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef symbol,
        tokens::Span span
    );

    bool parse_group(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef slice,
        tokens::Span span
    );

    bool parse_punctuation(
        tokens::TokenStream &in, clang::Token tok, llvm::StringRef symbol,
        tokens::Span span
    );

    bool parse_aux(tokens::TokenStream &in) {
        const clang::Token curr_tok = this->clang_.raw_tokens.at(this->idx_);

        const uint32_t tok_start = this->clang_.sm.getFileOffset(curr_tok.getLocation());
        const uint32_t tok_length = curr_tok.getLength();

        // tok_start is an absolute file offset; subtract the slice's start to get
        // a buffer-relative index into the scratch MemoryBufferRef.
        const char *text_start = this->file_.buffer.getBufferStart();
        const uint32_t buf_offset = tok_start - this->file_.start;

        const auto span = tokens::Span(tok_start, tok_length);
        const auto symbol = llvm::StringRef(text_start + buf_offset, span.length());

        return parse(in, curr_tok, symbol, span);
    }

    /** @} */

    ClangCtx clang_;
    FileInfo file_;
    size_t idx_{0};
};

}  // namespace gbox::parse

#endif  // GBOX_MACROS_SYN_PARSE_HPP_
