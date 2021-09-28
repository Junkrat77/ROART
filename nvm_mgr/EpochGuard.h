#pragma once

#include "threadinfo.h"

namespace NVMMgr_ns {
class EpochGuard {
  public:
    EpochGuard() {/* printf("in EpochGuard\n"); */ JoinNewEpoch(); }
    ~EpochGuard() { LeaveThisEpoch(); }
    static void DeleteNode(void *node) { MarkNodeGarbage(node); }
};
} // namespace NVMMgr_ns
