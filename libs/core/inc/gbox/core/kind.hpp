//! This file implements a kinded-ness system for cpp built on simple union types
//!
//! The hope is that we can get something closer to the primitives necessary for
//! type-oriented programming, which gbox in general views as the next step in firmware.
//!
//! TODO: DOCS explaining how the variants are made

/// Function definition for recursive template pattern
///
/// defines how matching should work for a given set of template arms.
template <typename... Arms>
union Variants;

/// Base case
template <>
union Variants<> {};

/// Template generation "function" implementation
///
/// Implements the match-pattern which defines the way in which each non-base case
/// recursive step generates one section of the Variants type.
///
/// ## Usage
/// Used to define an arbitrary storage type fit within a given union for the `Kind`
/// type, and can be used to implement various substitutions to the std::variant type
/// in the STL, in particular, a clean "Option" and "Result" type.
template <typename Head, typename... Tail>
union Variants<Head, Tail...> {
  public:
    Head head;
    Variants<Tail...> tail;

    Variants() {}
    ~Variants() {}
};

class Kind {
  public:
    Kind() = default;

  private:
    union Variants;
}
