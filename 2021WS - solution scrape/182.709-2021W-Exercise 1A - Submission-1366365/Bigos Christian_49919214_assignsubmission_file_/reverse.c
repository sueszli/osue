/**
 * @file reverse.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 02.11.2021
 *
 * @brief Modul welches einen übergebenen String invertiert
 *
 * In diesem Modul wird ein String den man bekommt invertiert und zurckgegeben.
 **/
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

char* reverse(char string[]){
    /**
     *@brief Länge von string
     */
    int stringlength = strlen(string);

    /**
     *@brief String in dem string kopiert wird zum bearbeiten
     */
    char mystring[stringlength];

    /**
     *@brief String in dem der zurückzugebenede String gespeichert wird
     */
    char* outstring = (char *)malloc(stringlength*sizeof(char));
    strcpy(mystring, string);

    /**
     *@brief Schleifenvariablen. i zum durchgehen von vorne, j zum durchgehen von hinten.
     */
    int i, j;

    for(i = 0, j = stringlength-1; i < stringlength; i++, j--){
        outstring[i] = mystring[j]; //Kopiert vom letzen umbearbeiteten zum ersten unbearbeiteten Platz
    }
    return outstring;
}