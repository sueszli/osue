# Frage 1:

Sie haben folgenden Code-Teil gegeben(nehmen Sie als System die Labor-Umgebung an).

Inhalt der Datei, welche mit `file` geöffnet wurde ist “hello”.

```c
int i;
char c;
while((i = getc(file) != EOF) {
	c = (char) i;
	printf("%s", &c);
}
```

Welche Aussagen treffen zu? (es können mehrere zutreffen)

1. Das Programm bricht mit einem Fehler (zB. SIGSEGV) ab.
2. Ausgabe: “hello”
3. Ausgabe: “hheellloo”
4. Ausgabe: “h”
5. Das Programm durchläuft diesen Teil ohne Fehler (zB. SIGSEGV).
6. Ausgabe “hh”

<br><br><br>

# Frage 2:

Welche Argumente muss man `getopt()` übergeben?

```c
getopt(..., ..., ..., C);
```

Zur Auswahl für die ersten beiden Argumente stehen:

1. `foo`
2. `argc`
3. `argv`
4. `-1`
5. `NULL`
6. `argopt`

Zur Auswahl für das letzte Argument stehen:

1. `"p:"`
2. `"p?"`
3. `"p!"`

<br><br><br>

# Frage 3:

Welche der folgenden Aussagen über eine Deklaration treffen zu? (es können mehrere zutreffen)

1. Jede Deklaration ist auch eine Definition
2. Bei der Deklaration wird dem Compiler der Typ eines Symbols bekannt gegeben
3. Eine Deklaration darf mehrmals vorkommen
