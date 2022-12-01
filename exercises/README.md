The pool of OSUE exercises / assignments / specifications.

Topics include (but are not limited to):
* C programming basics
* Sockets
* Process creation (fork, exec)
* Pipes
* Semaphores
* Shared memory


### Generating pdf files:

To generate the pdf documents, you must first latex.

The script `genpdfs.sh` is provided for convenience. It recursively runs latexmk
for all .tex files it finds in the current working directory and all subdirectories.

Run the script:
```bash
$ chmod +x ./genpdfs.sh
$ ./genpdfs.sh
```
