# 1. Frage:


Aus wievielen Bit besteht ein ``int``?

---

Meist 32 oder 64 (nicht immer gleich!)


<br><br><br>


# 2. Frage:


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


<br><br><br>


# 3. Frage:


Mit welchen Befehlen wird ein Shared Memory Objekt wieder freigegeben?

---

```c
munmap(my_shm, sizeof(*myshm));
shm_unlink(SHM_NAME);
close(shm_fd);
```


<br><br><br>


# 4. Frage:


Mit welchen Befehlen wird eine Semaphore wieder freigegeben?


---

```c
sem_close(my_sem);
sem_unlink(my_sem);
```


<br><br><br>


# 5. Frage:


Um mit einem Subprozess per ``stdin`` und ``stdout`` zu kommunizieren, werden wieviele Unnamed Pipes benötigt?

---

2 (jeweils für Lesen und Schreiben, Pipes sind unidirektional)


<br><br><br>


# 6. Frage:


Unterschied zwischen Deklaration und Definition

---

**Definition:** Speicher wird für die Variable reserviert.

**Deklaration:** Variable wird benannt und typisiert. Jede Deklaration ist auch eine Definition.

```c
int i; // deklariert und definiert
extern int j; // deklariert aber woanders definiert
int k = 23; // deklariert, definiert und initialisiert
```


<br><br><br>


# 7. Frage:


Was bedeutet das Signal ``SIGCHILD``?

---

Kindprozess terminiert oder wurde gestoppt.


<br><br><br>


# 8. Frage:


Was bedeutet das Signal ``SIGPIPE``?

---

Es wird versucht in eine Pipe zu schreiben, deren Leseende geschlossen ist


<br><br><br>


# 9. Frage:


Was bedeutet Definition?


---


Speicher wird für die Variable reserviert und ihr Name festgelegt.

```c
int i; // deklariert und definiert
int func(void) { ... } // funktion deklariert und definiert

extern int j; // deklariert aber woanders definiert
int func(void); // NUR deklariert

int k = 23; // deklariert, definiert und initialisiert
```


<br><br><br>


# 10. Frage:


Was bedeutet Deklaration?

---


Einer Variable wird ein Typ zugewiesen.

```c
int i; // deklariert und definiert
int func(void) { ... } // funktion deklariert und definiert

extern int j; // deklariert aber woanders definiert
int func(void); // NUR deklariert

int k = 23; // deklariert, definiert und initialisiert
```


<br><br><br>


# 11. Frage:


Was bedeutet der Prozesszustand ``zombie``?

---

Prozess hat terminiert, aber Status ist noch nicht beim Elternprozess angekommen.


<br><br><br>


# 12. Frage:


Was bedeutet ``inline`` bei einer Funktionsdefinition?


---

Hint für den Compiler, dass dieser Code schnellstmöglich ausgeführt werden soll.


<br><br><br>


# 13. Frage:


Was bedeutet
```c
(*(void(*)())x)();
```

---

Eine Funktion wird aufgerufen, deren Adresse auf dem Speicherwert ``x`` liegt.


<br><br><br>


# 14. Frage:


Was befindet sich im `/dev` Verzeichnis?

---

Gerätedateien


<br><br><br>


# 15. Frage:


Was befindet sich im `/etc` Verzeichnis?

---

Konfigurationsdateien


<br><br><br>


# 16. Frage:


Was befindet sich im `/proc` Verzeichnis?

---

Virtuelles Dateisystem


<br><br><br>


# 17. Frage:


Was befindet sich im `/sbin` Verzeichnis?

---

Wesentliche Systemkommandos


<br><br><br>


# 18. Frage:


Was ergibt der Aufruf ``ps``?

---

Liste der aktiven Prozesse (Tasklist)


<br><br><br>


# 19. Frage:


Was ist das Ergebnis vom folgenden Aufruf
```c
int a, b = 32;
a = b;
printf("%d\n", a);
```

---

32


<br><br><br>


# 20. Frage:


Was ist der Unterschied zwischen ``()`` und ``(void)`` als Funktionsparameter?


---

``int foo()`` nimmt eine unbestimmte Zahl an Parameter
``int foo(void)`` nimmt keinen Parameter


<br><br><br>


# 21. Frage:


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


<br><br><br>


# 22. Frage:


Was ist die Aufgabe des Schedulers?

---

Wählt nächsten Prozess, wenn sich mehrere die CPU teilen müssen, arbeitet nach dem Scheduling Algorithmus. Ziel: Rechenzeit der CPU optimal nutzen


<br><br><br>


# 23. Frage:


Was ist die Auswertereihenfolge von
```c
if (flag & asdf != qwer) { ... };
```

---

```c
if (flag & (asdf != qwer)) { ... };
```


<br><br><br>


# 24. Frage:


Was ist die Auswertungsreihenfolge von
```c
r = h << 4 + 1
```


---


```c
r = h << 5;
```


<br><br><br>


# 25. Frage:


Was ist ``errno``?

---

Globale Variable die den aktuellen Fehlercode beinhaltet.
Kann durch ``strerror`` mit Beschreibung versehen werden.


<br><br><br>


# 26. Frage:


Was kann in UNIX alles ein File sein?

---

- Directory
- Socket
- **Named** Pipes
- Symbolic Links
- Character Special Files (für Zugriff auf zeichenorientierte Geräte, zB Modem oder Drucker)
- Block Special Files (für Zugriff auf externe Geräte bei denen Daten in Blocks fixer Größe gespeichert werden, zB Festplatte)


<br><br><br>


# 27. Frage:


Was passiert bei dynamischer Codeanalyse?

---

Programm wird während des Ablaufs geprüft. 
Tool: valgrind


<br><br><br>


# 28. Frage:


Was passiert bei statischer Codeanalyse?

---

Code durschauen, Compilerwarnungen auswerten. 
Tool: z.B. splint


<br><br><br>


# 29. Frage:


Was passiert bei ``sem_post(my_sem)``?


---

Der Wert von ``my_sem`` wird inkrementiert. So signalisiert ein Prozess, dass er fertig ist mit der zu synchronisierenden Ressource.


<br><br><br>


# 30. Frage:


Was passiert bei ``sem_wait(my_sem)``?


---

Der Wert von ``my_sem`` wird dekrementiert. Wenn dieser danach ``0`` ist kann der Prozess zugreifen, ist dieser ``< 0`` wird blockiert.


<br><br><br>


# 31. Frage:


Was passiert beim Aufruf von ``bind(sock_fd, addr, addrlen)``?

---


Der Socket ``sock_fd`` wird die Adresse in ``addr`` zugewiesen.


<br><br><br>


# 32. Frage:


Was passiert beim Aufruf von ``ftruncate()``?

---

Die Größe der spezifizierten Datei wird gesetzt.


<br><br><br>


# 33. Frage:


Was passiert beim Aufruf von ``listen(sock_fd, 1)``?


---

Der Server wartet auf eingehende Verbindungen. Es ist maximal eine Verbindung erlaubt (sog. backlog). Verbindet sich ein zweiter Client wird die Verbindung verweigert.


<br><br><br>


# 34. Frage:


Was passiert beim Aufruf von ``mmap()``?

---


Dateien werden in den virtuellen Speicher des Prozesses abgebildet. Dadurch kann Lesen und Schreiben effizienter gemacht werden. Ermöglicht Interprozeskommunikation über Shared Memory.


<br><br><br>


# 35. Frage:


Was retourniert der folgende Aufruf?
```c
malloc(sizeof (char));
``` 


---


Die Startadresse des reservierten Speichers


<br><br><br>


# 36. Frage:


Was sind Pointer?

---

Pointer zeigen auf eine Adresse wodurch Variableninhalte angesprochen werden können.

```c
int *a;

a // Inhalt von a (eine Adresse)
*a // Inhalt auf den a zeigt
&a // Adresse von a
```


<br><br><br>


# 37. Frage:


Was sind positional arguments?

---

Nachdem alle Command Line Optionen abgearbeitet wurden, zeigt ``optind`` auf das erste positional argument

```c
./hello -a optarg arg1 arg2
// nach getopt()
argc = 5, optind = 3
```


<br><br><br>


# 38. Frage:


Was sind Signale?


---


Asynchrone Ereignisse, bewirken Unterbrechung des Programmablaufs, können extern Variablen verändern.


<br><br><br>


# 39. Frage:


Was sind Terminals?

---

Kernel hat viele pyhsische und virtuelle Terminals, jeder Prozess ist über Pipes mit einem Terminal lokal (ttyX) oder remote (ptsX) verbunden.


<br><br><br>


# 40. Frage:


Was sind Unnamed Pipes?


---


``prog1 | prog2`` verbinden zwei Prozesse mit einem unidirektionalen FIFO Kanal


<br><br><br>


# 41. Frage:


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


<br><br><br>


# 42. Frage:


Was sind ``unions``?

---


Im Gegensatz zu einem ``struct`` kann bei einer ``union`` lediglich ein Element einen Wert beinhalten. Der reservierte Speicherplatz ist so groß um das größte Element beinhalten zu können.


<br><br><br>


# 43. Frage:


Was wird in ``argc`` gespeichert?


---

Anzahl der Argumente in ``argv``


<br><br><br>


# 44. Frage:


Was überprüft die folgende Codezeile?

```c
if ((argc - optind) != 2) { ... }
```

---


Überprüft ob genau zwei Positional Arguments übergeben wurden.


<br><br><br>


# 45. Frage:


Welche Arten von `exec()` gibt es?

---

`p` -> sucht in `$PATH` nach Programmnamen
`l` -> variable Argumentenanzahl
`v` -> Argumentarray


<br><br><br>


# 46. Frage:


Welche Aufgaben übernimmt der Präprozessor?


---

Einfache Ersetzungen im Code werden durchgeführt vor der Kompilierung.


<br><br><br>


# 47. Frage:


Welche Möglichkeiten gibt es einen Prozess zu erstellen?

---

fork oder clone


<br><br><br>


# 48. Frage:


Welche Phasen durchläuft die Kompilation in C?


---



Code -> Pre-processor -> Compiler -> Linker


<br><br><br>


# 49. Frage:


Welche Variablen kommen in Makefiles vor?


---


`CC` - C Compiler
`CFLAGS` - C Compiler Optionen
`LDFLAGS` - Linker Optionen


<br><br><br>


# 50. Frage:


Welcher Block wird hier ausgeführt:

```c
if (-1) {
/* execute codeblock 1 */ 
} else {
/* execute codeblock 2 */
}
```

---


**Codeblock 1**
In C gilt ``0`` ist ``false`` - alle anderen Werte sind ``true``.


<br><br><br>


# 51. Frage:


Welcher Wert befindet sich immer in ``argv[argc]``?

---

``NULL``


<br><br><br>


# 52. Frage:


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


<br><br><br>


# 53. Frage:


Wie entsteht ein Orphan Prozess?

---

Elternprozess terminiert während der Kinderprozess weiterläuft.


<br><br><br>


# 54. Frage:


Wie hängen `fflush()` und `fclose()` zusammen?

---

`fclose()` ruft `fflush()` auf welches das Schreiben gepufferter Daten erzwingt. Danach wird Stream und File Descriptor geschlossen.


<br><br><br>


# 55. Frage:


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


<br><br><br>


# 56. Frage:


Wie ist ein UNIX Kernel aufgebaut?

---

- **Process Management Component:**
Signal Handling, Process/Thread Erstellung/Beendigung, CPU Scheduling

- **Memory Management Component:**
Virtual Memory, Paging, Page Replacement und Cache

- **I/O and Networking Component:**
(Virtual) File System, Terminals, Sockets


<br><br><br>


# 57. Frage:


Wie ist ein UNIX System aufgebaut?

---

- Hardware (CPU, Speicher, ...)
- Software (OS: Kernel + Treiber, Benutzerprogramme) 
- Mehrbenutzer­ und Mehrprozessbetriebssystem


<br><br><br>


# 58. Frage:


Wie kann ein Hintergrundprozess von der Shell abgesetzt werden?

---

Befehl mit ``&`` am Ende abgeben


<br><br><br>


# 59. Frage:


Wie kann ein Prozess auf ein Signal reagieren?

---

- ausführen einer Funktion
- ignorieren des Signals
- default Aktion


<br><br><br>


# 60. Frage:


Wie kann Output von ``stderr`` umgeleitet werden?

---

``program 2> output.txt``


<br><br><br>


# 61. Frage:


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


<br><br><br>


# 62. Frage:


Wie können Programme in UNIX gestartet werden?


---


**Indirekt** über die Shell oder
**Direkt** über system call ``(exec*)``


<br><br><br>


# 63. Frage:


Wie ruft man die Manpage von ``getopt(3)`` auf?

---

```bash
man 3 getopt
```


<br><br><br>


# 64. Frage:


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


<br><br><br>


# 65. Frage:


Wie verändert das ``extern`` Attribut eine Variable?

---

Die durch ``extern`` deklarierte Variable wird in einer anderen Datei definiert.


<br><br><br>


# 66. Frage:


Wie verändert das ``static`` Attribut eine Variable?


---

Ihr wird ein fixer Speicherplatz zugewiesen und der Zustand beibehalten.


<br><br><br>


# 67. Frage:


Wie verändert das ``volatile`` Attribut eine Variable?

---

Die Variable kann außerhalb des Programmkontexts verändert werden (Interrupt handling).

```c
volatile char keyPressed = ’ ’; long count = 0;
while (keyPressed != ’x’) {
++count; }
```

Ohne ``volatile`` würde der loop vom Compiler zu ``while(1)`` optimiert werden, da die Variable (aus Compilersicht) nie den wert verändern würde.


<br><br><br>


# 68. Frage:


Wie werden Prozesse verwaltet?

---

Prozesstabelle in Linux besteht aus ``task_struct``, 1 Eintrag pro laufendem Prozess


<br><br><br>


# 69. Frage:


Wie werden Rechte und Ressourcen eines neuen Prozesses bestimmt?


---

Werden vom aufrufenden Prozess geerbt.


<br><br><br>


# 70. Frage:


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


<br><br><br>


# 71. Frage:


Wie wird nach einem ``fork()`` zwischen Eltern- und Kindprozess unterschieden?


---


Die PID eines Kindprozesses ist immer ``0``.


<br><br><br>


# 72. Frage:


Wie wird ``p -> uid`` ausgewertet?

---

```c
(*p).uid
```


<br><br><br>


# 73. Frage:


Wieso sollten Elemente in einer ``struct`` der Größe nach geordnet werden?


---


Manche Compiler verändern bei der Optimierung die Reihenfolge der Variablen wodurch bei Shared Memory Zugriffen auf den falschen Speicher zugegriffen wird.


<br><br><br>


# 74. Frage:


Wo existieren Prozesse?


---

**Kernel Space** - Process Descriptor
und
**User Space** - Datensegment, Programmcode


<br><br><br>


# 75. Frage:


Wodurch unterscheiden sich ``*.h`` und ``*.c`` Dateien?


---


Modul wird in Header (.h) und Implementierung (.c) unterteilt wobei im Header Konstanten und Prototypen definiert werden.

Header beinhalten **keine** Definitionen.


<br><br><br>


# 76. Frage:


Wofür werden Protoypes verwendet?


---

Prototypen sind Deklarationen von Funktionen. Die Definition einer Funktion folgt nach dem Aufruf.


<br><br><br>


# 77. Frage:


Wozu dienen Makefiles?

---

Automatisierung der Programmerstellung, Spezifizierung der Dependencies.


<br><br><br>


