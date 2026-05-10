#pragma once

#include <boost/json/src.hpp>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>
#include "../core/Sha256.hpp"

struct MiningJob;

struct ProtocolError {
    int code{};
    std::string message;
};

struct TransactionData {
    Sha256Hash txid;
    Sha256Hash wtxid; // Spooky shit z SegWit upgradu, asi neimplementuju
    std::string data;
    

    static constexpr auto parse_tx_data(const boost::json::value& parse_data) -> TransactionData {
        auto data = parse_data.at("data").as_string();
        auto txid = Sha256Hash::from_hex(
            parse_data.at("txid").as_string()
        );
        auto wtxid = Sha256Hash::from_hex(
            parse_data.at("hash").as_string()
        );
        
        return TransactionData{.txid = txid, .wtxid = wtxid, .data = std::string{data}, };
    }
};

struct BlockTemplate {
    uint32_t version{};
    Sha256Hash previous_hash;
    uint32_t bits{};
    uint32_t time{};
    uint32_t height{};
    TransactionData coinbase;
    std::vector<TransactionData> transactions;
};

class MiningProtocol {
    public:
    MiningProtocol() = default;
    MiningProtocol(const MiningProtocol& protocol) = default;
    MiningProtocol(MiningProtocol&& protocol) = default;
    auto operator=(const MiningProtocol& protocol) -> MiningProtocol& = default;
    auto operator=(MiningProtocol&& protocol) -> MiningProtocol& = default;
    virtual ~MiningProtocol() = default;

    virtual auto get_job() -> std::expected<MiningJob, ProtocolError> = 0;

    virtual auto submit_solution(const BlockTemplate& solution) -> std::expected<void, ProtocolError> = 0;
};