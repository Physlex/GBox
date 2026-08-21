/**
 * @brief This file implements the adapter module for clang
 */

#include "gbox/proc/action.hpp"

#include <memory>

#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendOptions.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace gbox::action;
using namespace result;
Action::Action(clang::DiagnosticsEngine &dengine) : dengine_(dengine) {}

gbox::action::Result<std::unordered_map<std::string, std::string> > Action::execute(
    const std::vector<const char *> &args
) {
    auto invocation = std::make_shared<clang::CompilerInvocation>();
    if (!clang::CompilerInvocation::CreateFromArgs(*invocation, args, this->dengine_)) {
        return Err(ErrorKind::InvalidArgs);
    }

    // Run as AST-only; the driver handles the actual object emission separately.
    invocation->getFrontendOpts().ProgramAction = clang::frontend::ParseSyntaxOnly;

    auto instance = clang::CompilerInstance(invocation);
    instance.createDiagnostics(*llvm::vfs::getRealFileSystem());

    if (!instance.ExecuteAction(this->action_) ||
        instance.getDiagnostics().hasErrorOccurred()) {
        return Err(ErrorKind::Action);
    }

    return Ok(this->action_.getRewritten());
}
