#include <malloc.h>
#include <stdio.h>
#include <string.h>
 
void compress(FILE *inputStream, char *buffer, int *readCharCounter, int *writtenCharCounter) {

    int outputIndex = 0;
    int consecutiveChars = 1;
    //fgetc returns int because of EOF
    int lastChar = fgetc(inputStream);
    int currentChar = 0;
    //Loop runs until EOF is read, then one last time
    while (lastChar != EOF) {
        currentChar = fgetc(inputStream);
	(*readCharCounter)++;
	if (currentChar == lastChar) {
            consecutiveChars++;
        } else {
            buffer[outputIndex] = (char) lastChar;
            outputIndex++;
            //convert number of Consecutive chars from int to string
            int numberOfDigits = snprintf(NULL, 0, "%d", consecutiveChars);
            char *consecutiveCharsAsString = malloc(numberOfDigits + 1);
            snprintf(consecutiveCharsAsString, numberOfDigits + 1, "%d", consecutiveChars);
            // write to buffer
            memcpy(&buffer[outputIndex], consecutiveCharsAsString, numberOfDigits);
            outputIndex += numberOfDigits;
            free(consecutiveCharsAsString);

            consecutiveChars = 1;
            lastChar = currentChar;
        }
    }
   *writtenCharCounter += outputIndex;
}
