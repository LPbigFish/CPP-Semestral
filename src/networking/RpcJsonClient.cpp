#include "RpcJsonClient.hpp"
#include "../Logger.hpp"
#include "../core/address.hpp"
#include <boost/beast/core/detail/base64.hpp>
#include <boost/json/src.hpp>
#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <utility>

int RpcJsonClient::call_id = 1;

RpcJsonClient::RpcJsonClient(RpcConfig config): config{std::move(config)} {
    auth_header
        = craft_auth_header(this->config.username, this->config.password);
}

auto RpcJsonClient::craft_auth_header(
    const std::string& username, const std::string& password
) -> std::string {
    std::string credentials = username + ":" + password;
    std::string encoded;
    encoded.resize(
        boost::beast::detail::base64::encoded_size(credentials.size())
    );
    boost::beast::detail::base64::encode(
        encoded.data(), credentials.data(), credentials.size()
    );
    return "Basic " + encoded;
}

auto RpcJsonClient::build_hhtp_request(std::string_view body) const
    -> std::string {
    return std::format(
        "POST / HTTP/1.1\r\n"
        "Host: {}:{}\r\n"
        "Authorization: {}\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{}",
        config.host,
        config.port,
        auth_header,
        body.size(),
        body
    );
}

auto RpcJsonClient::parse_http_response(std::string_view response)
    -> std::expected<json::value, ProtocolError> {
    auto header_end = response.find("\r\n\r\n");
    if (header_end == std::string_view::npos) {
        return std::unexpected(
            ProtocolError{.message = "Invalid HTTP response"}
        );
    }

    auto status_line = response.substr(0, response.find("\r\n"));
    if (!status_line.contains("200 OK")) {
        return std::unexpected(
            ProtocolError{.message = "HTTP error: " + std::string(status_line)}
        );
    }

    auto body = response.substr(header_end + 4);

    json::value parsed;
    try {
        parsed = json::parse(body);
    } catch (const std::exception& e) {
        return std::unexpected(
            ProtocolError{
              .message = std::format("JSON parse error: {}", e.what()),
            }
        );
    }

    json::value* error = parsed.if_object()->if_contains("error");
    if (error && !error->is_null()) {
        return std::unexpected(
            ProtocolError{
              .message = std::format(
                  "RPC error: {}", error->as_object().at("message").as_string()
              ),
              .code
              = static_cast<int>(error->as_object().at("code").as_int64()),
            }
        );
    }

    return parsed.as_object().at("result");
}

auto RpcJsonClient::send_request(const json::object& data)
    -> std::expected<json::value, ProtocolError> {
    try {
        tcp::socket socket{io_context};
        tcp::resolver resolver{io_context};
        auto endpoints
            = resolver.resolve(config.host, std::to_string(config.port));
        asio::connect(socket, endpoints);

        auto body = json::serialize(data);
        Logger::instance().debug(
            "RPC -> {} ({} bytes)", data.at("method").as_string(), body.size()
        );
        auto request = build_hhtp_request(body);
        asio::write(socket, asio::buffer(request));

        asio::streambuf buffer;
        boost::system::error_code ec;
        asio::read(socket, buffer, ec);
        if (ec && ec != asio::error::eof) {
            return std::unexpected(
                ProtocolError{
                  .message = std::format("Network error: {}", ec.message()),
                }
            );
        }

        std::string response;
        auto bytes_read = buffer.size();
        response.resize(bytes_read);
        std::memcpy(response.data(), buffer.data().data(), bytes_read);

        socket.close();
        Logger::instance().debug("RPC <- response ({} bytes)", bytes_read);
        return parse_http_response(response);
    } catch (const std::exception& e) {
        return std::unexpected(
            ProtocolError{.message = std::format("Network error: {}", e.what())}
        );
    }
}

auto RpcJsonClient::call(std::string_view method, json::array params)
    -> std::expected<json::value, ProtocolError> {
    json::object request{
      {"jsonrpc", "2.0"},
      {"id", call_id++},
      {"method", std::string{method}},
      {"params", std::move(params)},
    };
    return send_request(request);
}

auto RpcJsonClient::get_block_template()
    -> std::expected<BlockTemplate, ProtocolError> {
    auto result = call(
        "getblocktemplate",
        json::array{json::object{{"rules", json::array{"segwit"}}}}
    );
    if (!result) {
        return std::unexpected(result.error());
    }

    try {
        auto& obj = result->as_object();
        std::vector<TransactionData> transactions;

        if (auto* txs = obj.if_contains("transactions")) {
            for (auto& tx_val : txs->as_array()) {
                transactions.push_back(TransactionData::parse_tx_data(tx_val));
            }
        }

        auto coinbase_amount
            = static_cast<uint64_t>(obj.at("coinbasevalue").as_int64());

        auto scriptpubkey_result = core::address::craft_p2pkh_scriptpubkey(
            config.address, config.network
        );

        if (!scriptpubkey_result) {
            std::string_view msg;
            switch (scriptpubkey_result.error()) {
                case core::address::address_parse_error::InvalidBase58Input:
                    msg = "Invalid address format";
                    break;
                case core::address::address_parse_error::InvalidLength:
                    msg = "Invalid address length";
                    break;
                case core::address::address_parse_error::InvalidNetworkPrefix:
                    msg = "Address does not match selected network";
                    break;
            }
            return std::unexpected(ProtocolError{.message = std::string{msg}});
        }

        uint32_t height = static_cast<uint32_t>(obj.at("height").as_int64());

        auto bits_str = std::string{obj.at("bits").as_string()};

        BlockTemplate tmpl{
          .previous_hash
          = Sha256Hash::from_hex(obj.at("previousblockhash").as_string()),
          .version = static_cast<uint32_t>(obj.at("version").as_int64()),
          .bits = static_cast<uint32_t>(std::stoul(bits_str, nullptr, 16)),
          .time = static_cast<uint32_t>(obj.at("curtime").as_int64()),
          .height = height,
          .coinbase_value = coinbase_amount,
          .script_pubkey = std::move(*scriptpubkey_result),
          .transactions = std::move(transactions),
        };

        Logger::instance().debug(
            "Block template: height={} bits=0x{:08x} txs={} prev={}",
            tmpl.height,
            tmpl.bits,
            tmpl.transactions.size(),
            tmpl.previous_hash.to_hex()
        );

        return tmpl;
    } catch (const std::exception& e) {
        return std::unexpected(
            ProtocolError{
              .message
              = std::format("Error parsing block template: {}", e.what()),
            }
        );
    }
}

auto RpcJsonClient::submit_block(const std::string& block_data)
    -> std::expected<void, ProtocolError> {
    Logger::instance().debug(
        "Submitting block ({} bytes)...", block_data.size()
    );
    auto result = call("submitblock", json::array{block_data});
    if (!result) {
        return std::unexpected(result.error());
    }

    if (!result->is_null()) {
        return std::unexpected(
            ProtocolError{
              .message = std::format("Block rejected: {}", result->as_string()),
            }
        );
    }

    Logger::instance().debug("Block accepted by node");
    return {};
}
