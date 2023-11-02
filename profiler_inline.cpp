#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using f32 = float;
using f64 = double;

constexpr u32 PROFILER_MAX_CHECKPOINTS = 4096;

// ========== public interface ==========

void BeginProfiling();
void FinishProfiling();
void PrintProfilingResults();

#define PROFILE_BLOCK_INNER2(name, count) ProfileBlock pb##count(name, count)
#define PROFILE_BLOCK_INNER1(name, count) PROFILE_BLOCK_INNER2(name, count)
#define PROFILE_BLOCK(name) PROFILE_BLOCK_INNER1(name, __COUNTER__)

struct ProfileCheckpoint {
    u64 start;
    u64 exclusiveTime;
    u64 inclusiveTime;
    u64 childrenTime;
    u64 hitCount;
    const char* label;
};

struct ProfileBlock {
    ProfileBlock(const char* label, u32 blockIdx);
    ~ProfileBlock();

    u64 rootElapsed;
    u64 rootStart;
    ProfileCheckpoint* parentCheckpoint;
};

f64 EstimateRDTSCFrequencyNS();

struct Profiler {
    ProfileCheckpoint checkpoints[PROFILER_MAX_CHECKPOINTS];
    ProfileCheckpoint* currentCheckpoint;
    u64 startTimestamp;
    u64 endTimestamp;
};

Profiler globalProfiler;

ProfileBlock::ProfileBlock(const char* label, u32 blockIdx) {
    ProfileCheckpoint* checkpoint = &globalProfiler.checkpoints[blockIdx + 1];
    parentCheckpoint = globalProfiler.currentCheckpoint;

    checkpoint->label = label;
    ++checkpoint->hitCount;

    globalProfiler.currentCheckpoint = checkpoint;

    u64 timestamp = __rdtsc();
    parentCheckpoint->exclusiveTime += timestamp - parentCheckpoint->start;
    checkpoint->start = timestamp;
    rootElapsed = checkpoint->inclusiveTime;
    rootStart = timestamp;
}

ProfileBlock::~ProfileBlock() {
    ProfileCheckpoint* checkpoint = globalProfiler.currentCheckpoint;

    u64 timestamp = __rdtsc();
    u64 dt = timestamp - checkpoint->start;
    checkpoint->exclusiveTime += dt;
    checkpoint->inclusiveTime = rootElapsed + (timestamp - rootStart);
    parentCheckpoint->childrenTime += checkpoint->exclusiveTime + checkpoint->childrenTime;

    globalProfiler.currentCheckpoint = parentCheckpoint;
    globalProfiler.currentCheckpoint->start = timestamp;
}

f64 EstimateRDTSCFrequencyNS() {
    i64 measurement_period_ns = 100000000ul;

    u64 rdtsc_t0 = __rdtsc();
    timespec sleepTime = (timespec){0ul, measurement_period_ns};
    nanosleep(&sleepTime, nullptr);
    u64 rdtsc_t1 = __rdtsc();

    return (f64)(rdtsc_t1 - rdtsc_t0) / (f64)measurement_period_ns;
}

void BeginProfiling() {
    globalProfiler = {};
    globalProfiler.currentCheckpoint = &globalProfiler.checkpoints[0];
    globalProfiler.startTimestamp = __rdtsc();
}

void FinishProfiling() {
    globalProfiler.endTimestamp = __rdtsc();
    globalProfiler.checkpoints[0].exclusiveTime =
        (globalProfiler.endTimestamp - globalProfiler.startTimestamp) -
        globalProfiler.checkpoints[0].childrenTime;
}

void PrintProfilingResults() {
    u64 totalTime =
        (f64)(globalProfiler.endTimestamp - globalProfiler.startTimestamp);

    printf("==== Profile results ====\n");
    for (u32 i = 0; i < PROFILER_MAX_CHECKPOINTS; ++i) {
        ProfileCheckpoint* checkpoint = &globalProfiler.checkpoints[i];
        if (checkpoint->label == nullptr) continue;

        f64 blockTime =
            (f64)(checkpoint->inclusiveTime);

        printf(
            "%.2f%% total | %.2f%% exclusive - %s [hits: %lu]\n",
            blockTime / totalTime * 100, (f64)checkpoint->exclusiveTime / totalTime * 100, checkpoint->label,
            checkpoint->hitCount);
    }
    printf("==== End of profile results ====\n");
}
