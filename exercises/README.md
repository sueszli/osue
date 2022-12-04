The pool of OSUE exercises / assignments / specifications.

Topics include (but are not limited to):
* C programming basics
* Sockets
* Process creation (fork, exec)
* Pipes
* Semaphores
* Shared memory

### Generating pdf files

To generate the pdf documents, you must first latex.

The script `genpdfs.sh` is provided for convenience. It recursively runs latexmk
for all .tex files it finds in the current working directory and all subdirectories.

Run the script:
```bash
$ chmod +x ./genpdfs.sh
$ ./genpdfs.sh
```

### Which ones to choose

Most sophisticated ones: 1/a/postfixcalc, 1/b/mastermind, 2/forksort, 3/battleships.

Current first exercises in the official lecture:

- ispalindrom
- mycompress
- mydiff
- myexpand
- mygrep
- fb_arc_set
- 3coloring

Current second exercises in the official lecture:

- cpair
- forksort
- forkFFT
- intmul

Current third exercises in the official lecture:

- http

Bonus exercise (competition):

- secvault


