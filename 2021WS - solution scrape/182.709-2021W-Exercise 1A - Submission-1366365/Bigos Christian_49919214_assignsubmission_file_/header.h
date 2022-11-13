/**
 * @file header.h
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 28.10.2021
 *
 * @brief Header für Exercise 1a
 *
 * Der Header für das Programm "ispalindom" wird hierdurch implementiert.
 **/

#ifndef HEADER_FILE
#define HEADER_FILE
/**
 * Wandle Großbuchstaben zu Kleinbuchstaben um.
 * @brief Diese Funktion wandelt den übergebenen String string
 * von Großbuchstaben zu Kleinbuchstaben um und gibt einen Pointer
 * auf diesen neuen String zurück.
 * @details Diese Funktion allokiert Speicherplatz für die Rückgabe von
 * outstring, welcher nach Funktionsaufruf wieder befreit werden muss. Sie geht
 * schrittweise den String string durch und wandelt mit Hilfe von tolower() aus
 * der Bibliothek string.h und einem Cast auf char den Inhalt zu Kleinbuchstaben
 * um und fügt diese in outstring ein.
 * @param string Der String aus dem die Großbuchstaben zu Kleinbuchstaben
 * umgewandelt werden soll.
 * @return Gibt einen Charpointer outstring zurück.
 */
char* tolowercase(char string[]);


/**
 * Lösche Leerzeichen aus einem String.
 * @brief Diese Funktion löscht aus dem übergebenen String string
 * die Leerzeichen, also ' ', und gibt einen Pointer aus diesen neuen
 * String zurück.
 * @details Diese Funktion allokiert Speicherplatz für die Rückgabe von 
 * outstring, welcher nach Funktionsaufruf wieder befreit werden muss. Sie geht
 * schrittweise durch einen Hilfsstring mystring durch und checkt wie viele
 * Zeichen kein Leerzeichen sind, um die nötige Größe von dem String outstring
 * zu bestimmen. Danach geht sie nocheinmal durch mystring durch und kopiert
 * jetzt alle Zeichen die kein Leerzeichen ' ' sind auf die nächste freie
 * Stelle in outstring.
 * @param string Der String aus dem Leerzeichen gelöscht werden soll.
 * @return Gibt einen Charpointer outstring zurück.
 */
char* nospaces(char string[]);


/**
 * Invertiere einen String.
 * @brief Diese Funktion invertiert den übergeben String string
 * von vorne nach hinten und gibt den Pointer auf diesen neuen String zurück.
 * @details Diese Funktion allokiert Speicherplatz für die Rückgabe von
 * outstring, welcher nach Funktionsaufruf wieder befreit werden muss.
 * Sie geht den String mystring, in dem der übergebene String string kopiert
 * wurde, vin hinten nach vorne durch und setzt die einzelnen Zeichen von
 * vorne nach hinten in den String outstring ein und gibt outstring zurück.
 * @param string Der String der invertiert werden soll.
 * @return Gibt einen Charpointer outstring zurück.
 */
char* reverse(char string[]);

#endif
