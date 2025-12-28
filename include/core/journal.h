#pragma once

#include "frame.h"
#include "common.h"
#include <atomic>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace quote {
namespace core {

template <uint32_t Capacity>
struct JournalQueue {
    alignas(64) std::atomic<uint64_t> write_idx{0};
    Frame buffer[Capacity];

    static constexpr uint32_t mask = Capacity - 1;
};

class Journal {
public:
    static constexpr uint32_t DEFAULT_CAPACITY = 65536; // 2^16

    Journal(const std::string& name, bool is_writer) 
        : name_(name), is_writer_(is_writer) {
        int shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        size_t size = sizeof(JournalQueue<DEFAULT_CAPACITY>);
        ftruncate(shm_fd, size);
        ptr_ = static_cast<JournalQueue<DEFAULT_CAPACITY>*>(
            mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
        );
        if (is_writer_) {
            ptr_->write_idx.store(0, std::memory_order_relaxed);
        }
    }

    void write(MsgType type, uint32_t source_id, nano_t gen_time, const void* data, size_t len) {
        uint64_t idx = ptr_->write_idx.load(std::memory_order_relaxed);
        Frame& frame = ptr_->buffer[idx & JournalQueue<DEFAULT_CAPACITY>::mask];
        
        frame.header.source_id = source_id;
        frame.header.msg_type = type;
        frame.header.gen_time = gen_time;
        frame.header.push_time = now_nano();
        frame.header.data_len = static_cast<uint32_t>(len);
        
        std::memcpy(frame.payload, data, len);
        
        ptr_->write_idx.store(idx + 1, std::memory_order_release);
    }

    uint64_t get_write_idx() const {
        return ptr_->write_idx.load(std::memory_order_acquire);
    }

    const Frame& read(uint64_t idx) const {
        return ptr_->buffer[idx & JournalQueue<DEFAULT_CAPACITY>::mask];
    }

    static void remove(const std::string& name) {
        shm_unlink(name.c_str());
    }

private:
    std::string name_;
    bool is_writer_;
    JournalQueue<DEFAULT_CAPACITY>* ptr_;
};

} // namespace core
} // namespace quote
