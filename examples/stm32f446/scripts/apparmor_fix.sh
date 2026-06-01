#!/usr/bin/env bash
set -euo pipefail

PROFILE_PATH="/etc/apparmor.d/nix-bwrap"

command -v apparmor_parser &>/dev/null || { echo "AppArmor is not installed."; exit 1; }

RESTRICTION=$(sysctl -n kernel.apparmor_restrict_unprivileged_userns 2>/dev/null || echo "0")
if [[ "$RESTRICTION" == "0" ]]; then
  echo "Unprivileged user namespace restriction is not active, nothing to do."
  exit 0
fi

echo "Installing AppArmor profile for Nix bwrap..."

sudo tee "$PROFILE_PATH" > /dev/null << 'PROFILE'
abi <abi/4.0>,
include <tunables/global>

profile nix-bwrap /nix/store/*/bin/bwrap flags=(unconfined) {
  userns,
  include if exists <local/nix-bwrap>
}
PROFILE

sudo apparmor_parser -r "$PROFILE_PATH"

echo "Done. You can now run: nix build"