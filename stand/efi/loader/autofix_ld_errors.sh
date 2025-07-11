#!/bin/sh

set -e

SRC_TREE="../../../sys/contrib/dev/acpica"
DEST_DIR="../contrib/dev/acpica"
MAKEFILE_PATH="Makefile"
CALL_GRAPH="tree.txt"

echo "Starting autofix ld errors..."

while :; do
	# Run make and capture the first linker error
	sudo make -j4 TARGET=amd64 2>&1 | tee build.log > /dev/null
	missing_func=`awk '/ld: error:/ {print; exit}' build.log \
		| grep -oE '[A-Za-z_][A-Za-z0-9_]*$'`
	
	if [ -z "$missing_func" ]; then
		echo "No missing function symbol found in linker errors." \
			| tee -a build.log
		break
	fi
	
	echo "Missing function: $missing_func" | tee -a build.log

	
	# Find the function's filename to later be added to the Makefile
	match=`grep -Erl --include="*.c" \
		"\* FUNCTION:[[:space:]]*$missing_func([[:space:]]|\$)" "$SRC_TREE" \
		| head -n1`
	
	if [ -z "$match" ]; then
		echo "Function implementation for $missing_func not found." \
			| tee -a build.log
		break
	fi
	
	# Extract source file path from grep output
	src_file=`echo "$match" | cut -d: -f1`
	
	# Only automatically add the function if it is
	# present in our call graph. If not, then we 
	# query the user on how to proceed.
	if grep -qF "$missing_func" "$CALL_GRAPH"; then
		echo "Found: $missing_func"
	else
		echo "$missing_func not in graph."
		echo "It is found in $src_file."
		echo "It is referenced by:"
		awk '
		/^ld: error: undefined symbol:/ && !found {
		    found = 1
		    next
		}
		found && /^ld: error:/ { exit }
		found && /\(/ {
		    line = $0
		    sub(/^.*\(/, "", line)
		    sub(/\).*$/, "", line)
		    print line
		}
		' build.log
		while true; do
		    printf "Continue putting it in anyway? (y/n): "
		    read ans
		    case "$ans" in
		        y|Y)
		            echo "Continuing..."
		            break
		            ;;
		        n|N)
		            echo "Aborting."
		            exit 1
		            ;;
		        *)
		            echo "Please type 'y' or 'n'."
		            ;;
		    esac
		done
	fi

	echo "$src_file" | tee -a build.log
	
	# Extract the filename, insert it into Makefile
	filename=$(basename "$src_file")

	echo "File: $filename" | tee -a build.log

	if [ ! -f "$MAKEFILE_PATH" ]; then
	    echo "Makefile not found: $MAKEFILE_PATH"
	    break
	fi

	if grep -q "[[:space:]]$filename[[:space:]]*\\\\" "$MAKEFILE_PATH"; then
	    echo "$filename already present in Makefile."
	    continue
	fi

	# Prefix of file --- easiest way to find where to insert it into Makefile
	prefix=`echo "$filename" | cut -c1-2`
	echo "Prefix: $prefix" | tee -a build.log

	# Capture pattern for line number of first occurence of file in 
	# Makefile SRCS+= that has the same scope of the file we are inserting
	pattern='^[[:space:]]*'"${prefix}"'[A-Za-z0-9_]*\.c[[:space:]]*\\$'
	
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

	echo "Successful iteration for $filename. Moving onto next." \
		| tee -a build.log
	echo "Press crtl+c to stop. Otherwise, script will continue..."
	sleep 1
	echo "...In 3..."
	sleep 1
	echo "...2..."
	sleep 1
	echo "...1..."
done
