#!/usr/bin/env bash
set -euo pipefail

RPC_USER="admin"
RPC_PASS="password"
RPC_PORT=18443

bitcoin-cli -regtest -rpcuser="$RPC_USER" -rpcpassword="$RPC_PASS" -rpcport="$RPC_PORT" getblocktemplate '{"rules":["segwit"]}'
