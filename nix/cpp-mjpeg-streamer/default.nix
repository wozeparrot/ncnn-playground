{
  stdenv,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation {
  pname = "cpp-mjpeg-streamer";
  version = "unstable-2022-08-12";

  src = fetchFromGitHub {
    owner = "nadjieb";
    repo = "cpp-mjpeg-streamer";
    rev = "65e2129954dbbdb3b2b227c3d76bdb1283e88855";
    hash = "sha256-1h5AymjsVl4bJ7L3xgMqYiVrjyXySyv/hotX1U+tOI8=";
  };

  nativeBuildInputs = [
    cmake
  ];
}
