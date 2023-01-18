Welcher Wert kann hier nicht verändert werden?
```c
int num;
char *const ptr = &num;
```

---

Der Wert auf den `ptr` zeigt kann verändert werden, der Pointer jedoch nicht.

Dementsprechend gilt:
```c
num = 42;
char *const ptr = &num;
printf("num: %d\n", *ptr); // "num: 42"
num = 99;
printf("num: %d\n", *ptr); // "num: 99"
```

aber

```c
num = 42; num2 = 99;
char *const ptr = &num;
ptr = &num2; // error: assignment of read-only variable ‘ptr’
```