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
        overlay = self: super: {
          cpp-mjpeg-streamer = self.callPackage ./nix/cpp-mjpeg-streamer {};
          ncnn = super.ncnn.overrideAttrs (oldAttrs: rec {
            version = "20230223";
            src = self.fetchFromGitHub {
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
          });
          ncnn-playground = self.callPackage ./nix/ncnn-playground {
            inherit (self) ncnn cpp-mjpeg-streamer;
          };
        };

        pkgs = import nixpkgs {
          inherit system;
          overlays = [overlay];
        };
      in {
        packages.ncnn-playground = pkgs.ncnn-playground;

        devShells.default = pkgs.mkShell {
          name = "ncnn-playground";

          nativeBuildInputs = with pkgs; [
            pkg-config

            gcc
            meson
            ninja
          ];

          buildInputs = with pkgs; [
            ncnn
            (opencv.override {enableGtk3 = true;})
            cpp-mjpeg-streamer
          ];
        };
      }
    );
}
