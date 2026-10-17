{
  description = "Communicate with the ECIL PJ-I over RS-232C (Jotta protocol)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        nativeBuildInputs = with pkgs; [
          cmake
          pkg-config
          stdenv
        ];
        buildInputs = with pkgs; [
          libserialport
        ];
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = nativeBuildInputs ++ buildInputs;
        };
        packages.default = pkgs.stdenv.mkDerivation {
          name = "libRs232EcilPJI";
          src = ./.;
          inherit nativeBuildInputs;
          inherit buildInputs;
        };
      }
    );
}
