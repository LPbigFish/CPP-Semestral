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
        let
          stdenv = pkgs.llvmPackages_latest.libcxxStdenv;

          dependencies = with pkgs; [
            openssl
            boost
            (gtest.override { inherit stdenv; })
          ];

          runtimeSoftware = with pkgs; [
            jq
            bitcoin
          ];

          clangTools = with pkgs; [
            llvmPackages_latest.clang-tools
            llvmPackages_latest.lldb
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

          packages.default = stdenv.mkDerivation {
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
        };
    };
}
