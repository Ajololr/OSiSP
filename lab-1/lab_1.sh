#!/bin/bash
find "$3" -size +"$1"c -size -"$2"c -type f -printf '%p %s \n' 2>/tmp/errors.txt | head -n 20
while read line; do
echo "$(basename $0): ${line}">&2
done < /tmp/errors.txt
