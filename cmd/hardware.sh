#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../

cmd/clean.sh
cmd/compile.elf
cmd/create-test-image.sh
sudo dd bs=4M if=test.hdd of=/dev/sdc1 conv=notrunc
