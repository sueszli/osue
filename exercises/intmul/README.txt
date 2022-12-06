DO THE DOCUMENTATION SOMEWHERE ELSE AND AT THE END!!! NOT IN THE SOURCE CODE!!!


The hexadecimal integer multiplication is done recursively.

1) Base case: print product of single digit multiplication to stdout.

2) General case: fork 4 children, each responsible for one product.

    a · b =
      + aH · bH · 16^n       -> done by 'HH' child
      + aH · bL · 16^(n/2)   -> done by 'HL' child
      + aL · bH · 16^(n/2)   -> done by 'LH' child
      + aL · bL              -> done by 'LL' child

  Each of the 4 children requires 2 pipes:
    parent to child pipe (p2c)  
    child to parent pipe (c2p)

  We write into the p2c pipes and then wait for a response in the c2p pipes.
  Then we add the 4 responses from the children together and print them on stdout.
