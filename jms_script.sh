#!/bin/bash

path=""
command=""
i=0

# loop through parameter list
for arg
	do
	let "i=i+1"
	if [ "$arg" = "-l" ]; then
		j=$((i+1))
		path=${!j}
	fi

	if [ "$arg" = "-c" ]; then
		j=$((i+1))
		command=${!j}
		j=$((i+2))
		n=${!j}
	fi
	done

# check if argument were given
flag=1
if [ -z "$path" ]; then
	echo You did not enter path;
	flag=0
fi
if [ -z "$command" ]; then
	echo You did not enter command;
	flag=0
fi
if [ "$flag" -eq 0 ]; then
	echo "Usage:./jms script.sh -l <path> -c <command>";
	echo;
	exit 1
fi

#echo arguments
echo
echo "Path   : \"$path\""
echo "Command: \"$command\""
echo n: $n
echo

#execute the command given
case $command in
	"list")
		for dir in "$( find $path -mindepth 1 -type d )"
		do
			echo "$dir"
		done;;
	"size")
		case $n in
			''|*[!0-9]*)
				du -h $path | sort -nr | awk '{print $2"\t"$1}' | tail -n+2 ;;
			*)	du -h $path | sort -nr | awk '{print $2"\t"$1}' | tail -n+2 | head -$n ;;
		esac;;
	"purge")
		echo Deleting every directory starting with \"sdi1400136_\"
		rm -rf $path/sdi1400136_*;;
	*)echo Invalid command !;;
esac
