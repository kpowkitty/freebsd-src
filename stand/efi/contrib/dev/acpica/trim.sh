#!/bin/bash

# Adjust this path to point to the top of your ACPICA source directory
SRC_DIR="/home/bee/projects/freebsd-src/stand/efi/contrib/dev/acpica/components"

# Match all .c and .h files under the directory
find "$SRC_DIR" \( -name "*.c" -o -name "*.h" \) -type f | while read -r file; do
  echo "Processing: ${file}"
  sed -Ei 's|#include[[:space:]]*<([^/<>]+/)*([^/<>]+)>|#include <\2>|g' "${file}"
done
