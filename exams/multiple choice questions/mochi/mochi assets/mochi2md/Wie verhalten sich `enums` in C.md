Wie verhalten sich ``enums`` in C?

---
Werden genutzt um Aliasse zu erstellen. Wenn nicht anders spezifiziert erhält das erste Element den Wert 0, und alle darauffolgenden werden inkrementiert.

```c
enum boolean {FALSE, TRUE};
/*
FALSE = 0
TRUE = 1 
*/
```