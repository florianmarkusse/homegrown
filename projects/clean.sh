#!/bin/bash
set -eo pipefail
set -x

cd "$(dirname "${BASH_SOURCE[0]}")"

find . -type d -name "build" -exec rm -r {} +
