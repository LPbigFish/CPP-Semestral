#!/usr/bin/env bash
set -euo pipefail

DATADIR="$(cd "$(dirname "$0")/.." && pwd)/.bitcoin-regtest"
CONF="$DATADIR/bitcoin.conf"
RPC_USER="admin"
RPC_PASS="password"
RPC_PORT=18443
P2P_PORT=18444

mkdir -p "$DATADIR"

cat > "$CONF" <<EOF
regtest=1
server=1
daemon=1
fallbackfee=0.0001
txindex=1
rpcuser=$RPC_USER
rpcpassword=$RPC_PASS
rpcallowip=127.0.0.1

[regtest]
rpcbind=127.0.0.1
rpcport=$RPC_PORT
port=$P2P_PORT
EOF

if pgrep -f "bitcoind.*-datadir=$DATADIR" > /dev/null 2>&1; then
    echo "Node already running."
    exit 0
fi

bitcoind -datadir="$DATADIR" -conf="$CONF"

for i in $(seq 1 30); do
    if bitcoin-cli -regtest -rpcuser="$RPC_USER" -rpcpassword="$RPC_PASS" -rpcport="$RPC_PORT" getblockchaininfo > /dev/null 2>&1; then
        echo "Node is ready."
        echo "  Network:   regtest"
        echo "  RPC port:  $RPC_PORT"
        echo "  RPC user:  $RPC_USER"
        echo "  RPC pass:  $RPC_PASS"
        echo "  Data dir:  $DATADIR"
        echo ""
        echo "Mine 101 blocks to unlock coinbase:"
        echo "  bitcoin-cli -regtest -rpcuser=$RPC_USER -rpcpassword=$RPC_PASS -rpcport=$RPC_PORT generatetoaddress 101 \$(bitcoin-cli -regtest -rpcuser=$RPC_USER -rpcpassword=$RPC_PASS -rpcport=$RPC_PORT getnewaddress)"
        exit 0
    fi
    sleep 0.5
done

echo "ERROR: Node did not start in time." >&2
exit 1
