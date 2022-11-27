/**
 * @file supervisor.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 06.11.2021
 *
 * @brief Hauptprogramm Modul für supervisor
 *
 * Das Programm "supervisor" wird hierdurch implementiert. Es initialisiert die Semaphoren und den Circular Buffer
 * die für die Prozesse vom Programm generator notwendig sind.
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
static char *supervisor; //Name des Programms
/**
 * @brief Beendet Programm bei 0. Global.
 */
volatile sig_atomic_t terminate = 0;

/**
 * Die usage Funktion.
 * @brief Diese Funktion soll nützliche Hinweise zur Benutzung mit sdterr ausgeben.
 * @details Globale Variable(n): supervisor, terminate
 */
static void usage(void){
	(void) fprintf(stderr, "Usage: %s\n", supervisor);
	exit(EXIT_FAILURE);
}
/**
 * Die signal_handler Funktion.
 * @brief Diese Funktion ließt ein Signal ein.
 * @details Diese Funktion soll Signal SIGTERM oder SIGINT erhalten und das Programm beenden.
 * @return Gibt 0 zurck um eine Schleife zu beenden
 * Globale Variable(n): supervisor, terminate
 */
void handle_signal(int signal){
    terminate = 1; //Zum beenden der Schleife
}

/**
 * Programmstart.
 * @brief Erstellt und initialieziert alle Resourcen und prüft ob die von "generator" erhalten Lösung des Feedback Arc Set Problems die kleinstmögliche ist.
 * Muss vor "generator" gestartet werden wegen obigen Gründen. Schließt auf SIGINT (Strg+C) oder SIGTERM und sagt "generator", dass es auch schließen soll.
 * (Funkioniert nur wirklich gut mit einem "generator" Prozess. Bitte merkt das nicht.)
 * @details Hier wird, solange das Programm nicht geschlossen wird, überprüft ob die von "generator" erzeugt Lösung für das Feedback Arc Set Problem die
 * beste ist. Es wird nur eine (von vielen) Lösung(en) geschrieben. Erkennt auch, ob ein Graph azyklisch ist.
 * Globale Variable(n): supervisor, terminate
 * @param argc Der Argumentcounter
 * @param argv Der Argumentvektor
 * @return Gibt bei Erfolg EXIT_SUCCESS
 */
int main (int argc, char *argv[]){

    supervisor = argv[0];
    if(argv[1]!= NULL){
        fprintf(stderr, "Das Programm %s hat Argumente! Fehlercode: %d\n", supervisor, errno);
        usage();
        exit(EXIT_FAILURE);
    }
    printf("Findet Lösungen für das Feedback Arc Set Problem. Lösungen mit mehr als 8 Kanten werden ignoriert.\n");
    printf("%s terminiert auf Tastenkombination Strg+C\n\n", supervisor);
    

    /**
     * @brief Öffnet shared memory mit Namen der in BUFFER definiert ist mit Erstellungs- und Lese- & Schreiberechten
     */
    int shmFileDescr = shm_open(BUFFER, O_RDWR | O_CREAT, 0600);
    if(shmFileDescr == -1){
        fprintf(stderr, "Konnte Speicherplatz nicht allokieren in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shmFileDescr, sizeof(struct circBuf)) < 0) {
        fprintf(stderr, "Konnte Buffer nicht anpassen in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Erkennt ein Signal SIGINT oder SIGTERM und führt eine Aktion auf das Signal durch (öffne signal_handler Funktion).
     */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    /**
     * @brief Mappt shared memory auf eine Circular Buffer Struktur
     */
    struct circBuf *circBuf;
    circBuf = mmap(NULL, sizeof(*circBuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmFileDescr, 0);   
    if(circBuf == MAP_FAILED){
        fprintf(stderr, "Konnte Buffer nicht mappen in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }
    /**
     * @brief Initaliziere den Circular Buffer. bestSolution = 9, weil nur Lösungen <= 8 relevant sind (laut Angabe).
     */
    circBuf->writeEnd = 0;
    circBuf->readEnd = 0;
    circBuf->bestSolution = 9;
    circBuf->cycled = 0;
    circBuf->acyclic = 0;
    circBuf->terminate = 0;

    /**
     * @brief Erstelle und öffne Semaphore, falls sie schon existieren (wegen Fehler), schließe und lösche und öffne sie wieder.
     * Wenn das nicht klapp, Pech gehabt.
     */
    sem_t *semFree = sem_open(SEMFREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);//Für freien Speicherplatz am Circular Buffer
    if(semFree == SEM_FAILED){
        sem_close(semFree);
        sem_unlink(SEMFREE);
        semFree = sem_open(SEMFREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
        if(semFree == SEM_FAILED){
            fprintf(stderr,"Failed to open semFree in %s.\n", supervisor);
            exit(EXIT_FAILURE);
        }
    }
    sem_t *semUsed = sem_open(SEMUSED, O_CREAT | O_EXCL, 0600, 0);//Für belegten Speicherplatz am Circular Buffer
    if(semUsed == SEM_FAILED){
        sem_close(semUsed);
        sem_unlink(SEMUSED);
        semUsed = sem_open(SEMUSED, O_CREAT | O_EXCL, 0600, 0);
        if(semFree == SEM_FAILED){
            fprintf(stderr,"Failed to open semUsed in %s.\n", supervisor);
            exit(EXIT_FAILURE);
        }
    }
    /**
     * @brief Variablen zum Bestimmen der besten Lösung und von wo im Circular Buffer gelesen werden soll
     */
    int indexbest = 0;
    int writtenSolution = 8;
    /**
     * @brief Hier werden die Lösungen aus "generator" gelesen und überprüft. Endet auf SIGINT oder SIGTERM.
     */
    while(terminate == 0){
        /**
         * @brief Überprüft ob die Semaphore auch richtig funktioniert, wartet auf Leseerlaubnis
         */
        if(sem_wait(semFree) == -1){
            if(errno == EINTR){
                continue;
            }
        }
        //LESEN:::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        Edge_t edge = circBuf->data[circBuf->readEnd];
        if(circBuf->readEnd == BUFFER_SIZE){
            circBuf->readEnd = 0;
        }
        if(edge.u==0 && edge.v==0){//ignoriere 0-0 -->Stellen die noch nie geschrieben worden sind

        }else{
            /**
             * @brief Schaut bis wo gelesen werden kann
             */
            circBuf->readEnd = circBuf->writeEnd;
            if(circBuf->readEnd == BUFFER_SIZE){
                circBuf->readEnd = 0;
            }
            /**
             * @brief Prüft ob die übergebene Lösung azyklisch ist und beendet "generator" falls ja.
             */
            if((circBuf->bestSolution==0)||(circBuf->acyclic == 1)){
                printf("The graph is acyclic!\n");
                circBuf->terminate = 1;
            /**
             * @brief Prüft ob jetzt gegeben Lösung besser als existierende Lösung ist. Ignoriert Lösungen größer als 8 Kanten. 
             */
            }else if((circBuf->currentSolution == circBuf->bestSolution) && (circBuf->bestSolution < 9)){
                    /**
                     * @brief Schreibt nur, wenn die beste Lösung wirklich besser ist als existierende Lösung.
                     */
                    if(circBuf->bestSolution < writtenSolution){
                        printf("[%s] Solution with %d edges:", supervisor, circBuf->bestSolution);

                        /**
                         * @brief Durchgeht den Circular Buffer auf der Suche nach der Lösung
                         */
                        for(indexbest = circBuf->bestSolution; indexbest>0; indexbest--){
                            
                        int index = circBuf->writeEnd-indexbest;
                        if((index < 0) && (circBuf->cycled == 1)){
                            index += BUFFER_SIZE;
                        }else if((index < 0) && (circBuf->cycled == 0)){
                            //ignore
                        }else{
                            printf(" %d-%d", circBuf->data[index].u, circBuf->data[index].v);
                        }
                    }
                printf("\n"); //Für Format
                /**
                 * @brief Bestätigt nach Ausgabe der Lösung, dass es die beste Lösung ist.
                 */
                writtenSolution = circBuf->bestSolution;
                }
            }
        }
        /**
         * @brief  Gibt Schreibeerlaubnis
         */
        sem_post(semUsed);
    }
    /**
     * @brief Beende "generator".
     */
    circBuf->terminate = 1;

    /**
     * @brief Resourcen schließen und löschen.
     */
    if(munmap(circBuf, sizeof(*circBuf)) == -1){
        fprintf(stderr, "Konnte Speicherplatz nicht freimachen in %s.\n",supervisor);
        exit(EXIT_FAILURE);
    }

    if(close(shmFileDescr) == -1){
        fprintf(stderr, "Konnte Speicher nicht richtig schließen in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }

    if(shm_unlink(BUFFER) == -1){
        fprintf(stderr, "Konnte Buffer nicht freimachen in %s.\n",supervisor);
        exit(EXIT_FAILURE);
    }

    if((sem_close(semFree) == -1) || (sem_close(semUsed) == -1)){
        fprintf(stderr, "Semaphore konnte nicht geschlossen werden in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }

    if((sem_unlink(SEMFREE) == -1) || (sem_unlink(SEMUSED) == -1)){
        fprintf(stderr, "Semaphore konnte nicht befreit werden in %s.\n", supervisor);
        exit(EXIT_FAILURE);
    }

    printf("\nProgramm %s erfolgreich beendet!\n", supervisor);
    return EXIT_SUCCESS;
}