#ifndef GBOX_CELL_TEST_PROBE_HPP_
#define GBOX_CELL_TEST_PROBE_HPP_

//! This file implements a value which reports its own destruction, so that a test can
//! assert a cell never destroys what it holds.

/// Counts the destructions performed against a value which still owns one
///
/// A moved-from probe hands its ownership to the probe it was moved into, so the count
/// only ever names values that were still live when they were destroyed.
struct Probe {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static inline int destroyed = 0;

    int value;
    bool owner = true;

    explicit Probe(int v) : value(v) {}

    Probe(Probe &&other) noexcept : value(other.value) { other.owner = false; }

    Probe &operator=(Probe &&other) noexcept {
        this->value = other.value;
        this->owner = other.owner;
        other.owner = false;

        return *this;
    }

    Probe(const Probe &) = delete;
    Probe &operator=(const Probe &) = delete;

    ~Probe() {
        if (this->owner) {
            Probe::destroyed += 1;
        }
    }
};

#endif  // GBOX_CELL_TEST_PROBE_HPP_
