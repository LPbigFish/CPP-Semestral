#!/usr/bin/env bash
set -euo pipefail

RPC_USER="admin"
RPC_PASS="password"
RPC_PORT=18443

CLI="bitcoin-cli -regtest -rpcuser=$RPC_USER -rpcpassword=$RPC_PASS -rpcport=$RPC_PORT"
ADDR_FILE="$(cd "$(dirname "$0")/.." && pwd)/addresses.txt"
TX_FILE="$(cd "$(dirname "$0")/.." && pwd)/transactions.txt"
COUNT="${1:-10}"

if [[ ! -f "$ADDR_FILE" ]]; then
    echo "ERROR: No addresses file. Run generate-addresses.sh first." >&2
    exit 1
fi

mapfile -t ADDRS < "$ADDR_FILE"
NUM=${#ADDRS[@]}

if [[ "$NUM" -lt 2 ]]; then
    echo "ERROR: Need at least 2 addresses. Generate more with generate-addresses.sh." >&2
    exit 1
fi

echo "" > "$TX_FILE"

WALLET_INFO=$($CLI listwallets 2>/dev/null || true)
if ! echo "$WALLET_INFO" | grep -q "default"; then
    $CLI createwallet "default" > /dev/null 2>&1 || true
fi

UTXOS=$($CLI listunspent)
AVAIL=$(echo "$UTXOS" | jq '[.[] | select(.spendable == true)] | length')

if [[ "$AVAIL" -eq 0 ]]; then
    MINE_ADDR="${ADDRS[$((RANDOM % NUM))]}"
    echo "No spendable UTXOs. Mining 101 blocks to $MINE_ADDR..."
    $CLI generatetoaddress 101 "$MINE_ADDR" > /dev/null
    echo "Done."
fi

for i in $(seq 1 "$COUNT"); do
    FROM_IDX=$((RANDOM % NUM))
    TO_IDX=$((RANDOM % NUM))
    while [[ "$TO_IDX" -eq "$FROM_IDX" ]]; do
        TO_IDX=$((RANDOM % NUM))
    done

    FROM="${ADDRS[$FROM_IDX]}"
    TO="${ADDRS[$TO_IDX]}"

    AMOUNT=$(printf "0.%02d" $((RANDOM % 49 + 1)))

    TXID=$($CLI sendtoaddress "$TO" "$AMOUNT" "" "" false true) || {
        echo "Tx $i failed (insufficient funds?), stopping." >&2
        break
    }
    echo "$TXID  $FROM -> $TO  $AMOUNT BTC" >> "$TX_FILE"
    echo "[$i/$COUNT] $TXID  $FROM -> $TO  $AMOUNT BTC"
done

echo ""
echo "Transactions saved to $TX_FILE"
