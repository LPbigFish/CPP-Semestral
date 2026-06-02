#pragma once

#include "core/BlockHeader.hpp"
#include "core/Sha256.hpp"
#include "engine/CpuEngine.hpp"
#include "hashers/Hasher.hpp"
#include "mining/MiningJob.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <print>

namespace benchmark {

constexpr uint32_t GENESIS_NONCE = 0x7C2BAC1D;

constexpr auto genesis_template(uint32_t nonce) -> BlockHeader {
    BlockHeader header;
    header.version = 1;
    header.previous_hash = Sha256Hash{0, 0, 0, 0, 0, 0, 0, 0};
    header.merkle_root = Sha256Hash::from_hex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    );
    header.time = 1231006505;
    header.bits = 0x1D00FFFF;
    header.nonce = nonce;
    return header;
}

constexpr auto genesis_target() -> Sha256Hash {
    return BlockHeader::get_target(0x1D00FFFF);
}

constexpr auto genesis_expected_hash() -> Sha256Hash {
    return Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
}

template<Hasher H> struct Named {
    using hasher_type = H;
    std::string_view name;
};

template<Hasher H> consteval auto named(std::string_view name) -> Named<H> {
    return {name};
}

template<Hasher H>
auto run_benchmark(std::string_view name, uint8_t threads) -> void {
    CpuEngine<H> engine{threads};
    BlockHeader solution;
    std::mutex mtx;
    std::condition_variable cv;
    bool found{false};

    engine.solution_callback([&](const BlockHeader& header) -> void {
        std::lock_guard lock{mtx};
        solution = header;
        found = true;
        cv.notify_one();
    });

    engine.submit_job(
        MiningJob{
          .block_header = genesis_template(GENESIS_NONCE - 10'000'000),
          .target = genesis_target()
        }
    );

    auto start = std::chrono::steady_clock::now();
    engine.start();

    {
        std::unique_lock lock{mtx};
        cv.wait(lock, [&] -> bool { return found; });
    }

    auto end = std::chrono::steady_clock::now();
    engine.stop();

    auto ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::println("{:<16} {:>3} threads | {:>10.2f} ms", name, threads, ms);
}

template<typename... Nameds>
auto run(uint8_t threads, Nameds... nameds) -> void {
    std::println("{:=<66}", "");
    std::println("  Hasher Benchmark | {} threads | Genesis Block", threads);
    std::println("{:=<66}", "");

    (run_benchmark<typename Nameds::hasher_type>(nameds.name, threads), ...);
}

} // namespace benchmark
