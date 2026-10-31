{
  description = "dev shell for <snake>";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        # Windows raylib: real static archive, with raylib's bundled GLFW
        # compiled in, so snake.exe needs no libraylib.dll / glfw3.dll.
        raylibWin = (pkgs.pkgsCross.mingwW64.raylib.override { sharedLib = false; }).overrideAttrs (
          old: {
            cmakeFlags =
              builtins.filter (f: builtins.match ".*USE_EXTERNAL_GLFW.*" f == null) old.cmakeFlags
              ++ [ (pkgs.lib.cmakeFeature "USE_EXTERNAL_GLFW" "OFF") ];
          }
        );

        # mingw's libgcc/libstdc++ always ask for -lmcfgthread (mcf model)
        mcfgthreadWin = pkgs.pkgsCross.mingwW64.windows.mcfgthreads;
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.raylib
            pkgs.raygui
            pkgs.gnumake
            pkgs.gcc
          ];

        };

        devShells.windows = pkgs.mkShell {
          buildInputs = [
            pkgs.pkgsCross.mingwW64.stdenv.cc # x86_64-w64-mingw32-g++
            pkgs.gnumake
            pkgs.raygui # header-only, so the native package is fine
            raylibWin
            mcfgthreadWin
          ];
          RAYLIB_WIN = "${raylibWin}";
          RAYGUI_INC = "${pkgs.raygui}/include";
          MCFGTHREAD_WIN = "${mcfgthreadWin}";
        };

      }
    );
}
