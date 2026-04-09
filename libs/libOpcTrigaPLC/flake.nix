{
  description = "A library to comunicate with the PLC in the Triga Control Table using OPC client";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    open26541pp.url = "https://github.com/open62541pp/open62541pp?submodules=true";
    open26541pp.flake = false;
    open26541pp.type = "git";
    open26541pp.submodules = true;
  };

  outputs = { nixpkgs, flake-utils, open26541pp, ... }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        nativeBuildInputs = with pkgs; [
          cmake
          git
          pkg-config
          python3
          stdenv
        ];
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = nativeBuildInputs;
        };
        packages.default = pkgs.stdenv.mkDerivation {
          name = "libOpcTrigaPLC";
          src = ./.;
          cmakeFlags = [
            "-DFETCHCONTENT_SOURCE_DIR_OPEN62541PP=${open26541pp}"
          ];
          inherit nativeBuildInputs;
        };
      }
    );
}
