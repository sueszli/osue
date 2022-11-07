#!/bin/bash

make ispalindrom
printf "<built project>\n\n\n"

./ispalindrom
printf "\n\n\n<executed project>\n"


make clean

git add . 
git commit -m "automatized push to github"
git push 
