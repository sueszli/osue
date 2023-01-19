# 1. Frage:

Was ist der Unterschied zwischen Linken und Compilieren?
Warum ist es sinnvoll diese Vorgänge zu trennen?

--- 

Der Unterschied zwischen Linken und Compilieren besteht darin, dass Compilieren den Quellcode eines Programms in Maschinencode übersetzt, während Linken die aus verschiedenen Teilen bestehenden Objektdateien eines Programms zu einer ausführbaren Datei zusammenfügt. Compilieren wandelt also den Quellcode in eine ausführbare Form um, während Linken mehrere Objektdateien zu einer einzigen Datei zusammenfügt, die auf dem Computer ausgeführt werden kann.

Es gibt mehrere Gründe, warum es sinnvoll sein kann, das Compilieren und Linken zu trennen:

- Modularität: Indem man den Quellcode in mehrere separate Dateien aufteilt und diese einzeln compiliert, kann man die Entwicklung und Wartung des Programms vereinfachen.

- Wiederverwendbarkeit: Indem man einzelne Funktionalitäten in separate Objektdateien auslagert, können diese leichter in andere Projekte eingebunden werden.

- Optimierung: Durch die Trennung von Compilieren und Linken kann man die Optimierung des Codes besser steuern und gezielt bestimmte Teile des Codes optimieren.

- Fehlerbehebung: Wenn Fehler im Code auftreten, ist es einfacher, diese zu finden und zu beheben, wenn man den Quellcode in separate Dateien aufgeteilt hat und diese einzeln compiliert und verlinkt.

- Systemunabhängigkeit: Indem man den Quellcode in separate Dateien aufteilt und diese einzeln compiliert, kann man das Programm leichter auf verschiedenen Systemen und Plattformen ausführen.


<br><br><br>


# 2. Frage:

Wozu benötigt man Libraries?

---

Libraries in C werden verwendet, um wiederholt verwendete Code-Funktionalitäten zusammenzufassen und zu organisieren. Sie ermöglichen es Entwicklern, auf bereits existierende Funktionen und Ressourcen zuzugreifen, anstatt sie jedes Mal neu zu schreiben, wenn sie benötigt werden. Dadurch kann die Entwicklungszeit verkürzt und die Code-Wartung vereinfacht werden. Einige Beispiele für C-Libraries sind die Standard C Library und die POSIX-Library.


<br><br><br>


# 3. Frage:

Was ist der Unterschied zwischen Werteparametern (auch Argumente genannt) und Variablenparametern (auch Parameter genannt)?

---

Der Unterschied zwischen Werteparametern und Variablenparametern besteht darin, wie die Argumente an eine Funktion übergeben werden und wie sie innerhalb der Funktion behandelt werden.

Werteparameter sind Argumente, die an eine Funktion als Kopie des tatsächlichen Werts übergeben werden. Wenn sich der Wert innerhalb der Funktion ändert, wird dieser nur innerhalb der Funktion geändert und hat keinen Einfluss auf den ursprünglichen Wert außerhalb der Funktion.

Variablenparameter hingegen sind Argumente, die als Verweis auf die tatsächliche Variable übergeben werden. Wenn sich der Wert innerhalb der Funktion ändert, wird dieser auch außerhalb der Funktion geändert, da die Funktion auf die tatsächliche Variable zugreift und nicht auf eine Kopie.

In C werden die Werteparameter durch normale Argumente und die Variablenparameter durch Pointer realisiert.


<br><br><br>


# 4. Frage:

Wie erfolgt die Umsetzung von Werte- und Variablenparametern in C Geben Sie jeweils ein Beispiel an!

---

In C werden Werteparameter als normale Argumente übergeben und Variablenparameter als Pointer.

Ein Beispiel für eine Funktion mit Werteparametern:

```c
void add(int a, int b) {
    int result = a + b;
    printf("%d + %d = %d\n", a, b, result);
}
```

In diesem Beispiel werden die Werte von `a` und `b` als Werteparameter an die Funktion add übergeben. Innerhalb der Funktion wird eine neue Variable result erstellt, die die Summe von `a` und `b` enthält. Änderungen an `a` und `b` innerhalb der Funktion haben keinen Einfluss auf die ursprünglichen Variablen außerhalb der Funktion.

Ein Beispiel für eine Funktion mit Variablenparametern:

```c
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
```

In diesem Beispiel werden die Adressen von `a` und `b` als Variablenparameter an die Funktion swap übergeben, die als Pointer deklariert sind. Innerhalb der Funktion werden die Werte von `a` und `b` mithilfe der Pointer ausgetauscht. Da die Funktion auf die tatsächlichen Variablen zugreift und nicht auf Kopien, werden die ursprünglichen Variablen außerhalb der Funktion ebenfalls geändert.


<br><br><br>


# 5. Frage:

Erklären Sie, welche Informationen am Stack bei Werte- und Variablenparametern abgelegt werden.

---

Bei Werteparametern werden die tatsächlichen Werte in den Stack geschrieben, während bei Variablenparametern die Adressen der Variablen gespeichert werden. Da Variablenparameter nur als Zeiger übergeben werden, werden deren Werte nicht im Stack abgelegt. Stattdessen wird der Adressspeicherort in den Stack geschrieben, sodass auf den Wert der Variablen zugegriffen werden kann.


<br><br><br>


# 6. Frage:

Wozu wird das Schlüsselwort `static` verwendet? Wann sollten Sie es nicht verwenden?

---

In C hat das Wort `static` verschiedene Bedeutungen, je nach Kontext in dem es verwendet wird.

Es kann Variablen, Funktionen, Blöcke oder Dateien betreffen.

- Variablen: `static` verhindert, dass die Variable während der Programmausführung zerstört wird, sodass sie über mehrere Funktionsaufrufe hinweg gespeichert werden kann.
- Funktionen: `static` verhindert, dass eine Funktion aus einer anderen Datei aufgerufen werden kann.
- Blöcke: `static` verhindert, dass Variablen innerhalb eines Blocks zerstört werden.
- Dateien (global): `static` verhindert, dass die Variablen und Funktionen der Datei in anderen Dateien verwendet werden können. Beispiel: Wenn man eine Variable in einer Header-Datei definiert, kann man das Schlüsselwort `static` verwenden, um sicherzustellen, dass die Variable nur innerhalb der Header-Datei verfügbar ist und nicht in anderen Dateien verwendet werden kann.

Wenn man in einem Multithreading-Programm das Schlüsselwort `static` verwendet, kann es zu unerwarteten Problemen führen, da `static` Variablen und Funktionen zwischen den verschiedenen Threads teilt. Daher sollte man immer vermeiden, `static` in einem Multithreading-Programm zu verwenden.


<br><br><br>


# 7. Frage:

Welche Informationen werden in Headerdateien abgelegt und was gehört in die Implementierungsdateien?

---

Header-Dateien enthalten Informationen wie Funktionsdeklarationen, Strukturdefinitionen, Makros und Konstanten. In Implementierungsdateien werden die Funktionen, die in den Header-Dateien deklariert wurden, definiert. Sie enthalten auch die Definitionen von Variablen und Strukturen, die in den Header-Dateien deklariert wurden. Außerdem können Implementierungsdateien auch Variablen, Funktionen und Strukturen enthalten, die nur für die Implementierungsdatei selbst bestimmt sind.


<br><br><br>


# 8. Frage:

Wie können Sie mit dem Präprozessor das mehrfache Einbinden von Headerdateien verhindern? Erklären Sie dies anhand eines Beispiels!

---

Der C-Präprozessor kann verhindern, dass Headerdateien mehrfach eingebunden werden, indem er das Schlüsselwort `#pragma once` verwendet, das dazu verwendet wird, ein einziges Einbinden einer bestimmten Headerdatei zu erzwingen.

Beispiel:

Angenommen, wir haben eine Headerdatei mit dem Namen `myheader.h`, die das Schlüsselwort `#pragma once` enthält. Wenn die Datei in mehreren Dateien eingebunden wird, wird der Präprozessor nur die erste Instanz der Datei verarbeiten, die anderen Instanzen werden ignoriert.

Die Alternative zu `#pragma once` sind sogenannte "Include-Guards". Diese bestehen aus einer Kombination aus Makros, die in der Headerdatei definiert werden, und den Präprozessor-Direktiven `#ifndef`, `#define` und `#endif`. Diese Makros werden verwendet, um zu verhindern, dass eine Headerdatei mehrfach eingebunden wird. Anstatt `#pragma once` wird das Makro in der Headerdatei definiert, z.B. `#ifndef MY_HEADER_H`, gefolgt von der Definition des Makros `#define MY_HEADER_H` und der direkten `#endif`. So kann der Präprozessor die Headerdatei nur einmal einbinden und alle anderen Instanzen werden ignoriert.


<br><br><br>


# 9. Frage:

Nennen Sie 3 Einsatzzwecke von Funktionspointern!

---

Funktionspointer können für viele verschiedene Zwecke verwendet werden, darunter:

- Sie können verwendet werden, um Callback-Funktionen zu erstellen, die aufgerufen werden, wenn bestimmte Ereignisse auftreten.
- Sie können verwendet werden, um Funktionen dynamisch aufzurufen.
- Sie können verwendet werden, um eine Funktion als Parameter an eine andere Funktion zu übergeben.


<br><br><br>


# 10. Frage:

Erstellen Sie Beispielcode für Funktionspointer. Defnieren Sie zuerst einen Funktionspointer names `pFunc` auf eine Funktion, welche einen Rückgabewert vom Typ `int` zurückliefert und einen `char` als Argument erwartet.
Lassen Sie `pFunc` auf die Funktion `strlen` aus der Standardlibrary zeigen und rufen Sie Funktion anschließend über `pFunc` auf um die Länge des Strings `teststring` zu ermitteln.

---

Beispielcode:

```c
char* testString = "Hello, World!";

int (*pFunc)(char);
pFunc = strlen;
int len = pFunc(testString);
```

<br><br><br>


# 11. Frage:

Erstellen Sie das `Makefile` für ein Programm `mytool`, welches aus 2 Modulen besteht mit Sourcedateien `module1.c` und `module2.c` sowie zugehörigen Headerdateien `module1.h` und `module2.h`.
Gehen Sie davon aus, dass Funktionalität von Modul 1 in Modul 2 verwendet wird und umgekehrt!
Vergessen Sie nicht auf das Target `clean`!

---

```Makefile

.PHONY: all clean

mytool: module1.o module2.o
	gcc -o mytool module1.o module2.o

module1.o: module1.c module1.h
	gcc -c module1.c

module2.o: module2.c module2.h
	gcc -c module2.c

all: mytool

clean:
	rm *.o mytool

```
