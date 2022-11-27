#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/*
// @author David Mischling
// @date 14.11.2021
// @file mycompress.c
//
// @brief this program compresses text that is read from files or the input stream and outputs it via a file or the output stream
*/

void usage(const char name[])	
{
	fprintf(stderr, "usage: %s [-o outfile] [file...] \n", name);
	exit(EXIT_FAILURE);
}

//compressing algorithm
void compressText(FILE *out, FILE *in, int *read, int *written)
{
	int i=1;
	char cNew, cOld;

	if ((cNew = fgetc(in)) != EOF)
	{
		*read += 1;

		while (true)
		{
			cOld=cNew;
			if ((cNew = fgetc(in)) == EOF)
			{
				fprintf( out, "%c%d" , cOld, i);
				*written += 2;
				break;
			} else
			{
				*read += 1;
				if (cNew == cOld)
				{
					i++;
				}else
				{
					fprintf( out, "%c%d" , cOld, i);
					*written += 2;
					i=1;
				}
			}
		}
	}

}


//main function
int main(int argc, char *argv[])
{
	char c; 
	int i, read=0, written=0, inFilesNr;
	const char *const name = argv[0];
	const char *outFilename = NULL;
	
	//reading arguments
	while((c=getopt(argc,argv,"o:"))!=-1)
	{
		switch(c)
		{
			case 'o':
				if(outFilename != NULL)
					usage(name);
				outFilename = optarg;
				break;
			default:
				usage(name);
		}
	}

	//collecting input files
	inFilesNr = argc - optind;
	char const * inFiles[inFilesNr];
	for (i=0; i<inFilesNr; i++)
	{
		inFiles[i] = argv[i+optind];
	}

	//setting output stdout/outputfile
	FILE *outFile = stdout;
    if (outFilename != NULL)
    {
        outFile = fopen(outFilename, "w");
        if (outFile == NULL)
        {
            fprintf(stderr, "Error while trying to open %s !\n%s\n", outFilename, name);
            exit(EXIT_FAILURE);
        }
    }

	//compressing the data
	if (inFilesNr == 0)
	{
		compressText(outFile, stdin, &read, &written);	
	} else
	{
		for(i=0; i<inFilesNr; i++)
		{
			FILE *inFile = fopen(inFiles[i], "r");
			if (inFile == NULL)
			{
				fprintf(stderr, "Error while trying to open %s !\n%s\n", inFiles[i], name);
				fclose(outFile);
				exit(EXIT_FAILURE);
			} else
			{
				compressText(outFile, inFile, &read, &written);
			}

			fclose(inFile);
		}
	}

	//closing outputfile
    if (outFile != stdout)
    {
        fclose(outFile);
    }

	//result
	fprintf( stderr, "Read:\t%d characters\n" ,read);
	fprintf( stderr, "Written:\t%d characters\n" ,written);
	fprintf( stderr, "Compression ratio:\t%f%%\n" ,(float)written/(float)read*100);

	//end
	exit(EXIT_SUCCESS);
}
