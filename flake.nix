{
  description = "C++ Flake";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  nixConfig = {
    extra-substituters = [
      "https://bitcoin-regtest-mining-adapted.cachix.org"
    ];
    extra-trusted-public-keys = [
      "bitcoin-regtest-mining-adapted.cachix.org-1:9gwl08sRWqIz9U7x7I/42ghX6HK0OdmAZBQJR5UMVdI="
    ];
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
        "x86_64-darwin"
      ];
      perSystem =
        { pkgs, ... }:
        let
          stdenv = pkgs.llvmPackages_latest.libcxxStdenv;

          customBitcoin = pkgs.bitcoin.overrideAttrs (oldAttrs: {
            pname = "bitcoin-regtest-fast";
            postPatch = (oldAttrs.postPatch or "") + ''
              sed -i 's/consensus.fPowNoRetargeting = true;/consensus.fPowNoRetargeting = false; consensus.nPowTargetSpacing = 120;/g' src/chainparams.cpp
            '';
          });

          dependencies = with pkgs; [
            openssl
            boost
            (gtest.override { inherit stdenv; })
          ];

          runtimeSoftware = with pkgs; [
            jq
            customBitcoin
          ];

          clangTools = with pkgs.llvmPackages_latest; [
            clang-tools
            lldb
          ];

          buildTools =
            with pkgs;
            [
              gnumake
              cmake
              cmake-format
            ]
            ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
              binutils
            ];
        in
        {
          devShells = {
            default = pkgs.mkShell.override { inherit stdenv; } {
              packages = dependencies ++ runtimeSoftware ++ clangTools ++ buildTools;
            };

            noBitcoin = pkgs.mkShell.override { inherit stdenv; } {
              packages = dependencies ++ clangTools ++ buildTools;
            };
          };

          packages = {
            default = stdenv.mkDerivation {
              pname = "semestralBitcoinMiner";
              version = "0.0.1";
              src = ./.;

              nativeBuildInputs = [ pkgs.cmake ];
              buildInputs = dependencies;

              cmakeFlags = [
                "-DCMAKE_BUILD_TYPE=Release"
              ];

              installPhase = ''
                runHook preInstall
                mkdir -p $out/bin
                install -m755 Semestral $out/bin/
                runHook postInstall
              '';
            };
            bitcoin = customBitcoin;
          };
        };
    };
}
