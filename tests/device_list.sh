#!/bin/sh
# Run hardware-independent allocation ownership checks with a GNU-compatible linker.
set -eu
cd "$(dirname "$0")/.."
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM
"${CC:-cc}" -std=gnu17 -Wall -Werror -I. ${CFLAGS:-} \
  tests/unit/device_list/test.c device/device_list.c \
  -Wl,--wrap=free ${LDFLAGS:-} -o "$build_dir/device_list_test"
"$build_dir/device_list_test"
