#pragma once

#include "../core/BlockHeader.hpp"
#include "../core/OpensslHasher.hpp"
#include "../core/Sha256.hpp"
#include "../core/utils.hpp"
#include <boost/json/src.hpp>
#include <cstdint>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <vector>

struct MiningJob;

struct ProtocolError {
    int code{};
    std::string message;
};

struct TransactionData {
    Sha256Hash txid;
    Sha256Hash wtxid; // Spooky shit z SegWit upgradu, asi neimplementuju
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

    static auto assemble_coinbase(
        uint64_t coinbase_amount,
        const std::vector<uint8_t>& scriptpubkey,
        uint32_t height
    ) -> TransactionData {
        /*
        {
  "version": "01000000",
  "inputcount": "01",
  "inputs": [
    {
      "txid":
"0000000000000000000000000000000000000000000000000000000000000000", "vout":
"ffffffff", "scriptsigsize": "1b", "scriptsig":
"03951a0604f15ccf5609013803062b9b5a0100072f425443432f20", "sequence": "00000000"
    }
  ],
  "outputcount": "01",
  "outputs": [
    {
      "amount": "ebc3149500000000",
      "scriptpubkeysize": "19",
      "scriptpubkey": "76a9142c30a6aaac6d96687291475d7d52f4b469f665a688ac"
    }
  ],
  "locktime": "00000000"
}
        */
        auto version = endian::to_le_bytes(uint32_t{1});
        auto input_count = uint8_t{1};
        auto prev_txid = std::array<uint8_t, 32>{};
        auto prev_vout = std::array<uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF};

        auto scriptsig = [height]() -> std::vector<uint8_t> {
            auto height_bytes = endian::to_le_bytes(height);
            uint8_t num_bytes{};
            if (height < 0x100) {
                num_bytes = 1;
            } else if (height < 0x10000) {
                num_bytes = 2;
            } else if (height < 0x1000000) {
                num_bytes = 3;
            } else {
                num_bytes = 4;
            }

            std::vector<uint8_t> result;
            result.push_back(num_bytes);
            result.insert(
                result.end(),
                height_bytes.begin(),
                height_bytes.begin() + num_bytes
            );
            return result;
        }();
        auto scriptsig_size = static_cast<uint8_t>(scriptsig.size());

        auto sequence = std::array<uint8_t, 4>{0x00, 0x00, 0x00, 0x00};
        auto ouput_count = uint8_t{1};
        auto amount = endian::to_le_bytes(coinbase_amount);
        auto scriptpubkey_size = static_cast<uint8_t>(scriptpubkey.size());
        auto locktime = std::array<uint8_t, 4>{0x00, 0x00, 0x00, 0x00};

        std::vector<uint8_t> raw_tx;
        core::concat_all(
            raw_tx,
            version,
            std::views::single(input_count),
            prev_txid,
            prev_vout,
            std::views::single(scriptsig_size),
            scriptsig,
            sequence,
            std::views::single(ouput_count),
            amount,
            std::views::single(scriptpubkey_size),
            scriptpubkey,
            locktime
        );

        return TransactionData{
          .txid = OpensslHasher::double_hash_bytes(raw_tx),
          .wtxid = OpensslHasher::double_hash_bytes(raw_tx),
          .data = core::bytes_to_hex(raw_tx),
        };
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

    virtual auto submit_solution(const BlockHeader& solution)
        -> std::expected<void, ProtocolError> = 0;
};
