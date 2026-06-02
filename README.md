# Bitcoin CPU Miner

CLI Bitcoin miner implementovaný v C++23. [Showcase Video](https://youtu.be/p0BtVJrG7Ec)

## Přehled

Program umožňuje těžit Bitcoin bloky pomocí CPU:

- **Multi-threaded mining** – rozdělení nonce prostoru mezi vlákna
- **Dva hashovací enginy** – OpenSSL a vlastní implementace SHA-256
- **Mid-state optimalizace** – prvních 64 bytů hlavičky se hashuje jednou, stav se uloží a pro každý nonce se dopočítává jen posledních 16 bytů
- **JSON-RPC 2.0** přes TCP/HTTP – komunikace s Bitcoin node
- **Benchmark mód** – porovnání výkonu obou engine na genesis bloku

## Požadavky

- CMake ≥ 4.0
- OpenSSL
- Boost (JSON, Asio)
- Google Test
- Clang
- Bitcoin Node
- Nix

## Build

```bash
nix build .
```

## Použití

Je lepší využít Nix devshell, protože obsahuje upravenou verzi Bitcoin node.

Nejprve spusťte Bitcoin node v regtest módu:

```bash
./scripts/start-node.sh
```

Vygenerujte adresu:

```bash
./scripts/generate-p2pkh-address.sh
```

Spusťte miner:

```bash
./Semestral --address m... --threads 4
```

### CLI argumenty

| Argument | Výchozí | Popis |
| -------- | ------- | ----- |
| `--address` | (povinný) | Bitcoin adresa pro odměnu |
| `--rpc-host` | `127.0.0.1` | RPC host |
| `--rpc-port` | `18443` | RPC port |
| `--rpc-username` | `admin` | RPC uživatel |
| `--rpc-password` | `password` | RPC heslo |
| `--threads` | všechna | Počet těžebních vláken |
| `--engine` | `openssl` | Hashovací engine (`openssl`/`own`) |
| `--network` | `regtest` | Síť (`mainnet`/`testnet`/`regtest`) |
| `--retry` | `5` | Max po sobě jdoucích RPC selhání před ukončením |
| `--timeout` | `30` | RPC timeout v sekundách |
| `--bench` | – | Spustit benchmark |
| `--verbose` | – | Extra logování |

## Architektura

### Složky

```_
src/
├── main.cpp                       # Vstupní bod, signal handling
├── Logger.hpp/.cpp                # Singleton logger (soubor + konzole)
├── Benchmark.hpp                  # Benchmark genesis bloku
├── core/
│   ├── Args.hpp/.cpp              # Parsování CLI argumentů
│   ├── Network.hpp                # Síťové konstanty (Mainnet/Testnet/Regtest)
│   ├── Endian.hpp                 # Endian konverze (constexpr)
│   ├── Sha256.hpp/.cpp            # Reprezentace 256-bit hashe
│   ├── BlockHeader.hpp/.cpp       # 80-bytová hlavička bloku
│   ├── MerkleTree.hpp/.cpp        # Výpočet Merkle root
│   ├── address.hpp                # P2PKH scriptPubKey z Base58 adresy
│   ├── base58.hpp                 # Base58Check dekódování
│   └── utils.hpp                  # Pomocné funkce
├── hashers/
│   ├── Hasher.hpp                 # Concept hashovacího rozhraní
│   ├── OpensslHasher.hpp/.cpp     # SHA-256 přes OpenSSL EVP
│   └── OwnHasher.hpp/.cpp         # Vlastní SHA-256
├── mining/
│   ├── MiningJob.hpp              # Struct: BlockHeader + target
│   └── BlockAssembly.hpp/.cpp     # Sestavení bloku a serializace
├── networking/
│   ├── MiningProtocol.hpp         # Typy pro getblocktemplate response
│   ├── RpcJsonClient.hpp/.cpp     # JSON-RPC 2.0 klient
└── engine/
    ├── CpuEngine.hpp              # Multi-threaded těžební engine
    └── Conductor.hpp              # Orchestrátor (polling, job dispatch)
```

### Důležité třídy

- **`CpuEngine`** – spravuje pool `std::jthread` vláken, každé si bere nonce range. Po nalezení řešení zavolá callback a zastaví ostatní vlákna.

- **`Conductor`** – propojuje `CpuEngine` s `RpcJsonClient`. Každých 5 sekund polluje node pro nový block template. Když engine najde řešení, zavolá `submit_block`.

- **`RpcJsonClient`** – JSON-RPC 2.0 klient po TCP. Posílá HTTP POST requesty s Basic auth, parsuje odpovědi přes Boost.JSON. Obsahuje metody `get_block_template()` a `submit_block()`.

- **`OwnHasher`** – vlastní implementace SHA-256 podle [sha256algorithm.com](https://sha256algorithm.com). Implementuje `save_state()`/`restore_state()` pro mid-state optimalizaci.

### Mid-state optimalizace

Header bloku má 80 bytů. Pole verze, prev_hash, merkle_root a čas/bits jsou během těžení konstantní a mění se pouze nonce (poslední 4 byty).

Těžební loop:

1. Hashuje prvních 64 bytů --> uloží vnitřní stav hashe
2. Pro každou nonce: obnoví stav, hashuje zbylých 16 bytů, provede druhý SHA-256

Tím se ušetří ~75 % práce oproti hashování celých 80 bytů při každé nonce.

## Testy

```bash
cd build && ctest --output-on-failure
```

Testy pokrývají:

- SHA-256 hash (oba enginy)
- Endian konverze
- Serializace a target comparison BlockHeader
- Merkle tree (genesis blok, (random) blok 179237)
- Base58 adresy a P2PKH scriptPubKey
- Multi-threaded těžení (oba enginy)
- RPC klient (auth header, HTTP parsing)
