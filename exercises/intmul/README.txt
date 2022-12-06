The hexadecimal integer multiplication is done recursively.

1) Base case: print product of single digit multiplication to stdout.

2) General case: fork 4 children, each responsible for one product.

    a · b =
      + aH · bH · 16^n       -> done by 'HH' child
      + aH · bL · 16^(n/2)   -> done by 'HL' child
      + aL · bH · 16^(n/2)   -> done by 'LH' child
      + aL · bL              -> done by 'LL' child

  Each of the 4 children requires 2 pipes.
  The 8 file descriptores for the pipes are stored in pipefd[].
  Write into the p2c pipes and wait then for a response in the c2p pipes.
  Then add the 4 responses from the children together and print on stdout.
