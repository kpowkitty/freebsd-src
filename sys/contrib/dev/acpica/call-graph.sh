#!/bin/sh

# example start command
# ./call-graph.sh AcpiNsRootInitialize 0

usage() {
	echo "Usage: $0 <function> <level_in_tree (usually 0)>"
	exit 1
};

if [ "$#" -ne 2 ]; then
	usage "$0"
fi

function="$1"
level="$2"
visited_file=".visited_functions"

# Initialize visited file if not exists
if [ ! -f "$visited_file" ]; then
	touch "$visited_file"
fi

# Check if function was already visited
if grep -qxF "$function" "$visited_file"; then
	echo "Already visited function $function, skipping to avoid cycles" \
		| tee -a skip.log > /dev/null
	exit 0
fi

clean_fn=`echo "$function" | sed 's/()[[:space:]]*$//'`

# Mark function as visited
echo "$clean_fn" >> "$visited_file"

# Find file containing the function
# ACPI format is "* FUNCTION:(spaces)$function"
file=`grep -rlE --include='*.[ch]' "\* FUNCTION:[[:space:]]+$clean_fn" .`

if [ -z "$file" ]; then
	# Occasionally, there is not Intel documentation for it, so
	# we need to change how we grep, but we only want to do this
	# when we can't find it the first way because this is much
	# more error prone... XXX
	file=`grep -rlE --include='*.c' "^$clean_fn \($" .`
	if [ -z "$file" ]; then
		echo "Line 54: Function $clean_fn not found in any source file." \
			| tee -a lost.log > /dev/null
		exit 1
	fi
fi

# Run cflow on current function (i.e., get its callees), suppress errors
calls=`cflow --main "$clean_fn" "$file" 2>/dev/null | awk '{print $NF}'`

for fn in $calls; do
	case "$fn" in
		*ACPI*|*return*)
			# These are macros we don't need to include for this
      		echo "Skipping $fn" | tee -a skip.log > /dev/null
      		;;
    	*Acpi*)
      		indent=`printf '%*s' $((level * 2)) ''`

			# Find the file this function is in
			file=`grep -rlE --include='*.[ch]' "\* FUNCTION:[[:space:]]+$fn" .`
			if [ -z "$file" ]; then
				# Occasionally, there is not Intel documentation for it, so
				# we need to change how we grep, but we only want to do this
				# when we can't find it the first way because this is much
				# more error prone... XXX
				file=`grep -rlE --include="*.c" "^$fn \($" .`
				if [ -z "$file" ]; then
					echo "Line 54: Function $fn not found in any source file." \
						| tee -a lost.log > /dev/null
					exit 1
				fi
			fi
			clean_fn=`echo "$fn" | sed 's/()[[:space:]]*$//'`
      		echo "${indent}File: [$file], Function: [$clean_fn]." \
				| tee -a tree.txt > /dev/null
      		next_level=$((level + 1))
      		./call-graph.sh "$clean_fn" "$next_level"
      		;;
		*.c:[0-9][0-9][0-9]*\>*)
			echo "Caught filepath: $fn" | tee -a path.log > /dev/null
			# The case where cflow prints the file & line number
			# that the function is at
      		indent=`printf '%*s' $((level * 2)) ''`
	  		
			# Extract file and line number
    		file=`echo "$fn" | grep -oE '[^[:space:]]+\.c'`
    		lineno=`echo "$fn" | grep -oE '\.c:[0-9]+' | cut -d: -f2`
			
			# DEBUG
			echo "File: $file" | tee -a debug.log > /dev/null
			echo "Lineno: $lineno" | tee -a debug.log > /dev/null
    		
			# Grep backwards from lineno to find function definition
    		line=`sed -n "${lineno}p" "$file"`
			echo "$line" | tee -a debug.log > /dev/null

    		# If found, extract function name (trim function of '(')
    		if [ -n "$line" ]; then
    		  	funcname=`echo "$line" | sed -n 's/[[:space:]](//p'`

				if grep -qxF "$funcname" "$visited_file"; then
					echo "Already visited function $funcname, skipping" \
						| tee -a skip.log > /dev/null
					continue
				fi
				
				# Mark it as visited and add it to our tree
				echo "$funcname" | tee -a "$visited_file" > /dev/null
    		  	echo "${indent}File: [$file], Function: [$funcname]." \
					| tee -a tree.txt > /dev/null
      			next_level=$((level + 1))
      			./call-graph.sh "$funcname" "$next_level"
    		else
				echo "$line not found." | tee -a error.log > /dev/null
				exit 1
    		fi
	  		;;
		*)
			echo "Error. Did not catch $fn." | tee -a error.log > /dev/null
			;;
	esac
done
