Wie ist die schematische Reihenfolge der aufgerufenen Funktionen für eine passive Socket?
---
```c

socket(); // socket file descriptor wird erstellt mit infos aus getaddrinfo()
bind(); // adresse wird socket zugewiesen
listen(); // auf eingehende Verbindungen wird gewartet
accept(); // Verbindung wird akzeptiert

/* Endlosschleife */
read(); // Anfrage wird gelesen
write(); // Antwort wird in socket geschrieben
/* Endlosschleife */

close(); // server wird beendet
```