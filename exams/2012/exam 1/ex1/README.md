I wasn't able to link and run `list.o` with listtool.o 

I ran the Makefile on the following architecture: <...> 5.10.102.1-microsoft-standard-WSL2 <...> x86_64 x86_64 x86_64 GNU/Linux

I then got the following error messages:
```
gcc -fPIE -fPIC -c -o listtool.o listtool.c 
gcc -o listtool list.o listtool.o -lcrypt
/usr/bin/ld: list.o: relocation R_X86_64_32 against `.rodata' can not be used when making a PIE object; recompile with -fPIE
/usr/bin/ld: failed to set dynamic section sizes: bad value
collect2: error: ld returned 1 exit status
make: *** [Makefile:10: listtool] Error 1
```

Also the `test.sh` test cases behave wrongly.

In conclusion: You can attempt to guess what the code behind list.o looks like by disassembling it, but I would suggest you practice for your exam by solving other problems.