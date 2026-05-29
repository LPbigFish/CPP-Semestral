#!/usr/bin/env bash
set -euo pipefail

RPC_USER="admin"
RPC_PASS="password"
RPC_PORT=18443

CLI="bitcoin-cli -regtest -rpcuser=$RPC_USER -rpcpassword=$RPC_PASS -rpcport=$RPC_PORT"

if ! $CLI listwallets 2>/dev/null | grep -q "default"; then
    $CLI createwallet "default" > /dev/null 2>&1 || $CLI loadwallet "default" > /dev/null 2>&1
fi

ADDR=$($CLI getnewaddress "" "legacy")

echo "$ADDR"
