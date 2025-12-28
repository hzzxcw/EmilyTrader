#pragma once

#include <iostream>
#include <atomic>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace quote {

// 使用 PAUSE 指令优化忙等
inline void cpu_pause() {
#if defined(__x86_64__) || defined(_M_X64)
    _mm_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ __volatile__("yield" ::: "memory");
#endif
}

template <typename T>
struct Slot {
    // 序列号用于生产者和消费者的同步
    // 生产者等待 sequence == pos
    // 生产者写完后 sequence = pos + 1
    // 消费者等待 sequence == pos + 1
    alignas(64) std::atomic<uint64_t> sequence;
    T data;
};

template <typename T, uint32_t Capacity>
struct RingBuffer {
    // 全局写索引，多个生产者通过 fetch_add 抢占
    alignas(64) std::atomic<uint64_t> write_idx{0};
    
    // 数据槽位
    Slot<T> buffer[Capacity];

    static constexpr uint32_t mask = Capacity - 1;

    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

    // 初始化槽位序列号，这对于 MPMC 正确性至关重要
    void init() {
        for (uint32_t i = 0; i < Capacity; ++i) {
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
        write_idx.store(0, std::memory_order_relaxed);
    }

    // 多生产者安全的推送函数
    void push(const T& item) {
        // 1. 抢占槽位
        uint64_t pos = write_idx.fetch_add(1, std::memory_order_relaxed);
        Slot<T>& slot = buffer[pos & mask];

        // 2. 忙等直到槽位可写 (即槽位的 sequence 等于当前轮次的 pos)
        // 这确保了：
        // a) 多个生产者按序写入同一位置
        // b) 生产者不会覆盖还没转完一圈的上一轮数据
        while (slot.sequence.load(std::memory_order_acquire) != pos) {
            cpu_pause();
        }

        // 3. 写入数据
        slot.data = item;

        // 4. 更新序列号，通知消费者数据已就绪
        // 使用 release 确保上面的数据写入对消费者可见
        slot.sequence.store(pos + 1, std::memory_order_release);
    }

    // 消费者读取函数（支持多个独立消费者）
    const T& wait_and_get(uint64_t read_idx) {
        Slot<T>& slot = buffer[read_idx & mask];
        // 忙等直到 sequence == read_idx + 1，表示该位置已被第 (read_idx/Capacity) 轮生产者写完
        while (slot.sequence.load(std::memory_order_acquire) != read_idx + 1) {
            cpu_pause();
        }
        return slot.data;
    }
};

class ShmManager {
public:
    template <typename T, uint32_t Capacity>
    static RingBuffer<T, Capacity>* create(const std::string& name) {
        // 移除旧的 SHM 以确保干净的初始状态
        shm_unlink(name.c_str());

        int shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open");
            return nullptr;
        }

        size_t size = sizeof(RingBuffer<T, Capacity>);
        if (ftruncate(shm_fd, size) == -1) {
            perror("ftruncate");
            return nullptr;
        }

        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            return nullptr;
        }

        auto rb = static_cast<RingBuffer<T, Capacity>*>(ptr);
        rb->init(); // 初始化序列号
        return rb;
    }

    template <typename T, uint32_t Capacity>
    static RingBuffer<T, Capacity>* attach(const std::string& name) {
        int shm_fd = shm_open(name.c_str(), O_RDWR, 0666);
        if (shm_fd == -1) {
            return nullptr;
        }

        size_t size = sizeof(RingBuffer<T, Capacity>);
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            return nullptr;
        }

        return static_cast<RingBuffer<T, Capacity>*>(ptr);
    }

    static void remove(const std::string& name) {
        shm_unlink(name.c_str());
    }
};

} // namespace quote
