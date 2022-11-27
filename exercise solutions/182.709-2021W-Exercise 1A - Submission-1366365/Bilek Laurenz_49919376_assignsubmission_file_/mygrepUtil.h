/**
 * @file mygrepUtil.h
 * @author Laurenz Bilek e11904655@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief header for mygrepUtil.c
 **/
#ifndef MYGREPUTIL_H  /* Include guard */
#define MYGREPUTIL_H

//deklarationen der konstanten
//deklarationen/definitionen der structs, funktionen etc
void usage();

char *convertToLowerCase(char **str);

void writeIntoFile(char *wf, char *read);

void closeFiles(FILE *rf, FILE *wf);

void readFromFile(int opt_i, int opt_o, char *outputfile, char *file, char *keyword);

void readFromStdin(int opt_i, int opt_o, char *o_arg, char *keyword);

#endif // MYGREPUTIL_H