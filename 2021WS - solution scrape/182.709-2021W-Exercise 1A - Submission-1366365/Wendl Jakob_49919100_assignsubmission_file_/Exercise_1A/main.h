
/**
	@author
		Jakob Wendl, 12026658
	@date
		26.10.2021
	@brief
		Name: main.c
		Header file for the implementation of mycompress
	@detail
		Multiple input files are read and a compressed version of all the files is printed to a specific output file.
		The input files are read one after the other. Each input file is compressed by a specific rule.
		The input is compressed by substituting subsequent identical characters by only one occurence of the
		character followed by the number of characters. For example, the input aaa is compressed to a3.
		The compressed versions of the input files are then printed to one specific output file one after the other.
*/

#ifndef MAIN_H

#define MAIN_H

/**
	@brief
		Multiple input files are read and a compressed version of all the files is printed to a specific output file.
	@details
		The input files are read one after the other. Each input file is compressed by a specific rule.
		The input is compressed by substituting subsequent identical characters by only one occurence of the
		character followed by the number of characters. For example, the input aaa is compressed to a3.
		The compressed versions of the input files are then printed to one specific output file one after the other.
	@param
		int argc: The number of arguments passed to ./main
		char **argv: The arguments passed to ./main including optional arguments and positional arguments
	@return
		status
*/
int main(int argc, char **argv);

#endif
