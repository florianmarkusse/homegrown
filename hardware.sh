#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

projects/clean.sh
./build-create-run.sh -n
sudo dd bs=4M if=test.hdd of=/dev/sdc1 conv=notrunc
