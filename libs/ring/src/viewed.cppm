module;

#include <valarray>

export module gbox.ring:viewed;

import gbox.core;
import :policy;

export namespace ring::viewed {

/// Partial specialization of the RingStorage type into a slice into an existing ring
template <typename T>
class ViewedStorage : public memory::Pinned, public policy::Ring<ViewedStorage<T>, T> {
  public:
    // TODO: IMPLEMENT

  protected:
    std::slice_array<T> storage_;
};

}  // namespace ring::viewed
