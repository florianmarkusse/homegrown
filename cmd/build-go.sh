#!/bin/bash
set -eo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

go build -o clean.elf clean/cli.go
go build -o compile.elf compile/compile.go
go build -o compile-run.elf compile-run/compile-run.go
go build -o run-qemu.elf run-qemu/run-qemu.go
go build -o hardware.elf hardware/hardware.go
go build -o headers.elf headers/headers.go
