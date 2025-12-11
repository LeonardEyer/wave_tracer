{
  description = "Development environment for wave_tracer (GCC 14 + Native Arch)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        gccStdenv = pkgs.gcc14Stdenv;

        myShell = (pkgs.mkShell.override { stdenv = gccStdenv; }) {
          name = "wave_tracer-dev";

          packages = with pkgs; [
            cmake
            ninja
            pkg-config
            ccache
            git
            git-lfs
            fmt
            sdl3
            libpng
            libdeflate
            openexr
            pugixml
            freetype
            tbb
            yaml-cpp
            libGL
            libGLU
            glew
            glfw
            mesa
            wayland
            libxkbcommon
            libtool
            ibus
            xorg.libX11
            xorg.libXmu
            xorg.libXi
            xorg.libXext
            xorg.libXft
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXcursor
            gdb
          ] ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
            pkgs.darwin.apple_sdk.frameworks.Cocoa
            pkgs.darwin.apple_sdk.frameworks.OpenGL
            pkgs.darwin.apple_sdk.frameworks.IOKit
            pkgs.darwin.apple_sdk.frameworks.CoreVideo
          ];

          # Tells Nix to stop stripping -march=native
          # This enables AVX/FMA instructions required by the code.
          NIX_ENFORCE_NO_NATIVE = "0";

          shellHook = ''
            echo "wave_tracer Development Environment"
            export CC=gcc
            export CXX=g++
          '';
        };
      in
      {
        devShells.default = myShell;
      }
    );
}
