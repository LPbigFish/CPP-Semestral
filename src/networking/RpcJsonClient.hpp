#pragma once

#include "../core/Network.hpp"
#include "MiningProtocol.hpp"
#include <boost/asio.hpp>
#include <expected>
#include <string>
#include <string_view>

namespace json = boost::json;
namespace asio = boost::asio;
namespace net = core::network;

using tcp = asio::ip::tcp;

struct RpcConfig {
    uint16_t port;
    uint32_t timeout{30};
    std::string host;
    std::string username;
    std::string password;
    std::string address;
    net::Network network;
};

class RpcJsonClient {
    asio::io_context io_context{};
    RpcConfig config{};
    std::string auth_header;
    static int call_id;

    auto send_request(const json::object& data)
        -> std::expected<json::value, ProtocolError>;

    [[nodiscard]] auto build_hhtp_request(std::string_view body) const
        -> std::string;

  public:
    explicit RpcJsonClient(RpcConfig config);

    auto call(std::string_view method, json::array params)
        -> std::expected<json::value, ProtocolError>;

    auto get_block_template() -> std::expected<BlockTemplate, ProtocolError>;

    auto submit_block(const std::string& block_data)
        -> std::expected<void, ProtocolError>;

    static auto
    craft_auth_header(const std::string& username, const std::string& password)
        -> std::string;

    static auto parse_http_response(std::string_view response)
        -> std::expected<json::value, ProtocolError>;

    [[nodiscard]] auto get_address() const -> std::string {
        return config.address;
    }
};
