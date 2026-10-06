module;

//! This module composes utilities for more ergonomic mixin integration with C++
//!
//! A policy is written against [`Self`], a placeholder for the class it will be applied
//! to, so a policy never names its host. [`For`] supplies the host and gathers every
//! group into a single base. (no more public A, public B, public C..., etc)
//!
//! [`Where`] groups the policies a class is defined under. [`MixIn`] groups bases that
//! take no host of their own. Both may appear in the same [`For`], in any order, and a
//! group may be named ahead of time and reused across classes.

export module gbox.core.mixins;

/// Stands in for the class a policy is applied to.
export struct Self;

/// Groups the policies a class is defined under.
export template <typename... Policy>
struct Where;

/// Groups bases that take no host of their own.
export template <typename... Base>
struct MixIn;

/// Substitutes `Host` for [`Self`] in a policy, leaving anything else alone.
template <typename Host, typename Policy>
struct Applied {
    using type = Policy;
};

template <typename Host, template <typename...> class Policy, typename... Args>
struct Applied<Host, Policy<Self, Args...>> {
    using type = Policy<Host, Args...>;
};

/// The bases a single group contributes to `Host`.
template <typename Host, typename Group>
struct Expand;

template <typename Host, typename... Policy>
struct Expand<Host, Where<Policy...>> {
    struct type : Applied<Host, Policy>::type... {};
};

template <typename Host, typename... Base>
struct Expand<Host, MixIn<Base...>> {
    struct type : Base... {};
};

/// Defines a constructor used to simplify policy aggregation across a classes inheritance
/// params
///
/// ## Example
/// ```cpp
/// class A : public For<A, Where<MoveOnly<Self>, Map<Self, int32_t>>, MixIn<Tagged>> {
///     // ... etc
/// };
/// ```
export template <typename Host, typename... Group>
struct For : Expand<Host, Group>::type... {};
