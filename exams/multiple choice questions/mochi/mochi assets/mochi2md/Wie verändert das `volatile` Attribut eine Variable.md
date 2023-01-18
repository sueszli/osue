Wie verändert das ``volatile`` Attribut eine Variable?
---
Die Variable kann außerhalb des Programmkontexts verändert werden (Interrupt handling).

```c
volatile char keyPressed = ’ ’; long count = 0;
while (keyPressed != ’x’) {
++count; }
```

Ohne ``volatile`` würde der loop vom Compiler zu ``while(1)`` optimiert werden, da die Variable (aus Compilersicht) nie den wert verändern würde.