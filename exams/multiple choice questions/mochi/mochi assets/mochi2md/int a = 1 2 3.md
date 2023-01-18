```c
int a[] = {1, 2, 3};
int* p = &a[0];
```
Was retournieren `sizeof(a)`, `sizeof(p)` und `sizeof(*p)`?
---

```c
sizeof(a) = 3*sizeof(int); // ganzes array
sizeof(p) = sizeof(*int); // pointer 
sizeof(*p) = sizeof(int); // erster wert des arrays
```