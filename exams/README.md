## Read me if the executables you found didn't run 🤖

Some previous exams come with an executable that you might not be able to / might not want to run on your machine because of security concerns.

Last time I tried, I was able to run them on the Inflab server running "Red Hat 7. x, CentOS 7." on a 64-bit x86 processor.

``` bash
$ uname -a 
# Linux apps1.inflab.tuwien.ac.at 3.10.0-1160.81.1.el7.x86_64 #1 SMP Fri Dec 16 17:29:43 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux
```

The executables themselves are also compiled for 64-bit x86 processors and GNU/Linux 2.6.18.

``` bash
$ file ./server 
# ./server: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked (uses shared libs), for GNU/Linux 2.6.18, BuildID[sha1]=f54643b8a73b54d90d7bf6bcdebfc92f80e2aa5a, not stripped
```

---

If you can't run the executables from the previous exams on your own machine you can:

- reverse-engineer the code by using Ghidra or your favorite disassembler and rewriting the source code
- use a virtual machine
- or just run them via ssh on your Inflab account
