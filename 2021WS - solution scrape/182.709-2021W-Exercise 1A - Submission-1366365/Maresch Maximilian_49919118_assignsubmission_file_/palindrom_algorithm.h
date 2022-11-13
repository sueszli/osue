/**
 * @author Maximilian Maresch
 */ 

/**
 * @brief Checks whether or not input is palindrom
 * @detail This function takes input, its size and indicators for whether or not the i and s argument was set. Based on this
 * it returns a string describing whether input is palindrom or not (based on the given options). If opt_i > 0, then the check 
 * for a palindrom is case insensitive. If opt_s > 0, then whitespaces are ignored when checking whether a input is a palindrom.
 * 
 * @param input - the input to check for beginning a palindrom
 * @param size - size of input
 * @param opt_i - indicator for whether or not the i argument was set
 * @param opt_s - indicator for whether or not the s argument was set
 * @return the string "is not a palindrom\n" or " is a palindrom\n" depending on whether input is a palindrom
 */ 
char* ispalindrom(char* input, int size, int opt_i, int opt_s);