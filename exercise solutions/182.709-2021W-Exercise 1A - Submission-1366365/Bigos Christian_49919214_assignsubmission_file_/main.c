/**
 * @file main.c
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 26.10.2021
 *
 * @brief Hauptprogramm Modul
 *
 * Das Programm "ispalindom" wird hierdurch implementiert. Es überprüft ob eine Eingabe
 * oder die Inhalte einer Datei ein Palindrom ist oder nicht. Es wird einige Optionen geben um weitere Einstellungen
 * für das Prüfen zu ermöglichen. Die Ausgabe kann optional in eine Datei "outfile" geschriben werden.
 **/
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

/**
 * @brief Der Name des Programms wird hier gespeichert. Global.
 */
static char *ispalindrom; //Name des Programms

/**
 * Die usage Funktion.
 * @brief Diese Funktion soll nützliche Hinweise zur Benutzung mit sdterr ausgeben.
 * @details Globale Variablen: ispalindrom
 */
static void usage(void){
	(void) fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", ispalindrom);
	(void) fprintf(stderr, "Inputfile format: name.in\n");
	(void) fprintf(stderr, "Outputfile format: name.out\n");
	exit(EXIT_FAILURE);
}
/**
 * Programmstart.
 * @brief Das Programm beginnt hier. Diese Funktion ist die allgemeine Funktion zur Funktionalität dieses Programms.
 * @details In dieser Funktion werden die übergeben Programmoptionen auf Richtigkeit geprüft und auf mögliche Input Dateien
 * des Typs ".in". Hier wird auch auf Palindrom geprüft. Diese Funktion ist viel zu lange geworden, da ich nicht die
 * Gehirnkapazität hatte um einzelne Funktionen zu machen wo ich sie hätte machen können (es war schon 3 Jahre her seit
 * meinem letzten Programm).
 * Globale Variable(n): ispalindrom
 * @param argc Der Argumentcounter
 * @param argv Der Argumentvektor
 * @return Gibt bei Erfolg EXIT_SUCCESS
 */
int main (int argc, char *argv[]){

	printf("Dieses Programm soll auf Palindrome prüfen.\n");
	ispalindrom = argv[0];

	/**
 	 * @brief Variablen in denen die Stelle gepeichert wird, wo die zutreffende Programmoption auftritt.
	 * Initilalisiert mit 1, da argv[0] den Programmnamen enthält und erst an argv[1] die erste Option auftauchen kann und diese
	 * immer 1 dazuzählen von der Orginalstelle. Enthält höchste Stelle an der die Option vorkommt.
     */
	int option_s = 1, option_i = 1, option_o = 1;
	
	/**
     * @brief Variable zum bestimmen von getopt()
     */
	int option;

	/**
 	 * @brief String in dem der Input von stdin oder einer Datei geschrieben wird.
 	 */
	char *instring = (char *)malloc(INT_MAX * sizeof(char));

	while ((option = getopt(argc, argv, "sio:")) != -1){
		switch(option){
			case 's': option_s = optind; break; //option_s checkt ob die Option für das Ignorieren von Abständen aus oder an ist
			case 'i': option_i = optind; break; //option_i checkt ob Groß- und Kleinschreibung ignoriert werden soll
			case 'o': option_o = optind; break; //option_o checkt ob in eine Output Datei geschrieben werden soll
			case '?': {
				usage(); 
				exit(EXIT_FAILURE); 
				break;
				}
			default: assert(0); break;
		}
	}
	if((option_o != 1) && (option_o < option_s)){
		fprintf(stderr, "Ungültige Angabe der Optionen! -o muss als letztes übergeben werden in %s! Fehlercode %d\n", ispalindrom,errno);
		usage();
		exit(EXIT_FAILURE);
		}
	if((option_o != 1) && (option_o < option_i)){
		fprintf(stderr, "Ungültige Angabe der Optionen! -o muss als letztes übergeben werden in %s! Fehlercode %d\n", ispalindrom,errno);
		usage();
		exit(EXIT_FAILURE);
	}

	/**
     * @brief Variable zum durchgehen von argv[]
     */
	int argumentcounter;

	/**
 	 * @brief Schaut bis wohin geschaut werden muss um eine doppelte Optionsübergabe der selben Option zu erkennen.
 	 */
	int maxarg = option_s;
	//Das obere Limit zum prüfen von doppelten Optionseingaben
	if(maxarg < option_i){
		maxarg = option_i;
		if(maxarg < option_o){
			maxarg = option_o;
		}
	}
	if(maxarg < option_o){
		maxarg = option_o;
	}
	/**
 	 * @brief Variable zum durchgehen aller argv[] zwischen argv[argumentcounter] und argv[maxarg].
 	 */
	int argcount;
	//Geht alle Optionen durch, endet auf möglicher Outputdatei. Beginnt bei 1, da argv[0] den Programmnamen enthält
	for(argumentcounter = 1; argumentcounter < maxarg; argumentcounter++){
		//Geht alles dazwischen noch durch
		for(argcount = argumentcounter + 1; argcount < maxarg; argcount++){
			if(strcmp(argv[argumentcounter], argv[argcount]) == 0){
			fprintf(stderr, "Eine Option wurde mehrmals übergeben in %s! Fehlercode %d\n", ispalindrom,errno);
			usage();
			exit(EXIT_FAILURE);
			}
		}
	}


	//Die Output Datei wird gecheckt
	/**
 	 * @brief Variable in dem der Name der Outputdatei gespeichert wird
 	 */
	char* outfile = argv[argumentcounter-1];
	/**
 	 * @brief Variable in dem die Länge von outfile gespeichert wird zur späteren Bestimmnug der Endung ".out"
 	 */
	int outl = 0;
	if(outfile != NULL){
		outl = strlen(outfile);
	}

	if(option_o > 1){ //checkt ob Option o übergeben wurde und die Datei die Endung ".out" hat. Bricht ab wenn keine Outputdatei oder falsche Endung
		if((outfile == NULL) || (outfile[outl-4] != '.') || (outfile[outl-3] != 'o') || (outfile[outl-2] != 'u') || (outfile[outl-1] != 't')){

		fprintf(stderr, "Option o ohne Argument übergeben oder Output Datei falsch angegeben in %s! Fehlercode %d\n", ispalindrom, errno);
		usage();
		exit(EXIT_FAILURE);
		}
	}

	/* Dieser grauenhafte Block verhindert, dass infile Segmentation fault erzeugt. Es wurde vorher alles genauso wie outfile behandelt
	jedoch will das Ding einfach nicht funktionieren. Ein Beispiel: char* infile = argv[argumentcounter] hat Segmentation fault
	verursacht, obwohl es das nicht tun solltes*/
	/**
 	 * @brief Input Datei(en) werden darin gespeichert. Initialisiert als "NULL" statt NULL, weil sonst Segmentation fault passiert. (Planlos wieso.)
 	 */
	char* infile = "NULL";
	/**
 	 * @brief Länge von infile darin gespeichert zur späteren Bestimmeng der Endung ".in" für Input Datei(en)
 	 */
	int inl = 0;
	if(argv[argumentcounter] != NULL){
		infile = argv[argumentcounter];
	}

	inl = strlen(infile);
	//Wenn keine Input Datei(en) entdeckt wurden, dann benutzen wir stdin
	if((strcmp(infile, "NULL")==0) || (infile[inl-3] != '.') || (infile[inl-2] != 'i') || (infile[inl-1] != 'n')){ //überprüft Dateiendung ".in"
		printf("Da keine Inputdateien angegeben wurden oder eine fehlerhafte Eingabe getätigt wurde, bitte eine Eingabe machen.\n");
		printf("Eingaben kann man mit 'Enter' prüfen lassen.\nProgramm terminiert auf 'Strg + C' bzw. 'Ctrl + C'\n");

		/**
 	 	* @brief Lokale Schleifenvariable um instring zu lesen und einen brauchbaren String outstring zu bekommen mit dem man was anstellen kann
 	 	*/
		int i;
		fgets(instring, INT_MAX * sizeof(char), stdin);
		//Überprüft ob die maximal elaubte Länge des Inputs verletzt wurde (Bei Länge > INT_MAX wird der Speicherbedarf ziemlich groß, mit LONG_MAX sehr schlimm)
		if(instring[strlen(instring)-1] != '\n'){
			fprintf(stderr, "Input zu groß! Maximale Zeilenlänge: %d\nTerminiere %s mit Fehlercode %d.\n", INT_MAX, ispalindrom,errno);
			exit(EXIT_FAILURE);
		}

		/**
		 * @brief Bestimmt maximale Größe von instring die gelesen werden kann.
		 */
		int size = strlen(instring);
		/**
		 * @brief Hier wird der Output geschrieben werden. Unser String den wir bearbeiten, falls gewünscht.
		 */
		char* outstring = (char *)malloc(size * sizeof(char));

		for(i = 0; ((i < size) && (instring[i] != EOF) && (instring[i] != '\n')); i++){ //Werde EOF(End Of File) und nextLine los
			outstring[i] = instring[i];
		}

		if(option_s > 1){ //Benutzt temporären Speicher um nospace aufzurufen (Leerzeichen weg)
			char* tmp = nospaces(outstring);
			strcpy(outstring, tmp);
			free(tmp);
		}

		if(option_i > 1){ //Benutzt temporären Speicher um tolowercase aufzurufen (Kleinbuchstaben)
			char* tmp = tolowercase(outstring);
			strcpy (outstring, tmp);
			free(tmp);
		}
		// In diesem Block wird überprüft ob die Eingabe ein Palindrom ist
		/**
		 * @brief Variable in der die Entscheidung gespeichert wird ob outstring ein Palindrom ist oder nicht.
		 */
		int palindrom = 0;
		char* tmp = reverse(outstring);
		if(strcmp(tmp, outstring)==0){
			palindrom = 1;
		}
		free(tmp);
		if(option_o > 1){ //Schreibe in Output Datei
			char tmp[strlen(instring)];
			strcpy(tmp, instring);
			tmp[strlen(tmp)-1] = '\0'; //setzt letztes Zeichen auf leeres Zeichen statt \n oder EOF damit Ausgabe schön ist

			FILE *filepointer = fopen(outfile, "w"); //Write-only auf Output Datei
			if(filepointer == NULL){
				fprintf(stderr, "Output Datei konnte nicht geöffnet werden in %s! Fehlercode: %d\n", ispalindrom, errno);
				exit(EXIT_FAILURE);
			}
			
			fprintf(filepointer, "%s ist ", tmp);
			if(palindrom == 0){
				fprintf(filepointer, "kein Palindrom\n");
			} else{
				fprintf(filepointer, "ein Palindrom\n");
			}
			fclose(filepointer); //Datei schließen

		}else{ //Schreibe auf stdout
			char tmp[strlen(instring)];
			strcpy(tmp, instring);
			tmp[strlen(tmp)-1] = '\0'; //setzt letztes Zeichen auf leeres Zeichen statt \n oder EOF damit Ausgabe schön ist
			fprintf(stdout, "%s ist ", tmp);
			if(palindrom == 0){
				fprintf(stdout, "kein Palindrom\n");
			} else{
				fprintf(stdout, "ein Palindrom\n");
			}
		}

		free(outstring); //outstring muss befreit werden wegen malloc

	}
	else{ //Wenn Input Datei(en) entdeckt wurden, dann arbeiten wir damit weiter
		//Reinigt die Ouput Datei
		FILE *outfilepointer;
		if((outfile == NULL) || (option_o == 1) || (outfile[outl-4] != '.') || (outfile[outl-3] != 'o') || (outfile[outl-2] != 'u') || (outfile[outl-1] != 't')){
			//Kein Outfile
		} else{
			outfilepointer = fopen(outfile, "w+");
			fprintf(outfilepointer, NULL);
			fclose(outfilepointer);
		}
		//Hier werden jetzt die Input Dateien bearbeitet

		/**
		 * @brief Oberste Schleife dieses Codeblocks. Soll alle Input Dateien der Reihe nach durchgehen, falls sie existieren.
		 * Prüft auf Gleichheit mit "NULL" statt NULL, weil infile = NULL Segmentation fault verursacht.
		 */
		while(strcmp(infile, "NULL")!=0){//Solange ich den String nicht "NULL" gesetzt habe soll es weitermachen

			FILE *infilepointer = fopen(infile, "r"); //Read-only der Input Datei
			if(infilepointer == NULL){
				fprintf(stderr, "Angegebene Input Datei existiert nicht in diesem Verzeichnis!\nBeende das Programm %s mit Fehlercode %d.\n", ispalindrom, errno);
				exit(EXIT_FAILURE);
			}
			argumentcounter++; //Von weiter oben zum durchgehen von argv[]
			infile = "NULL"; //Reset, da infile schon gelesen wurde
			int inl = 0;
			if(argv[argumentcounter] != NULL){
				infile = argv[argumentcounter];
			}
			inl = strlen(infile);

			int i, j; //Schleifenvariablen

			fgets(instring, INT_MAX * sizeof(char), infilepointer);
			//Überprüft ob die maximal elaubte Länge des Inputs verletzt wurde (Bei Länge > INT_MAX wird der Speicherbedarf ziemlich groß, mit LONG_MAX sehr schlimm)
			if(instring[strlen(instring)-1] != '\n'){
				fprintf(stderr, "Input zu groß! Maximale Zeilenlänge: %d\nTerminiere %s mit Fehlercode %d.\n", INT_MAX, ispalindrom, errno);
				exit(EXIT_FAILURE);
			}

			/**
		 	* @brief Wichtiger Codeblock der mindestens einmal durchgeführt wird dank do-while. Dieser Block soll eine Zeile aus der Inputdatei lesen
			* und überprüfen ob sie ein Palindrom ist. Deshalb wichtig, dass es eine do-while Schleife ist, damit auch die letzte Zeile der Datei bearbeitet wird
			* obwohl Endbedingung dann auch erreicht wurde.
		 	*/
			do{//solange es noch etwas zu lesen gibt in der Datei
				int size = strlen(instring);
				char* outstring = (char *)malloc(size * sizeof(char)); //Hier wird der Output geschrieben werden und mit diesem String wird gearbeitet

				for(i = 0, j = 0; ((i < size) && (instring[i] != EOF)); i++){ //Werde EOF(End Of File) los

					if(instring[i] != '\n'){ //Falls noch keine Zeile angefangen wurde
						outstring[j] = instring[i];

						j++;

					} else { //Neue Zeile wurde angefangen, also verarbeite outstring jetzt
						j = 0; //Resette den Index von outstring
					
						if(option_s > 1){ //Benutzt temporären Speicher um nospace aufzurufen (Leerzeichen weg)
							char* tmp = nospaces(outstring);
							strcpy(outstring, tmp);
							free(tmp);
						}
						if(option_i > 1){ //Benutzt temporären Speicher um tolowercase aufzurufen (Kleinbuchstaben)
							char* tmp = tolowercase(outstring);
							strcpy (outstring, tmp);
							free(tmp);
						}
						// In diesem Block wird überprüft ob die Eingabe ein Palindrom ist
						int palindrom = 0;
						char* tmp = reverse(outstring);
						if(strcmp(tmp, outstring)==0){
							palindrom = 1;
						}
						free(tmp);
						if(option_o > 1){ //Schreibe in Output Datei
							char tmp[strlen(instring)];
							strcpy(tmp, instring);
							tmp[strlen(tmp)-1] = '\0'; //setzt letztes Zeichen auf leeres Zeichen statt \n oder EOF damit Ausgabe schön ist
 
							FILE *outfilepointer = fopen(outfile, "a"); //Append hängt an statt zu überschreiben
							if(outfilepointer == NULL){
								fprintf(stderr, "Output Datei konnte nicht geöffnet werden in %s! Fehlercode: %d\n", ispalindrom, errno);
								exit(EXIT_FAILURE);
							}
			
							fprintf(outfilepointer, "%s is ", tmp);
							if(palindrom == 0){
								fprintf(outfilepointer, "not a palindrom\n");
							} else{
								fprintf(outfilepointer, "a palindrom\n");
							}
							fclose(outfilepointer);

						}else{ //Schreibe auf stdout
							char tmp[strlen(instring)];
							strcpy(tmp, instring);
							tmp[strlen(tmp)-1] = '\0'; //setzt letztes Zeichen auf leeres Zeichen statt \n oder EOF damit Ausgabe schön ist
							fprintf(stdout, "%s is ", tmp);
							if(palindrom == 0){
								fprintf(stdout, "not a palindrom\n");
							} else{
								fprintf(stdout, "a palindrom\n");
							}
						}
					}
				}
				fgets(instring, INT_MAX * sizeof(char), infilepointer);
				free(outstring);
			} while(feof(infilepointer)==0 );

			if((strcmp(infile, "NULL")==0) || (infile[inl-3] != '.') || (infile[inl-2] != 'i') || (infile[inl-1] != 'n')){ //setzt Endbedingung für while Schleife (keine Dateien mit Endung .in)
				infile = "NULL";
			}
			
			
			fclose(infilepointer); //Schließt die Datei auf die infilepointer zeigt(die Input Datei)
		}
	}
	

	free(instring);
	printf("Das Programm %s hat erfolgreich terminiert!\n", ispalindrom);
	return EXIT_SUCCESS;
}