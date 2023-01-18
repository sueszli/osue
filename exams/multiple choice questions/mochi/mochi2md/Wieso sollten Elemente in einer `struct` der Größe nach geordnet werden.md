Wieso sollten Elemente in einer ``struct`` der Größe nach geordnet werden?

---

Manche Compiler verändern bei der Optimierung die Reihenfolge der Variablen wodurch bei Shared Memory Zugriffen auf den falschen Speicher zugegriffen wird.