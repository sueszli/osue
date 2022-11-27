#include <stdio.h>
#include <getopt.h>
#include <string.h>
char *myprog;

int getDiff(char s1 [], char s2 [], int caseS){
    
    int count = 0, length = 0;
    if (strlen(s1) > strlen(s2)){
        length = strlen(s2);
    }
    else{
        length = strlen(s1);
    }
    
    
        for (int i = 0; i < length-1; i++)
        {
            char cha1[2] = "\0", cha2[2] = "\0";
    	    cha1[0] = s1[i];
            cha2[0] = s2[i];
            
            if (caseS == 0 && strncasecmp(cha1, cha2, 1) != 0 ){
                count ++;

            }
            if (caseS == 1 && strncmp(cha1, cha2, 1) != 0){
                count++;
            }
            
            
        }
   
              
    return count;
}

void myCompare(char *f1, char *f2, char *out, int caseS){
    char buffer1 [1024], buffer2 [1024];
    int line = 1, count = 0;
    FILE *in1 = fopen(f1,"r");
    FILE *in2 = fopen(f2,"r");
    FILE *ou = fopen(out,"w");

    if (in1 == NULL)
    {
        //fopen failed
    }
    if (in2 == NULL)
    {
        //fopen failed
    } 
    if (out == NULL)
    {
        //fopen failed
    }
    
    while (fgets(buffer1, sizeof(buffer1), in1) != NULL && fgets(buffer2, sizeof(buffer2), in2) != NULL)
    {
        count = getDiff(buffer1, buffer2, caseS);

        if (count > 0)
        {
            
            if(fprintf(ou, "Line: %i, characters: %i\n",line,count) < 0){
                //fprintf failed
            }
        }
        line++;
        count = 0;
    }
    fclose(in1);
    fclose(in2);
    fclose(ou);
}



int main(int argc, char *argv[]){
    char *outFile = "stdout.txt", *file1 = "difftest1.txt", *file2 = "difftest2.txt";
    int caseSensitive = 1; // 0 = false, 1 = true
    myprog = argv[0];
    int c; 
    while ((c = getopt(argc, argv, "io:")) != -1)   
    {
        switch (c)
        {
        case 'i':
            caseSensitive = 0;
            break;
        case 'o':
            outFile = optarg;
            break;

        case '?':

            break;
        default:
        
        break;
        }
    }

    myCompare(file1, file2, outFile, caseSensitive);
 
    
    return 0;
    
}

//gcc −std=c99 −pedantic −Wall −D_DEFAULT_SOURCE −D_BSD_SOURCE −D_SVID_SOURCE −D_POSIX_C_SOURCE=200809L −g −c mydiff.c