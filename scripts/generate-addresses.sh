#!/usr/bin/env bash
set -euo pipefail

RPC_USER="admin"
RPC_PASS="password"
RPC_PORT=18443

CLI="bitcoin-cli -regtest -rpcuser=$RPC_USER -rpcpassword=$RPC_PASS -rpcport=$RPC_PORT"
ADDR_FILE="$(cd "$(dirname "$0")/.." && pwd)/addresses.txt"
COUNT="${1:-10}"

if ! $CLI listwallets 2>/dev/null | grep -q "default"; then
    $CLI createwallet "default" > /dev/null 2>&1 || $CLI loadwallet "default" > /dev/null 2>&1
fi

for i in $(seq 1 "$COUNT"); do
    ADDR=$($CLI getnewaddress "addr-$i")
    echo "$ADDR" >> "$ADDR_FILE"
done

echo "Generated $COUNT addresses -> $ADDR_FILE"
