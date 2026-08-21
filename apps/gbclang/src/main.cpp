//! TODO: Comprehensive docs on usage, invocation, and design process.

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <stdint.h>
#include <unistd.h>

#include <cstdlib>
#include <memory>
#include <vector>

#include "gbclang/cli.hpp"
#include "gbox/core/result.hpp"
#include "gbox/proc/action.hpp"

using namespace gbox;
using namespace gbclang;
using namespace result;
/// Typed response for the `configure_diag` method.
struct ConfigureDiagRes {
    /// Diagnostic options — must outlive the engine
    std::unique_ptr<clang::DiagnosticOptions> diag_opts;

    /// Fully configured diagnostic engine
    llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diag;

    /// Path to be used by the clang driver, stripped of gb influence
    std::string path;
};

/// Configure the clang diagnostics system to gracefully handle gbclang invocations
Result<ConfigureDiagRes, int> build_diag(std::vector<const char *> &args) {
    auto diag_opts = clang::CreateAndPopulateDiagOpts(args);

    auto diag_client =
        std::make_unique<clang::TextDiagnosticPrinter>(llvm::errs(), *diag_opts);

    auto strip_res = cli::stripGBFromPath(args[0]);
    if (strip_res.is_err()) {
        return Err(1);
    }

    auto stripped_pathname = strip_res.assume_ok();

    auto clean_res = cli::clangPathFromName(stripped_pathname);
    if (clean_res.is_err()) {
        return Err(1);
    }

    auto clean_pathname = clean_res.assume_ok();

    auto exe_basename = llvm::StringRef(llvm::sys::path::stem(clean_pathname));
    if (exe_basename.equals_insensitive("cl")) {
        exe_basename = "clang-cl";
    }

    diag_client->setPrefix(std::string(exe_basename));

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_id(new clang::DiagnosticIDs());

    llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diag(
        new clang::DiagnosticsEngine(diag_id, *diag_opts, diag_client.release())
    );

    return Ok(ConfigureDiagRes{std::move(diag_opts), diag, clean_pathname});
}

int32_t main(int argc, const char **argv) {
    if (argc == 1) {
        printf("%s: error: no input files\n", argv[0]);
        return 1;
    }

    auto args_vec = std::vector<const char *>(argv, argv + argc);

    auto diag_config_res = build_diag(args_vec);
    if (diag_config_res.is_err()) {
        return diag_config_res.assume_err();
    }

    auto [diag_opts, diag, clean_path] = diag_config_res.assume_ok();
    auto driver =
        clang::driver::Driver(clean_path, llvm::sys::getDefaultTargetTriple(), *diag);
    auto target_and_mode =
        clang::driver::ToolChain::getTargetAndModeFromProgramName(clean_path);
    driver.setTargetAndMode(target_and_mode);

    auto *compilation = driver.BuildCompilation(args_vec);
    if ((compilation == nullptr) || diag->hasErrorOccurred()) {
        return 1;
    }

    for (const auto &job : compilation->getJobs()) {
        const auto *cmd = llvm::dyn_cast<clang::driver::Command>(&job);
        if (cmd == nullptr) {
            continue;
        }

        const llvm::opt::ArgStringList &args = cmd->getArguments();
        if (!llvm::is_contained(args, llvm::StringRef("-emit-obj"))) {
            continue;
        }

        const char *const *begin = args.data() + 1;
        const char *const *end = begin + args.size() - 1;
        auto args_slice = std::vector<const char *>(begin, end);

        // TODO: Refactor the string into a map of files rewritten -> rewritten code
        auto action_exec_res = gbox::action::Action(*diag).execute(args_slice);
        if (action_exec_res.is_err()) {
            return 1;
        }

        llvm::outs() << "TEST, ABOUT TO PRINT KEY, VALUE FOR REWRITTEN:\n";
        auto rewritten = action_exec_res.assume_ok();
        for (const auto &[key, value] : rewritten) {
            llvm::outs() << key << ": " << value << "\n";
        }
    }

    llvm::SmallVector<llvm::StringRef> args(argv, argv + argc);
    return llvm::sys::ExecuteAndWait(clean_path, args);
}
