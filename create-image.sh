#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

cd bootboot-in
MKBOOT=$(find ../dependencies/bootboot -type f -name "mkbootimg")
"${MKBOOT}" config.json ../bootboot-out/mykernel.iso
