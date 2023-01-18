Was sind ``structs``?

---

Fassen mehrere Variablen zu einer logischen Einheit zusammen deren Größe der Summe der einzelnen Elemente entspricht.

```c
struct account {
char username [32];
char password [32];
unsigned int uid;
};

struct account user1 = {.uid=1, .username="alice", .password="4l1c3"};
```