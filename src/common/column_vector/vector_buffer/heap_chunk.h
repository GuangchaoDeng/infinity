//
// Created by JinHai on 2022/11/30.
//

#pragma once

#include "common/memory/allocator.h"
#include "common/types/internal_types.h"

namespace infinity {

struct HeapChunk {
public:
    inline explicit
    HeapChunk(u64 capacity) : capacity_(capacity), current_offset_(0), object_count_(0) {
        GlobalResourceUsage::IncrObjectCount();
        ptr_ = Allocator::allocate(capacity);
    }

    inline
    ~HeapChunk() {
        Allocator::deallocate(ptr_);
        ptr_ = nullptr;
        capacity_ = 0;
        current_offset_ = 0;
        object_count_ = 0;
        GlobalResourceUsage::DecrObjectCount();
    }

    ptr_t ptr_{nullptr};
    u64 current_offset_{0};
    u64 capacity_{0};
    u64 object_count_{0};
};

struct StringHeapMgr {
    // Use to store string.
    static constexpr u64 CHUNK_SIZE = 4096;
public:
    inline explicit
    StringHeapMgr(u64 chunk_size = CHUNK_SIZE) : current_chunk_size_(chunk_size) {
        GlobalResourceUsage::IncrObjectCount();
    }

    inline
    ~StringHeapMgr() {
        GlobalResourceUsage::DecrObjectCount();
    }

    ptr_t
    Allocate(size_t nbytes);

    [[nodiscard]] String
    Stats() const;

public:
    [[nodiscard]] inline size_t
    chunks() const { return chunks_.size(); }

    [[nodiscard]] inline u64
    current_chunk_idx() const {
        return current_chunk_idx_;
    }

    [[nodiscard]] inline u64
    current_chunk_size() const {
        return current_chunk_size_;
    }
private:
    Vector<UniquePtr<HeapChunk>> chunks_;
    u64 current_chunk_size_{CHUNK_SIZE};
    u64 current_chunk_idx_{std::numeric_limits<u64>::max()};
};

}