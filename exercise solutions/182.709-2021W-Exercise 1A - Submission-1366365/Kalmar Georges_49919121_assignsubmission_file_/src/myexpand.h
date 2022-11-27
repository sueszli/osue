/**
 * @file myexpand.h
 * @author Georges Kalmar 9803393
 * @date 9.11.2021
 *
 * @brief Provides useful functions for the program.
 * 
 * This program contains the abort function to be used for exit failure and the replaceTab
 * function the contains the logic for replacing tabulators with spaces. 
 **/

#ifndef MYEXPAND_H
#define MYEXPAND_H

/** 
 * @brief Function that exits the program
 * @details This function is called if an error occured due to an invalid input of a client, in this case the program cannot continue the tasks properly.
 * Therefore it prints the name of the program accompanied with the valid form inputs should be given and exits the program with EXIT_FAILURE
 * @param myprog is used to give the funtion the program names Argument using argv[0]
 **/
void abortProg(char* myprog);
/** 
 * @brief Function that replaces the tabs in the input sequence with specified spaces in the output area
 * @details This function opens one or several files/or uses the stdin instead and reads the single chars. Whenever a tab is recognized it is replaced by
 * a specified amount of spaces, the tabstop distance. This distance is calculated each time assuring that the function puts the exact amount of 
 * spaces in the line in order to continue the chars at the next multiple of the tabstop distance.
 * @param filepath gives the funtion the name of the input file or NULL if stdin should be used instead
 * @param out is the pointer to the opened output file or stdout
 * @param tabstop specifies the amount of spaces that replace the tab
 * @param myprog is used to give the funtion the program names Argument using argv[0]
 **/
void replaceTab(char* filepath, FILE* out, int tabstop,char* myprog);

#endif