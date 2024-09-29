#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

go build -o compile.elf compile/compile.go
go build -o run-qemu.elf run-qemu/run-qemu.go
