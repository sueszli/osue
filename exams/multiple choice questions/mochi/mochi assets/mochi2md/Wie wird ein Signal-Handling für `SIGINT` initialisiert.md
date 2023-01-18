Wie wird ein Signal-Handling für ``SIGINT`` initialisiert?
---

```c
struct sigaction sa;
sa.sa_handler = signal_handler;
sigaction(SIGINT, &sa, NULL);

/* ... */

void signal_handler(int sig) {
  /* veränderung von variablen, print von messages, etc. */
}
```