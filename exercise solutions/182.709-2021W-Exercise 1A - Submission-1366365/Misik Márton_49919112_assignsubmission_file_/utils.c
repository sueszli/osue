#include "utils.h"

void appendToFullResult(char **fullResult, char *whatToAppend) {
    char *tmp = *fullResult;
    *fullResult = (char *) malloc(sizeof(char) * (strlen(tmp) + strlen(whatToAppend) + 1));
    if(*fullResult == NULL){
        fprintf(stderr, "malloc failed in appendToFullResult\n");
        exit(EXIT_FAILURE);
    }

    if ((strcpy(*fullResult, tmp) == NULL) || (strcat(*fullResult,whatToAppend) == NULL)) {
        fprintf(stderr, "strcpy failed in appendToFullResult\n");
        exit(EXIT_FAILURE);
    }

    free(tmp);
}

void fileManager(char **finalOutput, char *fileNames[], int fileNamesStart, int fileNamesCount, bool sflag, bool iflag)  {
    for (int i = fileNamesStart; i < (fileNamesStart + fileNamesCount); i++) {
        //TODO fix ganyolas
        FILE *fp = fopen(fileNames[i], "r");
        char buffer[BUFF_SIZE];
        while((fgets(buffer, BUFF_SIZE, fp)) != NULL){
            char *res = evaluate(buffer, sflag, iflag);
            //appendToFullResult(&fullresult_gl, res);
            appendToFullResult(finalOutput, res);
        }
        fclose(fp);
    }
}

/*void tester(char *char1){
    char *temp = char1;
    char *extra = " a macska";
    printf("%s", temp);
    char1 = malloc(sizeof(char) * (strlen(char1) + strlen(extra)));
    if((strcpy(char1, temp) == NULL) || ((strcpy(char1 + (strlen(temp)), extra)) == NULL)){
        fprintf(stderr, "fml");
        exit(EXIT_FAILURE);
    }
}*/

char *evaluate(char *source, bool sflag, bool iflag){
    int originalLength = 0;
    if(source[strlen(source) - 1] == '\n'){
        originalLength = strlen(source) - 1;
    }
    else {
        originalLength = strlen(source);
    }
    char reversed[originalLength];
    for(int i = 0; i < originalLength; i++){
        reversed[i] = source[originalLength - i - 1];
    }
    bool isPalindrom = true;
    if(sflag){
        int j = 0;
        for(int i = 0; (i < originalLength) && isPalindrom; i++){
            if(source[i] == ' '){
                continue;
            }
            else if(reversed[j] == ' '){
                i--;
            }
            else
            {
                if(iflag){
                    if(tolower(source[i]) != tolower(reversed[j])){
                        isPalindrom = false;
                    }
                }
                else if(source[i] != reversed[j]){
                    isPalindrom = false;
                }
            }
            j++;
        }
    }
    else if(iflag){
        for(int i = 0; (i < originalLength) && isPalindrom; i++){
            if(tolower(source[i]) != tolower(reversed[i])){
                isPalindrom = false;
            }
        }
    }
    else {
        for(int i = 0; (i < originalLength) && isPalindrom; i++){
            if(source[i] != reversed[i]){
                isPalindrom = false;
            }
        }
    }

    char *verdict;
    if(isPalindrom){
        verdict = " is a palindrom\n";
    }
    else {
        verdict = " is not a palindrom\n";
    }

    char *answer = malloc(sizeof(char) * (strlen(verdict) + strlen(source) + 1));
    if(answer == NULL){
                fprintf(stderr, "malloc failed in evaluate\n");
                exit(EXIT_FAILURE);
            }
    if((strncpy(answer, source, originalLength) == NULL) || (strcpy(answer + originalLength, verdict) == NULL)){
        fprintf(stderr, "strcpy failed in evaluate\n");
        exit(EXIT_FAILURE);
    }
    return answer;
}