If you run all exercises that depend on an object file then you might get an error message like I did.
I ran the Makefiles on the following architecture: <...> 5.10.102.1-microsoft-standard-WSL2 <...> x86_64 x86_64 x86_64 GNU/Linux

I then got the following error messages:
```
gcc -fPIE -fPIC -c -o listtool.o listtool.c 
gcc -o listtool list.o listtool.o -lcrypt
/usr/bin/ld: list.o: relocation R_X86_64_32 against `.rodata' can not be used when making a PIE object; recompile with -fPIE
/usr/bin/ld: failed to set dynamic section sizes: bad value
collect2: error: ld returned 1 exit status
make: *** [Makefile:10: listtool] Error 1
```

I also did some research and used the `-fPIE` flag for compilation but it didn't make any difference.
I don't know how to fix this problem.

In conclusion: You can attempt to guess what the code behind the given object file looks like by disassembling it and writing everything from scratch such that it can be compiled to run on any machine (but I don't think it is worth the effort and suggest you practice for your exam by solving other problems instead).
