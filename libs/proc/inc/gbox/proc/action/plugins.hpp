#ifndef GBOX_MACROS_PLUGINS_HPP_
#define GBOX_MACROS_PLUGINS_HPP_

/** @file `plugins.hpp`
 *  @brief TODO: DOCS
 */

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/ADT/RewriteBuffer.h>
#include <llvm/ADT/StringRef.h>

#include <memory>
#include <unordered_map>

namespace gbox::plugins {

/// Boilerplate to handle clang
class HandleFuncDecl : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    HandleFuncDecl(clang::Rewriter &rewriter) : rewriter_(rewriter) {}

    //! @brief TODO: DOCS
    void run(const clang::ast_matchers::MatchFinder::MatchResult &res) override;

  private:
    clang::Rewriter &rewriter_;
};

/// This method implements the "frontend" logic for parsing each t-unit
class ProcMacroConsumer : public clang::ASTConsumer {
  public:
    ProcMacroConsumer(clang::Rewriter &rewriter) : rewriter_(rewriter) {}

    /// Override to handle each translation unit according to the consumer.
    void HandleTranslationUnit(clang::ASTContext &ctx) override;

  private:
    clang::Rewriter &rewriter_;
};

/// This class really just exists as the hook-up boilerplate for the sake of the clang
/// compilation process.
class ProcMacroAction : public clang::ASTFrontendAction {
  public:
    ProcMacroAction() = default;
    virtual ~ProcMacroAction() = default;

    /// Public constructor
    inline std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &ci, llvm::StringRef infile
    ) override {
        this->infile_ = infile;
        rewriter_.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
        return std::make_unique<ProcMacroConsumer>(rewriter_);
    }

    inline std::unordered_map<std::string, std::string> getRewritten() const {
        return this->rewritten_;
    }

    /// Callback before any given translation unit begins processing
    ///
    /// Ensures that the frontend action is called iff the main action expected
    /// from the compiler invocation is some form of emitter.
    bool BeginSourceFileAction(clang::CompilerInstance &ci) override;

    /// Invoked after each translation unit is finished processing.
    void EndSourceFileAction() override;

  private:
    clang::Rewriter rewriter_;
    std::string infile_;
    std::unordered_map<std::string, std::string> rewritten_;
};

}  // namespace gbox::plugins

#endif  // GBOX_MACROS_PLUGINS_HPP_
