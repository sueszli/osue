#include "compressor.h"
#include "stdio.h"
#include "string.h"

extern char *PROGRAM_NAME;
/**
 * Compresses a string
 * @brief Simple reads to a file
 * @details Compresses a string like "sheeesh" to "s1h1e3s1h1"
 * @param input String to compress
 * @return Compressed string
 */
char* compress(char *input)
{

    if(input == NULL){
        fprintf(stderr, "Input is null.\n");
    }

    size_t inputSize = strlen(input);
    char symbol[2];   
    int symbol_counter = 0;
    char *result = malloc(sizeof(char) * inputSize + 1);
    int i = 0;
    char symbol_amount[5] = "";
    
    symbol[0] = input[0];
     symbol[1] = '\0';
     
    while (i <= inputSize)
    {
        if (symbol[0] == input[i])
        {
            symbol_counter++;
            i++;
        }
        else
        {
            sprintf(symbol_amount, "%d", symbol_counter); // integer to string
            strcat(result, symbol);
            strcat(result, symbol_amount);
            symbol_counter = 0;
            symbol[0] = input[i];
        }
    }         
    return result;
}