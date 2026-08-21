/**
 * @file cli.cpp
 * @brief This implements command line parameters
 */

#include "gbclang/cli.hpp"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>

#include <gbox/core/types.hpp>

namespace gbclang::cli {

Result<std::string> stripGBFromPath(std::string path) {
    const auto gb_substr = std::string("gbclang");
    const auto gb_replace = std::string("clang");
    size_t pos = path.find(gb_substr);

    if (pos == std::string::npos) {
        return Err(CliErrorKind::InvalidArgs);
    }

    const auto clang_path = path.replace(pos, gb_substr.length(), gb_replace);
    return Ok(clang_path.substr(pos, gb_replace.length()));
}

Result<std::string> clangPathFromName(std::string executable_path) {
    if (!llvm::sys::fs::exists(executable_path)) {
        const auto path = llvm::sys::findProgramByName(executable_path);
        if (path) {
            executable_path = *path;
        }
    }

    return Ok(std::string(executable_path));
}

Result<std::string> writeVirtualFile(
    const std::string &input_file, const std::string &rewritten_content,
    const std::string &output_dir
) {
    const uint32_t VIRTUAL_PATH_LEN = 128;
    auto virtual_path = llvm::SmallString<VIRTUAL_PATH_LEN>(output_dir);
    llvm::sys::path::append(
        virtual_path, llvm::sys::path::stem(input_file) + ".gbox.cpp"
    );

    std::error_code ec;
    auto out = llvm::raw_fd_ostream(virtual_path.str(), ec);
    if (ec) {
        return Err(CliErrorKind::Io);
    }

    out << rewritten_content;

    return Ok(std::string(virtual_path));
}

}  // namespace gbclang::cli
