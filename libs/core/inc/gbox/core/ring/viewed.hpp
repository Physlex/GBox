#ifndef GBOX_CORE_RING_VIEWED_HPP_
#define GBOX_CORE_RING_VIEWED_HPP_

#include <valarray>

#include "gbox/core/memory.hpp"
#include "gbox/core/ring/mod.hpp"

namespace viewed {
/// Partial specialization of the RingStorage type into a slice into an existing ring
template <typename T>
class ViewedStorage : public memory::Pinned, public super::Ring<ViewedStorage<T>, T> {
  public:
    // TODO: IMPLEMENT

  protected:
    std::slice_array<T> storage_;
};
}  // namespace viewed

#endif  // GBOX_CORE_RING_VIEWED_HPP_
