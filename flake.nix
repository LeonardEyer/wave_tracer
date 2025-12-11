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

        # Use GCC 14 for full C++20 <format> support
        gccStdenv = pkgs.gcc14Stdenv;

        myShell = (pkgs.mkShell.override { stdenv = gccStdenv; }) {
          name = "wave_tracer-dev";

          packages = with pkgs; [
            # --- Build Tools ---
            cmake
            ninja
            pkg-config
            ccache

            # --- Version Control ---
            git
            git-lfs

            # --- Libraries ---
            fmt
            sdl3
            libpng
            libdeflate
            openexr
            pugixml
            freetype
            tbb
            yaml-cpp

            # --- GUI / Graphics ---
            libGL
            libGLU
            glew
            glfw
            mesa

            # --- Linux System Deps ---
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
            
            # --- Tools ---
            gdb
          ] ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
            pkgs.darwin.apple_sdk.frameworks.Cocoa
            pkgs.darwin.apple_sdk.frameworks.OpenGL
            pkgs.darwin.apple_sdk.frameworks.IOKit
            pkgs.darwin.apple_sdk.frameworks.CoreVideo
          ];

          # !!! CRITICAL FIX !!!
          # Tells Nix to stop stripping -march=native
          # This enables AVX/FMA instructions required by the code.
          NIX_ENFORCE_NO_NATIVE = "0";

          shellHook = ''
            echo "🌊 wave_tracer Development Environment"
            echo "   (GCC 14 | Native Arch Enabled)"
            echo "---------------------------------------"
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
