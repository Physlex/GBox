module;

export module gbox.core:mixins;

//! This module composes utilities for more ergonomic mixin integration with C++
//!
//! Both models inherit the constructors of the base type. Each constructor applies
//! the policy requirements to the underlying class in kind, reducing boilerplate for
//! mixin usage. (no more public A, public B, public C..., etc)
//!
//! For policies in particular, we no longer need to specify a million class Derived
//! types, see the [`Policies`] constructor for more details.
//!
//! The inheritance constructors can be used together, in which case an implementor will
//! add each one as it's own public inheritance.
//!
//! The inheritance constructors can also be used with regular inheritence. They simply
//! remove boilerplate, and don't touch any other parts of the program.

/// Defines a constructor used to simplify mixin aggregation across a classes inheritance
/// params
///
/// ## Example
/// ```C++
/// // For mixins: Into, Map, and MoveOnly:
/// class A: public MixIn<Into<A>, Map<A>, MoveOnly> {
///     // ...etc
/// };
/// ```
template <typename... BasicMixIns>
struct MixIn : public BasicMixIns... {};

/// Defines a constructor used to simplify policy aggregation across a classes inheritance
/// params
///
/// ## Example
/// ```C++
/// class A: public Policies<A, Map, Into> {
///     // ... etc
/// };
/// ```
template <class Derived, template <typename> class... Policy>
struct Policies : public Policy<Derived>... {};
