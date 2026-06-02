# CLI Args

## Networking

- `rpc-host <IP>` Default 127.0.0.1
- `rpc-port <PORT>` Default 18443
- `rpc-username <username>` Default admin
- `rpc-password <password>` Default password
- `retry <number>` Default 5 retries on RPC connection
- `timeout <seconds>` Default 10s on RPC connection
- `verbose` Log everything

## Mining

- `address <address>` Required
- `threads <num of threads>` Deafault **ALL**
- `engine <openssl | own>` Default **openssl**

## Misc

- `network <mainnet|testnet|regtest>` Default regtest
- `bench` Runs benchmark, ignores the rest
