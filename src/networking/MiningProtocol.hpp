#pragma once

#include "../core/Sha256.hpp"
#include <boost/json.hpp>
#include <cstdint>
#include <string>
#include <vector>

struct ProtocolError {
    std::string message;
    int code{};
};

struct TransactionData {
    Sha256Hash txid;
    Sha256Hash wtxid;
    std::string data;

    static constexpr auto parse_tx_data(const boost::json::value& parse_data)
        -> TransactionData {
        auto data = parse_data.at("data").as_string();
        auto txid = Sha256Hash::from_hex(parse_data.at("txid").as_string());
        auto wtxid = Sha256Hash::from_hex(parse_data.at("hash").as_string());

        return TransactionData{
          .txid = txid,
          .wtxid = wtxid,
          .data = std::string{data},
        };
    }
};

struct BlockTemplate {
    Sha256Hash previous_hash;
    uint32_t version{};
    uint32_t bits{};
    uint32_t time{};
    uint32_t height{};
    uint64_t coinbase_value{};
    std::vector<uint8_t> script_pubkey;
    std::vector<TransactionData> transactions;
};
