/**
 * @file tolowercase.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 28.10.2021
 *
 * @brief Modul welches einen übergebenen String zu Kleinbuchstaben ändert
 *
 * In diesem Modul wird ein String den man bekommt in Kleinbuchstaben umgewandelt und als String zurückgegeben.
 **/
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

char* tolowercase(char string[]){
    /**
     * @brief Länge von string
     */
    int stringlength = strlen(string);
    /**
     *@brief String in dem der zurückzugebene String gespeichert werden soll
     */
    char* outstring = malloc(stringlength*sizeof(char));

    strcpy(outstring, string);

    /**
     *@brief Schleifenvariable zum durchgehen von outstring
     */
    int i;

    for(i = 0; i < stringlength; i++){
        outstring[i] = (char)tolower(outstring[i]); //Benutzt die Funktion tolower() um den übergeben char klein zu machen
    }
    return outstring;
}