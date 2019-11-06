#pragma once

#include <chrono>
#include <thread>

namespace NVMMgr_ns {

static const int GC_NODE_COUNT_THREADHOLD = 1024;

static const int GC_INTERVAL = 50;

int epoch;

class GarbageNode {
  public:
    // The epoch that this node is unlinked
    // This do not have to be exact - just make sure it is no earlier than the
    // actual epoch it is unlinked from the data structure
    uint64_t delete_epoch;
    void *node_p;
    GarbageNode *next_p;

    /*
     * Constructor
     */
    GarbageNode(uint64_t p_delete_epoch, void *p_node_p)
        : delete_epoch{p_delete_epoch}, node_p{p_node_p}, next_p{nullptr} {}

    GarbageNode() : delete_epoch{0UL}, node_p{nullptr}, next_p{nullptr} {}
} __attribute__((aligned(64)));

class GCMetaData {
  public:
    // This is the last active epoch counter; all garbages before this counter
    // are guaranteed to be not being used by this thread
    // So if we take a global minimum of this value, that minimum could be
    // be used as the global epoch value to decide whether a garbage node could
    // be recycled
    uint64_t last_active_epoch;

    GarbageNode header;

    // This points to the last node in the garbage node linked list
    // We always append new nodes to this pointer, and thus inside one
    // node's context these garbage nodes are always sorted, from low
    // epoch to high epoch. This facilitates memory reclaimation since we
    // just start from the lowest epoch garbage and traverse the linked list
    // until we see an epoch >= GC epoch
    GarbageNode *last_p;

    // The number of nodes inside this GC context
    // We use this as a threshold to trigger GC
    int node_count;

    GCMetaData() {
        last_active_epoch = static_cast<uint64_t>(-1);
        last_p = &header;
        node_count = 0;
    }
    ~GCMetaData() {}
} __attribute__((aligned(64)));

class Epoch_Mgr {
  public:
    bool exited_flag;
    std::thread *thread_p;

  public:
    Epoch_Mgr() {
        epoch = 0;
        exited_flag = false;
    }

    ~Epoch_Mgr() {}

    inline void IncreaseEpoch() { epoch++; }

    /*
     * ThreadFunc() - The cleaner thread executes this every GC_INTERVAL ms
     *
     * This function exits when exit flag is set to true
     */
    void ThreadFunc() {
        // While the parent is still running
        // We do not worry about race condition here
        // since even if we missed one we could always
        // hit the correct value on next try
        while (exited_flag == false) {
            // printf("Start new epoch cycle\n");
            IncreaseEpoch();

            // Sleep for 50 ms
            std::chrono::milliseconds duration(GC_INTERVAL);
            std::this_thread::sleep_for(duration);
        }
        return;
    }

    /*
     * StartThread() - Start cleaner thread for garbage collection
     *
     * NOTE: This is not called in the constructor, and needs to be
     * called manually
     */
    void StartThread() {
        thread_p = new std::thread{[this]() { this->ThreadFunc(); }};

        return;
    }
} __attribute__((aligned(64)));

inline uint64_t GetGlobalEpoch() { return epoch; }
} // namespace NVMMgr_ns
