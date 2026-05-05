#pragma once

#include "../mining/MiningJob.hpp"
#include <functional>


class MiningEngine {
public:
    MiningEngine() = default;

    MiningEngine(const MiningEngine& mining_engine);

    MiningEngine(MiningEngine&& mining_engine) noexcept;

    auto operator=(MiningEngine mining_engine) -> MiningEngine&;

    auto operator=(MiningEngine&& mining_engine) noexcept -> MiningEngine&;

    virtual ~MiningEngine() = default;

    virtual auto start() -> void = 0;
    
    virtual auto stop() -> void = 0;

    virtual auto submit_job(const MiningJob& job) -> void = 0;

    virtual auto solution_callback(std::function<void(const BlockHeader&)>) -> void = 0;
};