{
  stdenv,
  pkg-config,
  meson,
  ninja,
  ncnn,
  opencv,
  cpp-mjpeg-streamer,
}:
stdenv.mkDerivation {
  pname = "ncnn-playground";
  version = "0.1.0";

  src = ../../.;

  nativeBuildInputs = [
    pkg-config

    meson
    ninja
  ];

  mesonBuildType = "release";
  mesonFlags = [
    "-Db_lto=true"
  ];

  buildInputs = [
    ncnn
    opencv
    cpp-mjpeg-streamer
  ];
}
