#include "BlockAssembly.hpp"
#include "../core/BlockHeader.hpp"
#include "../core/MerkleTree.hpp"
#include "../core/Sha256.hpp"
#include "../core/utils.hpp"
#include "../hashers/OpensslHasher.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace {

auto build_scriptsig(uint32_t height) -> std::vector<uint8_t> {
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
        result.end(), height_bytes.begin(), height_bytes.begin() + num_bytes
    );
    return result;
}

auto build_commitment_script(const std::vector<uint8_t>& commitment_hash)
    -> std::vector<uint8_t> {
    auto script = std::vector<uint8_t>{0x6A, 0x24, 0xAA, 0x21, 0xA9, 0xED};
    script.insert(script.end(), commitment_hash.begin(), commitment_hash.end());
    return script;
}

auto build_coinbase_segwit(
    uint64_t amount,
    const std::vector<uint8_t>& script_pubkey,
    uint32_t height,
    const std::vector<uint8_t>& commitment_hash
) -> std::vector<uint8_t> {
    auto version = endian::to_le_bytes(uint32_t{1});
    auto input_count = std::array<uint8_t, 1>{0x01};
    auto prev_txid = std::array<uint8_t, 32>{};
    auto prev_vout = std::array<uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF};
    auto scriptsig = build_scriptsig(height);
    auto scriptsig_size
        = std::array<uint8_t, 1>{static_cast<uint8_t>(scriptsig.size())};
    auto sequence = std::array<uint8_t, 4>{};
    auto reward_amount = endian::to_le_bytes(amount);
    auto reward_script_size
        = std::array<uint8_t, 1>{static_cast<uint8_t>(script_pubkey.size())};
    auto commitment_script = build_commitment_script(commitment_hash);
    auto commitment_script_size = std::array<uint8_t, 1>{
      static_cast<uint8_t>(commitment_script.size())
    };
    auto locktime = std::array<uint8_t, 4>{};
    auto marker = std::array<uint8_t, 1>{0x00};
    auto flag = std::array<uint8_t, 1>{0x01};
    auto witness_item_count = std::array<uint8_t, 1>{0x01};
    auto witness_item_size = std::array<uint8_t, 1>{0x20};
    auto witness_data = std::array<uint8_t, 32>{};
    auto output_count = std::array<uint8_t, 1>{0x02};
    auto commitment_amount = std::array<uint8_t, 8>{};

    std::vector<uint8_t> tx;
    core::concat_all(
        tx,
        version,
        marker,
        flag,
        input_count,
        prev_txid,
        prev_vout,
        scriptsig_size,
        scriptsig,
        sequence,
        output_count,
        reward_amount,
        reward_script_size,
        script_pubkey,
        commitment_amount,
        commitment_script_size,
        commitment_script,
        witness_item_count,
        witness_item_size,
        witness_data,
        locktime
    );
    return tx;
}

auto build_coinbase_nonwitness(
    uint64_t amount,
    const std::vector<uint8_t>& script_pubkey,
    uint32_t height,
    const std::vector<uint8_t>& commitment_hash
) -> std::vector<uint8_t> {
    auto version = endian::to_le_bytes(uint32_t{1});
    auto input_count = std::array<uint8_t, 1>{0x01};
    auto prev_txid = std::array<uint8_t, 32>{};
    auto prev_vout = std::array<uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF};

    auto scriptsig = build_scriptsig(height);
    auto scriptsig_size
        = std::array<uint8_t, 1>{static_cast<uint8_t>(scriptsig.size())};

    auto sequence = std::array<uint8_t, 4>{};
    auto reward_amount = endian::to_le_bytes(amount);

    auto reward_script_size
        = std::array<uint8_t, 1>{static_cast<uint8_t>(script_pubkey.size())};

    auto commitment_script = build_commitment_script(commitment_hash);
    auto commitment_script_size = std::array<uint8_t, 1>{
      static_cast<uint8_t>(commitment_script.size())
    };

    auto output_count = std::array<uint8_t, 1>{0x02};

    auto commitment_amount = std::array<uint8_t, 8>{};
    auto locktime = std::array<uint8_t, 4>{};

    std::vector<uint8_t> tx;
    core::concat_all(
        tx,
        version,
        input_count,
        prev_txid,
        prev_vout,
        scriptsig_size,
        scriptsig,
        sequence,
        output_count,
        reward_amount,
        reward_script_size,
        script_pubkey,
        commitment_amount,
        commitment_script_size,
        commitment_script,
        locktime
    );
    return tx;
}

} // namespace

namespace block_assembly {

auto assemble(const BlockTemplate& tmpl) -> Result {
    std::vector<Sha256Hash> wtxids;
    // One empty wtxid (coinbase)
    wtxids.emplace_back();

    for (const auto& tx : tmpl.transactions) {
        wtxids.push_back(tx.wtxid);
    }

    auto witness_root_internal
        = MerkleTree{std::move(wtxids)}.finalize().to_internal_bytes();

    std::array<uint8_t, 64> commitment_input{};
    std::ranges::copy(witness_root_internal, commitment_input.begin());

    auto commitment_hash = OpensslHasher::double_hash_bytes(commitment_input);
    auto commitment_bytes = commitment_hash.to_internal_bytes();
    auto commitment_vec = std::vector<uint8_t>(
        commitment_bytes.begin(), commitment_bytes.end()
    );

    auto segwit_coinbase = build_coinbase_segwit(
        tmpl.coinbase_value, tmpl.script_pubkey, tmpl.height, commitment_vec
    );

    auto non_witness = build_coinbase_nonwitness(
        tmpl.coinbase_value, tmpl.script_pubkey, tmpl.height, commitment_vec
    );

    auto coinbase_txid = OpensslHasher::double_hash_bytes(non_witness);

    std::vector<TXID> txids;
    txids.reserve(1 + tmpl.transactions.size());
    txids.push_back(coinbase_txid);
    for (const auto& tx : tmpl.transactions) {
        txids.push_back(tx.txid);
    }
    auto merkle_root = MerkleTree{std::move(txids)}.finalize();

    MiningJob job;
    job.block_header.previous_hash = tmpl.previous_hash;
    job.block_header.merkle_root = merkle_root;
    job.block_header.version = tmpl.version;
    job.block_header.time = tmpl.time;
    job.block_header.bits = tmpl.bits;
    job.block_header.nonce = 0;
    job.target = BlockHeader::get_target(tmpl.bits);

    return Result{
      .job = job,
      .segwit_coinbase = std::move(segwit_coinbase),
      .transactions = tmpl.transactions,
      .reward = tmpl.coinbase_value,
      .height = tmpl.height,
    };
}

auto build_submission(const BlockHeader& solved_header, const Result& assembled)
    -> std::string {
    auto header_copy = solved_header;
    auto header_bytes = header_copy.serialize();
    auto hex = core::bytes_to_hex(header_bytes);

    auto tx_count = static_cast<uint64_t>(1 + assembled.transactions.size());
    if (tx_count < 0xFD) {
        hex += core::bytes_to_hex(
            std::array<uint8_t, 1>{static_cast<uint8_t>(tx_count)}
        );
    } else if (tx_count <= 0xFFFF) {
        auto le = endian::to_le_bytes(static_cast<uint16_t>(tx_count));
        hex += "fd";
        hex += core::bytes_to_hex(le);
    } else if (tx_count <= 0xFFFFFFFF) {
        auto le = endian::to_le_bytes(static_cast<uint32_t>(tx_count));
        hex += "fe";
        hex += core::bytes_to_hex(le);
    } else {
        auto le = endian::to_le_bytes(tx_count);
        hex += "ff";
        hex += core::bytes_to_hex(le);
    }

    hex += core::bytes_to_hex(assembled.segwit_coinbase);
    for (const auto& tx : assembled.transactions) {
        hex += tx.data;
    }

    return hex;
}

} // namespace block_assembly
