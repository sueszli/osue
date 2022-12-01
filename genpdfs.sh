#!/bin/bash

# generate pdfs
sudo apt-get install latexmk
find . -name '*.tex' -execdir latexmk -pdf -g -silent {} \;

# remove everything else
find . -name '*.aux' -execdir rm {} \;
find . -name '*.fdb_latexmk' -execdir rm {} \;
find . -name '*.fls' -execdir rm {} \;
find . -name '*.log' -execdir rm {} \;
find . -name '*.out' -execdir rm {} \;
