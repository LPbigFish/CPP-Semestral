{
  description = "C++ Flake";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
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
        {
          devShells.default = pkgs.mkShell.override { stdenv = pkgs.llvmPackages_latest.libcxxStdenv; } {
            packages =
              with pkgs;
              [
                llvmPackages_latest.clang-tools
                llvmPackages_latest.lldb
                gnumake
                cmake
                cmake-format
              ]
              ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
                binutils
              ]
              ++ [
                pkg-config
                boost
                openssl
                (gtest.override { stdenv = pkgs.llvmPackages_latest.libcxxStdenv; })

                jq
                bitcoin
              ];

            shellHooks = "";
          };
        };
    };
}
