//! This module implements the executor module

#include "gbox/runtime/executor/channel.hpp"

using namespace channel;

template <typename Sig>
super::Result<void> ExecutionChannel<Sig>::schedule_impl(Task task) {}
