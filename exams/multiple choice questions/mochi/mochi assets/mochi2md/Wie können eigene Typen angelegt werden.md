Wie können eigene Typen angelegt werden?

---

Durch den ``typedef`` Operator.

```c

struct account { 
char username [32];
char password [32];
unsigned int uid;
};

typedef struct account account_t;
account_t user1 = {"alice", "al1c3", 42};
```