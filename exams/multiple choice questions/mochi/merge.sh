#!/bin/bash

IFS=$'\n'

input="./mochi2md"

files=$(find $input -name "*.md")
counter=1

# Loop through each file and print contents
for file in $files
do
    echo -e "# $counter. Frage:\n\n"
    # echo -e "#" $(basename $file)
    cat $file
    echo -e "\n\n\n<br><br><br>\n\n"
    ((counter++))
done

