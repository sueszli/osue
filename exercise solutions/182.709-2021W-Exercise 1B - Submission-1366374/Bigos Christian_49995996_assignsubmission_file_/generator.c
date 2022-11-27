 /**
 * @file generator.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 06.11.2021
 *
 * @brief Hauptprogramm Modul für generator
 *
 * Das Programm "generator" ....
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include "header.h"

 /**
 * @brief Der Name des Programms wird hier gespeichert. Global.
 */
static char *generator; //Name des Programms

/**
 * Die usage Funktion.
 * @brief Diese Funktion soll nützliche Hinweise zur Benutzung mit sdterr ausgeben.
 * @details Globale Variablen: ispalindrom
 */
static void usage(void){
	(void) fprintf(stderr, "Usage: %s EDGE1...\n", generator); //Falsche Angabe von Argumenten?
    (void) fprintf(stderr, "EDGE defined as: startNode-endNode\n");
    (void) fprintf(stderr, "startNode and endNode must be positive Integer values.\n");
    (void) fprintf(stderr, "Nodes musn't skip Integers and must start at 0, otherwise %s will not work properly!\n");
	exit(EXIT_FAILURE);
}


/**
 * Programmstart.
 * @brief Das Programm wird hier ausgeführt. Es soll auf einen in den Argumenten angegebenen Graphen das Feedback Arc Set Problem lösen.
 * Das Programm wird ohne dem Programm "supervisor" nicht laufen. "supervisor" muss als erstes eingeschalten werden.
 * @details Liest die in das Programm übergebenen Argumente der Reihe nach ein. Die Argumente sind direktionale Kanten eines Graphen.
 * Die Knoten müssen natürliche Zahlen sein und dürfen keine einzige Zahl überspringen. Die Kanten werden in ein struct Edge_t (definiert in header.h) gespeichert.
 * Auf diese Kanten wird der Monte Carlo Algorithmus, der in den Angabefolien beschrieben ist, angewendet.
 * Nach korrektem Ausführen des Algorithmus werden die Ergebnisse in einen Circular Buffer geschrieben, der in "supervisor" initializiert und in
 * header.h definiert wurde. Dieser Buffer ist ein Shared Memory Objekt, also sind die Ergebnisse in beiden Programmen vorhanden (es kann aber wegen dem
 * Zeitbedarf des schreibens und lesens aus dem Speicher etwas dauern). "generator" terminiert auf ein SIGINT (Strg+C) Signal in "supervisor" oder
 * wenn der übergebene Graph azyklisch ist.
 * Globale Variable(n): generator
 * @param argc Der Argumentcounter
 * @param argv Der Argumentvektor
 * @return Gibt bei Erfolg EXIT_SUCCESS
 */
int main (int argc, char *argv[]){
/**
 * @brief Programmname
 */
    generator = argv[0];

    /**
     * @brief Öffnet shared memory mit Namen der in BUFFER definiert ist mit Lese- & Schreiberechten
     */
    int shmFileDescr = shm_open(BUFFER, O_RDWR, 0600);
    if(shmFileDescr == -1){
        fprintf(stderr, "Konnte Speicherplatz nicht allokieren in %s.\n", generator);
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Mappt geöffnete shared memory auf eine Variable vom Typ Circular Buffer Struktur
     */
    struct circBuf *circBuf;
    circBuf = mmap(NULL, sizeof(struct circBuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmFileDescr, 0);
    if(circBuf == MAP_FAILED){
        fprintf(stderr, "Konnte Buffer nicht mappen in %s.\n", generator);
        exit(EXIT_FAILURE);
    }


    /**
     * @brief Öffne die in "supervisor" initalizierten Semaphoren
     * Ignoriert ob sie da sind
     */
    sem_t *semFree = sem_open(SEMFREE, 0);//Für freien Speicherplatz am Circular Buffer
    sem_t *semUsed = sem_open(SEMUSED, 0);//Für belegten Speicherplatz am Circular Buffer

    int option;
    while ((option = getopt(argc, argv, "")) != -1){ //Keine Optionen sollen übergeben werden
		switch(option){
			case '?': {
                fprintf(stderr, "Eine Option wurde übergeben in %s!\n", generator);
				usage(); 
				break;
				}
			default: assert(0); break;
		}
	}

    /**
     * @brief Gibt Fehler auf Graph ohne Kanten
     */
    if(argv[1] == NULL){
        fprintf(stderr, "Kein Graph übergeben in %s!\n", generator);
        usage();
    }

    /**
     * @brief Aller äußerste Schleife. Läuft endlos bis "supervisor" die Variable auf 1 stellt
     * Wendet den Monte Carlo Algorithmus auf die übergebenen Kanten immer wieder an
     */
    while(circBuf->terminate == 0){
    /**
     * @brief Initializiert die Anzahl der Kanten der jetzigen Lösung auf 8 (weil mehr sinnlos ist laut Angabe).
     */
    circBuf->currentSolution = 8;
    int argcount; /** @brief Counter für argv[]*/
    int charcount; /** @brief Counter zum durchgehen von argv[argcount]*/
    int startOrEnd; /** @brief Zum Entscheiden ob ein Anfangsknoten oder Endknoten gelesen wird (BOOL)*/

    Edge_t edges[argc]; /** @brief Hier werden die Kanten gespeichert*/

    /**
     * @brief Ließt die chars im Argument einzeln ein und speichert diese in edges[] ab, falls sie eine gültige Eingabe sind
     * @details Ließt argv[] solange durch, bis keine Argumente vorhanden sind. Die chars jedes Arguments werden einzeln eingelesen
     * um auf das richtige Format zu prüfen. Es kann mit Hilfe von startOrEnd bestimmt werden, ob der Startknoten oder Endknoten
     * gerade gelesen wird.
     */
    for(argcount = 1; (argcount < argc) &&(argv[argcount] != NULL); argcount++){ //Ließt argv[] auf Kanten aus
        startOrEnd = 0; //Startknoten lesen
        int number = 0;

        for(charcount = 0; charcount < strlen(argv[argcount]); charcount++){ //Überprüft ob gelesener char eine Ziffer von 0-9 ist oder '-'

            char c = argv[argcount][charcount];
            int digit = c - '0'; //Bestimmt die Ziffer -'0' um die Verschiebung bis char 0 loszuwerden
            
            if(isdigit(c)){ //Funktion aus <ctype.h> zum bestimmen einer Ziffer

                number += digit; //Für die Ziffern
                if(startOrEnd == 0){edges[argcount -1].u = number;} //argcount-1, weil argcount bei 1 statt 0 beginnt
                if(startOrEnd == 1){edges[argcount-1].v = number;}
                number *= 10; //Für die Stellen (Einzer,Zehner,Hunderter..)
            }else if(c == '-'){
                startOrEnd = 1; //Endknoten lesen
                number = 0; //Resette den Integer
            } else{
                fprintf(stderr, "Etwas falsches wurde übergeben in %s!\nKnoten müssen eine Nummer sein!\n", generator);
                usage();
            }
            
        }
    }


    /**
     * @brief Hier wird bestimmt, wie viele verschiedene Knoten geschrieben wurden, denn für alle zusammenhängende Graphen gilt #Kanten >= #Knoten.
     */
    int numberOfNodes = 0; //Bestimmt die Anzahl der Knotenpunkte
    int numberOfEdges = argc-1; //Anzahl der Kanten ist um eins weniger als die übergebenen Argumente wegen den Programm generator(auch Argument)
    for(argcount = 0; argcount < argc-1; argcount++){
        if(numberOfNodes < edges[argcount].u){
            numberOfNodes = edges[argcount].u;
        }
        if(numberOfNodes < edges[argcount].v){
            numberOfNodes = edges[argcount].v;
        }
    }
    numberOfNodes++; //Kompensation für Knoten 0


    /**
     * @brief Initaliziert die Arrays randomOrder, in dem Knoten in zufälliger Reihenfolge geschrieben werden sollen, und wasWritten(BOOL), welches
     * überprüft ob der Knoten an der Stelle schon einmal in randomOrder eingefügt wurde
     */
    int randomOrder[numberOfNodes]; //Hier werden die Knoten zufällig hineinsortiert
    int wasWritten[numberOfNodes];
    for(argcount = 0; argcount < numberOfNodes; argcount++){ //Füllt randomOrder mit -1 und wasWritten mit 0(noch nicht belegt)
        randomOrder[argcount] = -1;
        wasWritten[argcount] = 0;
    }


    /**
     * @brief In dieser Schleife werden die Knoten in zufälliger Reihenfolge hineingeschrieben
     * @details In dieser Schleife werden die Knoten in zufälliger Reihenfolge hineingeschrieben
     * man überprüft ob der Knoten schon einmal hineingeschrieben wurde mit den Hilfsarray wasWritten.
     * srand() soll mit Hilfe der Zeit aus time() einen Seed für rand() erstellen, damit es nicht mehr Pseudo zufällig ist
     * (immer das gleiche Ergebnis bei gleicher Eingabe sonst)*/
    time_t t;
    srand((unsigned) time(&t));
    for(argcount = 0; argcount < numberOfNodes; argcount++){
        int random = rand() % numberOfNodes;
        while(1){//Checkt ob eine Nummer schon geschrieben wurde in gegebenem Array{
            if(wasWritten[random] == 0){
                randomOrder[argcount] = random;
                wasWritten[random] = 1;
                break;
            }else{
                random = rand() % numberOfNodes;
            }
        }
        //printf("randomOrder[%d] ist: %d\n", argcount, randomOrder[argcount]);//////////////////////////
    }



    /**
     * @brief i,j,k sind Schleifenvariablen, current ist ein Counter der die Anzahl der in dieser Iteration als Lösung gefunden Kanten speichern soll
     */
    int i, j, k, current = 0;
    /**
     * @brief Benutze den Algorithmus aus der Angabe
     * @details Schau in randomOrder[] und überprüfe, ob der Endknoten einer Kante in randomOrder[] vor dem Startknoten derselben Kante ist.
     * Wenn ja, dann ist das eine Kante die in die Lösung soll.
     */
    for(i = 0; i < numberOfNodes; i++){//Durchgehe die zufällig sortierte Knoten
        int currentNode = randomOrder[i];

        for(j = 0; j < numberOfEdges; j++){//Suche nach allen Kanten mit derzeitigem Startknoten
            int startNode = edges[j].u;
            int endNode = edges[j].v;

            if(startNode == currentNode){

                for(k = 0; randomOrder[k] != endNode; k++){ //finde endKnoten in randomOrder
                }
                if(i > k){ //Stellenwert vom Startknoten in randomOrder> Stellenwert vom Endknoten in randomOrder (Mentale Notiz)
                    int semValueFree=-2;
                    int semValueUsed=-2;
                    if((sem_getvalue(semFree, &semValueFree) == -1) ||(sem_getvalue(semUsed, &semValueUsed) == -1)){
                        fprintf(stderr, "Konnte den Wert der Semaphore nicht bestimmen in %s.\n", generator);
                        exit(EXIT_FAILURE);
                    }
                    /**
                     * @brief Warten auf Schreiberlaubnis
                     */
                    sem_wait(semUsed);
                    //SCHREIBEN::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                    /**
                     * @brief Hier wird das Ergebnis in den Buffer geschrieben.
                     */
                    Edge_t edge;
                    edge.u = startNode;
                    edge.v = endNode;
                    circBuf->data[circBuf->writeEnd] = edge;
                    circBuf->writeEnd++;
                    /**
                     * @brief Statt Modulo, setzt cycled aus circBuf aus "supervisor" auf 1 um anzuzeigen, dass ein Zyklus durchgangen wurde.
                     * Die Essenz eines Circular Buffer.
                     */
                    if(circBuf->writeEnd == BUFFER_SIZE){ 
                        circBuf->writeEnd = 0;
                        circBuf->cycled = 1;
                    }
                    
                    current++;
                    /**
                     * @brief Gibt Leseerlaubnis
                     */
                    sem_post(semFree);
                }
            }
        }
    }
    /**
     * @brief Setzt die Größe der jetzigen Lösung und vergleicht mit Größe bester Lösung. Setzt acyclic auf 1 falls Lösung Länge 0 hat-->azyliksch.
     */
    sem_wait(semUsed);
    if(current == 0){
        circBuf->acyclic = 1;
    }
    if(circBuf->currentSolution >= current){
        circBuf->currentSolution = current;
        if(circBuf->bestSolution > circBuf->currentSolution){
            circBuf->bestSolution = circBuf->currentSolution;
        }
    }
    sem_post(semFree);
    
    }///////////ENDE VON WHILE
    /**
     * @brief Resourcen befreit und geschlossen
     */
    if(munmap(circBuf, sizeof(*circBuf)) == -1){
        fprintf(stderr, "Konnte Speicherplatz nicht freimachen in %s.\n",generator);
        exit(EXIT_FAILURE);
    }

    if(close(shmFileDescr) == -1){
        fprintf(stderr, "Konnte Speicher nicht richtig schließen in %s.\n", generator);
        exit(EXIT_FAILURE);
    }

    if((sem_close(semFree) == -1) || (sem_close(semUsed) == -1)){
        fprintf(stderr, "Semaphore konnte nicht geschlossen werden in %s.\n", generator);
        exit(EXIT_FAILURE);
    }

    printf("Das Programm %s erfolgreich beendet!\n", generator);
    return EXIT_SUCCESS;
}