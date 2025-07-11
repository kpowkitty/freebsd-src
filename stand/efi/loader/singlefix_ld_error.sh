#!/bin/sh

set -e

SRC_TREE="../../../sys/contrib/dev/acpica"
DEST_DIR="../contrib/dev/acpica"
MAKEFILE_PATH="Makefile"
CALL_GRAPH="tree.txt"

echo "Starting singlefix ld error..."

usage() {
	echo "Usage: $0 <grandparent/parent_of_file> <file_to_bring_over>"
	exit 1
}

if [ $# -ne 2 ]; then
	usage
fi

gpp=$1
filename=$2
src_file="$SRC_TREE/$gpp/$filename"

stand_path="$DEST_DIR/$gpp/$filename"

echo "$stand_path"
echo "$src_file"

# Copy the file to matching location in DEST_DIR
stand_file="$stand_path"
sudo mkdir -p "$DEST_DIR/$gpp"
sudo cp "$src_file" "$stand_file"

echo "Copied $src_file to $stand_file" | tee -a build.log

echo "Editing $stand_file..." | tee -a build.log

# Automatically rewrite #include lines in the copied file
# Example: #include <contrib/dev/acpica/include/accommon.h> -> #include <accommon.h>

sudo sed -Ei '' \
	's|#include <contrib/dev/acpica/include/([^>]+)>|#include <\1>|g' \
	"$stand_file"

echo "Edited $stand_file." | tee -a build.log

if [ ! -f "$MAKEFILE_PATH" ]; then
    echo "Makefile not found: $MAKEFILE_PATH"
    break
fi

if grep -q "[[:space:]]$filename[[:space:]]*\\\\" "$MAKEFILE_PATH"; then
    echo "$filename already present in Makefile."
    exit 0
fi

# prefix of file --- easiest way to find where to insert it into Makefile
prefix=`echo "$filename" | cut -c1-2`
echo "Prefix: $prefix" | tee -a build.log

# capture pattern for line number of first occurence of file in 
# Makefile SRCS+= that has the same scope of the file we are inserting
pattern='^[[:space:]]*'"${prefix}"'[A-Za-z0-9_]*\.c[[:space:]]*\\$'
# echo "Grep pattern: $pattern." | tee -a build.log

# extract the line number
insertion_line=`grep -nE "$pattern" "$MAKEFILE_PATH" | head -n1 \
	| cut -d: -f1`

echo "Insertion line=[$insertion_line]." | tee -a build.log


if [ -n "$insertion_line" ]; then
	echo "Inserting $filename above $insertion_line"
	sed -i '' "${insertion_line}i\\
	$filename \\\\
" "$MAKEFILE_PATH"
else
    echo "Failed to find matching prefix for $filename." \
	    | tee -a build.log
    break
fi

echo "Successful migration for $filename." \
	| tee -a build.log
