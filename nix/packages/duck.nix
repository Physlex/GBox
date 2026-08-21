# Documentation generator for the gbox sources, run from the development shell as `duck`.
{ rustPlatform, fetchFromGitHub, pkg-config, libclang }:
rustPlatform.buildRustPackage {
  pname = "duck";
  version = "unstable";

  src = fetchFromGitHub {
    owner = "rdmsr";
    repo = "duck";
    rev = "ff95938795dfc9e55f891eb27de68a4bf63b122a";
    hash = "sha256-aBCm69V//ZtYmmE3GAvUqErSnVIp59te6bJOlibICSM==";
  };

  cargoHash = "sha256-XzqhofYePhrusi5FPx9qWh8gBCsNUb3QAvi78SKNoJs=";

  LIBCLANG_PATH = "${libclang.lib}/lib";
  buildInputs = [ libclang ];
  nativeBuildInputs = [ pkg-config ];
}
