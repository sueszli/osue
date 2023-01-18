Was ist der Unterschied zwischen
```c
const int i = 23;
```
und
```c
#define MYCONST 23;
```
---
``const`` definiert eine typisierte Konstante im Code die nicht verändert werden kann. Durch ``#define`` definierte Werte werden vom Präprozessor erfasst und im Code ersetzt.