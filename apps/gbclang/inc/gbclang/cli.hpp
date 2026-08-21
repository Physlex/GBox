#ifndef GBOX_MACROS_DRIVER_HPP_
#define GBOX_MACROS_DRIVER_HPP_

//! This module defines the combined command line utilities for
//! converting from gbclang to general-purpose clang.

#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <string>

#include "gbox/core/result.hpp"

using namespace result;
namespace gbclang::cli {

/// Various error defintions used in the CLI interface.
enum class CliErrorKind : uint8_t {
    /// An argument provided to the CLI was incorrectly parsed
    InvalidArgs,

    /// An IO error occured during an OS read/write operation
    Io,
};

/// Result alias for Cli results
template <typename T>
using Result = Result<T, CliErrorKind>;

/// Remove the "gb" prefix from our clang driver path.
extern Result<std::string> stripGBFromPath(std::string path);

/// Extract the "true" clang path from the clang driver name.
extern Result<std::string> clangPathFromName(std::string path);

// TODO: DOCS
Result<std::string> writeVirtualFile(
    const std::string &input_file, const std::string &rewritten_content,
    const std::string &output_dir
);

}  // namespace gbclang::cli

#endif  // GBOX_MACROS_DRIVER_HPP_
