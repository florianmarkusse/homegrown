#!/bin/bash
set -eo pipefail
set -x

cd "$(dirname "${BASH_SOURCE[0]}")"

rm -f test.hdd DATAFLS.INF DSKIMG.INF BOOTX64.EFI kernel.bin
find . -type d -name "build" -exec rm -r {} +
