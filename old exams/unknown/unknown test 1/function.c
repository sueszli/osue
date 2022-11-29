#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

int main(int argc, char *argv[])
{

    char *iban = "GB29NWBK60161331926819\0";
    char reversed[strlen(iban)];
    char firstLetters[5];
    strncpy(firstLetters, iban, 4);
    char *rest = &iban[4];
    strcpy(reversed, rest);
    strcat(reversed, firstLetters);
    char expr[80];
    int pos = 0;

    for (int i = 0; i < strlen(iban); i++)
    {

        int number = (int)reversed[i];
        if (isupper(number))
        {
            number = number - 55;
            sprintf(&expr[pos], "%d", number);
            pos += 2;
        }
        else
        {
            expr[pos] = number;
            pos += 1;
        }
    }

    printf("%s", expr);

    exit(EXIT_SUCCESS);
}