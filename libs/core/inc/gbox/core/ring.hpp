#ifndef GBOX_CORE_RING_HPP_
#define GBOX_CORE_RING_HPP_

//! This file implements a simple byte-level ring buffer
//!
//! It allows queueing/dequeuing any memory type by copying it's
//! byte-level representation.
//!
//! The container uses a malloc'd implementation, as in the initial
//! construction of the ring uses a memory allocation and hence, syscall.
//!
//! To reduce overhead, the ring buffer is treated statically, meaning
//! unless a second ring buffer is defined, the ring buffer provided has
//! no built-in capability to change it's size.

namespace ring {
#include "ring/mod.hpp"
#include "ring/owned.hpp"
#include "ring/viewed.hpp"

template <typename T, std::size_t C>
using RingBuffer = owned::OwnedStorage<T, C>;

template <typename T>
using RingView = viewed::ViewedStorage<T>;
}  // namespace ring

#endif  // GBOX_CORE_RING_HPP_
