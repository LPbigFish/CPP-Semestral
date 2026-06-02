#include "../src/networking/RpcJsonClient.hpp"
#include <gtest/gtest.h>
#include <string>
#include <string_view>

TEST(RpcJsonClientTest, CraftAuthHeader) {
    auto result = RpcJsonClient::craft_auth_header("user", "pass");
    EXPECT_EQ(result, "Basic dXNlcjpwYXNz");
}

TEST(RpcJsonClientTest, CraftAuthHeaderEmpty) {
    auto result = RpcJsonClient::craft_auth_header("", "");
    EXPECT_EQ(result, "Basic Og==");
}

TEST(RpcJsonClientTest, CraftAuthHeaderAdminPassword) {
    auto result = RpcJsonClient::craft_auth_header("admin", "password");
    EXPECT_EQ(result, "Basic YWRtaW46cGFzc3dvcmQ=");
}

TEST(RpcJsonClientTest, ParseHttpResponseValidResult) {
    std::string_view response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: application/json\r\n"
                                "\r\n"
                                R"({"result":"ok","error":null,"id":1})";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::string{result->as_string()}, "ok");
}

TEST(RpcJsonClientTest, ParseHttpResponseRpcError) {
    std::string_view response
        = "HTTP/1.1 200 OK\r\n"
          "\r\n"
          R"({"result":null,"error":{"code":-32600,"message":"Invalid request"},"id":1})";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, -32600);
    EXPECT_TRUE(result.error().message.find("RPC error") != std::string::npos);
}

TEST(RpcJsonClientTest, ParseHttpResponseHttpError) {
    std::string_view response = "HTTP/1.1 401 Unauthorized\r\n"
                                "\r\n"
                                "";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().message.find("HTTP error") != std::string::npos);
}

TEST(RpcJsonClientTest, ParseHttpResponseNoHeaderTerminator) {
    std::string_view response = "HTTP/1.1 200 OK";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(
        result.error().message.find("Invalid HTTP") != std::string::npos
    );
}

TEST(RpcJsonClientTest, ParseHttpResponseInvalidJson) {
    std::string_view response = "HTTP/1.1 200 OK\r\n"
                                "\r\n"
                                "not json";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(
        result.error().message.find("JSON parse error") != std::string::npos
    );
}

TEST(RpcJsonClientTest, ParseHttpResponseEmptyBody) {
    std::string_view response = "HTTP/1.1 200 OK\r\n"
                                "\r\n"
                                "";

    auto result = RpcJsonClient::parse_http_response(response);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(
        result.error().message.find("JSON parse error") != std::string::npos
    );
}

TEST(RpcJsonClientTest, CallFailsWithoutServer) {
    RpcConfig cfg{
      .port = 18443,
      .host = "not-existent-random.offline",
      .username = "a",
      .password = "b",
      .address = "2N1uLZt3n5sHjvVh9Zp7qj8XoG9m1z6yqj",
      .network = net::Regtest{},
    };
    RpcJsonClient client{cfg};

    json::array params{};
    auto result = client.call("getblockchaininfo", std::move(params));

    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(
        result.error().message.find("Network error") != std::string::npos
    );
}
