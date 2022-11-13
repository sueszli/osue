/**
 * @file nospaces.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 28.10.2021
 *
 * @brief Modul welches Leerzeichen aus einem übergebenen String löscht
 *
 * In diesem Modul wird die Funktion zum löschen von Leerzeichen aus einem übergebenen String gelöscht und der daraus folgende
 String wird zurückgegeben.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "header.h"

char* nospaces(char string[]){
    /**
     *@brief Länge von string
     */
    int stringlength = strlen(string);
    /**
     *@brief String in dem string kopiert wird zum bearbeiten
     */
    char mystring[stringlength];

    strcpy(mystring, string);

    /**
     *@brief Variable zum speichern der nötigen Größe von outstring
     */
    int m = 0; //Größe von outstring bestimmen
    
    /**
     *@brief Schleifenvariablen. i für allgemeines Fortschreiten, j für Fortschreiten wenn nicht Leerzeichen gelesen wird.
     */
    int i, j; //Loop Variablen, weil aus irgendeinem Grund trotz -std=c99 Option der Compiler schreit, wenn man in "for" initialisiert

    for(i = 0; i < stringlength; i++){
       char reader = mystring[i];
        if(reader != ' '){ //Sollte Leerzeichen nicht zählen //'\n' muss noch behandelt werden?
            if(reader != '\0'){ //Zusätzlicher Check damit man nicht das Ende des Strings zählt
            m++;
            }
        } 
    }

    /**
     *@brief String in dem der zurückzugebende String gespeichert wird.
     */
    char* outstring = (char *)malloc(m*sizeof(char));

    for(i = 0, j = 0; i < stringlength; i++){
        char reader = mystring[i];
        if(reader != ' '){ //Sollte Leerzeichen nicht kopieren //'\n' muss noch behandelt werden?
            if(reader != '\0'){ //Zusätzlicher Check damit man nicht das Ende des Strings kopiert
                outstring[j] = mystring[i];
                j++; //j wird nur erhöht, wenn kein Leerzeichen auftaucht (damit man keine leeren Zeichen setzt)
            }
        }
    }
    return outstring;
}