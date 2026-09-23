#ifndef GBOX_MACROS_ACTION_HPP_
#define GBOX_MACROS_ACTION_HPP_

// TODO: DOCS

#include <clang/Basic/Diagnostic.h>

#include <string>
#include <unordered_map>

import gbox.funky;
// TODO: Probably should condense this into the current module instead of keeping seperate
#include "gbox/proc/action/plugins.hpp"

namespace gbox::action {

/**
 *  @brief This enumerator defines the error kinds avaliable for a given action
 */
enum class ErrorKind : uint8_t { InvalidArgs, Action };

/// Adaptor result alias. Uses the gbox::adaptor::ErrorKind as it's errorfull value.
template <typename T>
using Result = result::Result<T, ErrorKind>;

/// This class implements the action adaptor type
///
/// The action adaptor converts a minimal slice of clang input arguments into
/// the related clang frontend action framework.
class Action {
  public:
    /// @brief Constructor for the action
    Action(clang::DiagnosticsEngine &dengine);

    /**
     *  @brief Provided the given arguments, execute the action.
     */
    Result<std::unordered_map<std::string, std::string>> execute(
        const std::vector<const char *> &args
    );

  private:
    plugins::ProcMacroAction action_;
    clang::DiagnosticsEngine &dengine_;
};

}  // namespace gbox::action

#endif  // GBOX_MACROS_ACTION_HPP_
