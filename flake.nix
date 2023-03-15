{
  description = "ncnn playground";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem
    (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages.cpp-mjpeg-streamer = pkgs.stdenv.mkDerivation {
          pname = "cpp-mjpeg-streamer";
          version = "unstable-2022-08-12";

          src = pkgs.fetchFromGitHub {
            owner = "nadjieb";
            repo = "cpp-mjpeg-streamer";
            rev = "65e2129954dbbdb3b2b227c3d76bdb1283e88855";
            hash = "sha256-1h5AymjsVl4bJ7L3xgMqYiVrjyXySyv/hotX1U+tOI8=";
          };

          nativeBuildInputs = with pkgs; [
            cmake
          ];
        };

        packages.ncnn-playground = pkgs.stdenv.mkDerivation {
          pname = "ncnn-playground";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            pkg-config

            meson
            ninja
          ];

          buildInputs = with pkgs; [
            (ncnn.overrideAttrs (oldAttrs: rec {
              version = "20230223";
              src = pkgs.fetchFromGitHub {
                owner = "Tencent";
                repo = "ncnn";
                rev = version;
                hash = "sha256-rH1shom4JckFS++NxS2Hiobk0rzZ3lUZp7eC87RGSrI=";
              };
              patches = [
                ./nix/ncnn_pc.patch
              ];
              cmakeFlags = [
                "-DNCNN_CMAKE_VERBOSE=1"
                "-DNCNN_SHARED_LIB=1"
                "-DNCNN_ENABLE_LTO=1"
                "-DNCNN_BUILD_EXAMPLES=0"
                "-DNCNN_BUILD_TOOLS=0"
                "-DNCNN_BUILD_BENCHMARK=0"
              ];
            }))
            opencv
            packages.cpp-mjpeg-streamer
          ];
        };

        devShells.default = pkgs.mkShell {
          name = "ncnn-playground";

          nativeBuildInputs = with pkgs; [
            pkg-config

            gcc
            meson
            ninja
          ];

          buildInputs = with pkgs; [
            (ncnn.overrideAttrs (oldAttrs: rec {
              version = "20230223";
              src = pkgs.fetchFromGitHub {
                owner = "Tencent";
                repo = "ncnn";
                rev = version;
                hash = "sha256-rH1shom4JckFS++NxS2Hiobk0rzZ3lUZp7eC87RGSrI=";
              };
              patches = [
                ./nix/ncnn_pc.patch
              ];
              cmakeFlags = [
                "-DNCNN_CMAKE_VERBOSE=1"
                "-DNCNN_SHARED_LIB=1"
                "-DNCNN_ENABLE_LTO=1"
                "-DNCNN_BUILD_EXAMPLES=0"
                "-DNCNN_BUILD_TOOLS=0"
                "-DNCNN_BUILD_BENCHMARK=0"
              ];
            }))
            (opencv.override {enableGtk3 = true;})
            packages.cpp-mjpeg-streamer
          ];
        };
      }
    );
}
