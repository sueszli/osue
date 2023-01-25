## About this exam

80 points are acheivable in total consisting of:

- 30 points for the theoretical questions
- 50 points for the coding exercise (10 + 20 + 20 for each question)

Point deductions for wrong answers on theoretical questions:

- the theoretical questions are asked in a multiple choice format (each question can have one or more correct answers)
- for each wrong answer on an individual question, points are deducted from the theoretical part only but not from the coding exercise
- you can't get less than 0 points for the theoretical questions in total

Total number of theoretical questions: 15

<br><br><br>

## Theoretical questions

- What is the difference between UDP and TCP? 
- Strings in C end with null characters?
- Initial value of local and global variables?
- How preprocessing works?

<br><br><br>

## Coding exercise

### 1. create a socket as a server

Create a passive socket of domain `AF_INET` and type `SOCK_STREAM`.
Listen for connections on the port given by the argument `port_str`.
Return the file descriptor of the communication socket `int setup_connection(const char *port_str)`.

The file descriptor will be used in the next step!

### 2. accept connections from the created socket as a server

Wait for connections on the received socket file descriptor.

Read the argument transmitted by the client from the connection and sav



### 3. send around files with a forked child
