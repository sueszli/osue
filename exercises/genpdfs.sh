#!/bin/bash

# remove all existing pdfs, then generate pdfs
sudo apt-get install latexmk
find . -name '*.pdf' -execdir rm {} \;
find . -name '*.tex' -execdir latexmk -pdf -g -silent {} \;

# remove byproducts of pdf generation
find . -name '*.aux' -execdir rm {} \;
find . -name '*.fdb_latexmk' -execdir rm {} \;
find . -name '*.fls' -execdir rm {} \;
find . -name '*.log' -execdir rm {} \;
find . -name '*.out' -execdir rm {} \;
