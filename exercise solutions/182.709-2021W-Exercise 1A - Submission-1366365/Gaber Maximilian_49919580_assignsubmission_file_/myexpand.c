#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char myexpand(char *text, int tabstop) {
    int len = strlen(text);
    char *selected;
    for (int i = 0, counter = 0, letterCount = 0; i < len; ++i) {
        if (text[i] != 116 && text[i] != 92) {
            selected = &text[i];
            printf("%c", *selected);
            letterCount++;
        } else {
            counter++;
            if (counter == 2) {
                int space = tabstop - ((letterCount) % tabstop);
                for (int j = 0; j < space; ++j) {
                    printf(" ");
                }
                counter = 0;
                letterCount = 0;
            }
        }
    }
    return 0;
}

char myexpandOutfile(char *text, int tabstop, FILE *fp) {
    int len = strlen(text);
    char *selected;
    for (int i = 0, counter = 0, letterCount = 0; i < len; ++i) {
        if (text[i] != 116 && text[i] != 92) {
            selected = &text[i];
            printf("%c", *selected);
            fputc(*selected, fp);
            letterCount++;
        } else {
            counter++;
            if (counter == 2) {
                int space = tabstop - ((letterCount) % tabstop);
                for (int j = 0; j < space; ++j) {
                    fputc(32, fp);
                    printf(" ");
                }
                counter = 0;
                letterCount = 0;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int tabstop = 8;
    if (argc > 1) {
        FILE *fp;
        FILE *fp2 = NULL;
        int opt;
        char *filename;
        while ((opt = getopt(argc, argv, "t:o:")) != -1) {
            int i = 1;
            switch (opt) {
                case 't':
                    tabstop = (int) strtol(argv[i + 1], (char **) NULL, 10);
                    i += 2;
                    break;
                case 'o':
                    filename = optarg;
                    fp2 = fopen(filename, "w");
                    i += 2;
                    if (fp2 == NULL) {
                        printf("Error in opening file %s", filename);
                        return (1);
                    }
                    break;
                case '?':
                    printf("unknown option: %c\n", optopt);
                    break;
            }
        }
        for (; optind < argc; optind++) {
            //printf("extra arguments: %s\n", argv[optind]);
            fp = fopen(argv[optind], "r");
            if (fp == NULL) {
                printf("Error in opening file");
                return (1);
            }
            char text[100];
            while (fgets(text, sizeof(text), fp)) {
                //print the line
                //saves in file
                if (fp2 != NULL) {
                    myexpandOutfile(text, tabstop, fp2);
                }
            }
            printf("\n");
            fputc(13,fp2);
        }
        fclose(fp);
        fclose(fp2);
    } else {
        printf("Enter a value: ");
        char input[100];
        fgets(input, 100, stdin);
        myexpand(input, tabstop);
    }
    return 0;
}