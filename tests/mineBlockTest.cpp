#include "../src/engine/CpuEngine.hpp"
#include "../src/mining/BlockAssembly.hpp"
#include "../src/networking/RpcJsonClient.hpp"

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>

TEST(MineBlockTest, FetchTemplateMineAndSubmit) {
    RpcConfig cfg{
      .host = "127.0.0.1",
      .port = 18443,
      .username = "admin",
      .password = "password",
      .address = "mmgpupYQZV67rEnkPSwzfAde7HkQTkWpcy",
    };
    RpcJsonClient rpc{cfg};

    auto tmpl_result = rpc.get_block_template();
    ASSERT_TRUE(tmpl_result.has_value()) << tmpl_result.error().message;
    auto& tmpl = *tmpl_result;

    auto assembled = block_assembly::assemble(tmpl);

    CpuEngine engine{1};

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

    engine.submit_job(assembled.job);
    engine.start();

    {
        std::unique_lock lock{mtx};
        ASSERT_TRUE(cv.wait_for(lock, std::chrono::minutes(2), [&] -> bool {
            return found;
        })) << "Mining timed out";
    }

    engine.stop();

    auto hash = solution.hash();
    ASSERT_LT(hash, assembled.job.target);

    auto block_hex = block_assembly::build_submission(solution, assembled);
    auto submit_result = rpc.submit_block(block_hex);
    ASSERT_TRUE(submit_result.has_value()) << submit_result.error().message;
}
