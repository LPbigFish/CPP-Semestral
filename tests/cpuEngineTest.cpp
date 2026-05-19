#include "../src/engine/CpuEngine.hpp"
#include "../src/core/BlockHeader.hpp"
#include "../src/core/OpensslHasher.hpp"

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

namespace {

constexpr uint32_t GENESIS_NONCE = 0x7C2BAC1D;

auto genesis_template(uint32_t nonce) -> BlockHeader {
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

auto genesis_target() -> Sha256Hash {
    return BlockHeader::get_target(0x1D00FFFF);
}

auto genesis_expected_hash() -> Sha256Hash {
    return Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
}
} // namespace

TEST(CpuEngineTest, FindsGenesisNonceSingleThread) {
    CpuEngine engine(1);

    MiningJob job;
    job.block_template = genesis_template(GENESIS_NONCE - 100);
    job.target = genesis_target();

    std::mutex mtx;
    std::condition_variable cv;
    BlockHeader solution;
    bool found = false;

    engine.solution_callback([&](const BlockHeader& header) {
        std::lock_guard lock{mtx};
        solution = header;
        found = true;
        cv.notify_one();
    });

    engine.submit_job(job);
    engine.start();

    {
        std::unique_lock lock{mtx};
        ASSERT_TRUE(cv.wait_for(lock, std::chrono::seconds(30), [&] -> bool {
            return found;
        }));
    }

    EXPECT_EQ(solution.nonce, GENESIS_NONCE);
    EXPECT_EQ(solution.hash(), genesis_expected_hash());
}

TEST(CpuEngineTest, FindsGenesisNonceMultithreaded) {
    uint8_t num_threads = std::max(2u, std::thread::hardware_concurrency() - 1);
    OpensslHasher hasher;
    CpuEngine engine(num_threads);

    MiningJob job;
    job.block_template = genesis_template(GENESIS_NONCE - 1'000);
    job.target = genesis_target();

    std::mutex mtx;
    std::condition_variable cv;
    BlockHeader solution;
    bool found = false;

    engine.solution_callback([&](const BlockHeader& header) -> void {
        std::lock_guard lock{mtx};
        solution = header;
        found = true;
        cv.notify_one();
    });

    engine.submit_job(job);
    engine.start();

    {
        std::unique_lock lock{mtx};
        ASSERT_TRUE(cv.wait_for(lock, std::chrono::minutes(4), [&] -> bool {
            return found;
        }));
    }

    EXPECT_EQ(solution.nonce, GENESIS_NONCE);
    EXPECT_EQ(solution.hash(), genesis_expected_hash());
}
