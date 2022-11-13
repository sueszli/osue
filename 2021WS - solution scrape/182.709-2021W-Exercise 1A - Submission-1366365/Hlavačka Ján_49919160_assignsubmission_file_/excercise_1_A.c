#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include<fcntl.h> 

#define buff_block 80

void str_uppercase(char *line){
	while(*line){
		*line = toupper((unsigned char) *line);
		line++;
	}
}

void contains_keyword(char *keyword, char *buff, int buff_size, int is_case_sensitive, FILE *fp){
	char *tmp = malloc(sizeof (char) * buff_size);
	strcpy(tmp,buff);
	if(!is_case_sensitive){
		str_uppercase(tmp);
	}
	if(strstr(tmp,keyword) != NULL){
		printf("%s",buff);
		if(fp != NULL){
			fprintf(fp,"%s",buff);
		}
	}
	free(tmp);
}

int read_from_console(char *keyword, int is_case_sensitive, char *output_file){
	char *buff = malloc(sizeof (char) * buff_block);
	FILE *fp = NULL;
	if(output_file != NULL){
		fp = fopen(output_file, "w+");
	}
	
	do{
		char *str = fgets(buff, buff_block, stdin); 
		
		if (!str)
		{
    		// couldn't read input for some reason
    		return 1; 
		}
		
		contains_keyword(keyword, buff, buff_block, is_case_sensitive, fp);
	}while(buff[0] != '\n');
	
	free(buff);
	if(fp != NULL){
		fclose(fp);
	}	
	
	return 0;
}

void readLine(int fd, char *buffer)
{
    unsigned char byteRead;
    int k = 0;
    do
    {
        if (read(fd, (void *)&byteRead, 1) < 1)
        {
            return;
        }
        buffer[k++] = byteRead;
    } while (byteRead != '\n');
}

int read_from_files(char *keyword, int is_case_sensitive, char *output_file, int arg_position, int argc, char **argv){
	FILE *fp = NULL;
	if(output_file != NULL){
		fp = fopen(output_file, "w+");
	}
	
	int file_in;		
	int buff_size = buff_block;
	char *buff = malloc(sizeof (char) * buff_size);
	int file_arg;
	for(file_arg = arg_position; file_arg < argc; file_arg++){
		file_in = open(argv[file_arg], O_RDONLY);
		if(file_in < 0){
			perror("Error opening file");
			return 1;
		}
		do
		{
		    memset(buff, 0, buff_size);
		    readLine(file_in, buff);
		    contains_keyword(keyword, buff, buff_size, is_case_sensitive, fp);    
		} while (buff[0] != '\0');
		close(file_in);
    }
    free(buff);
    if(fp != NULL){
		fclose(fp);
	}
	return 0;	
}

int main(int argc, char **argv)
{	
	
	if(argc <= 1){
		printf("no argument provided, program is terminated\n");
		return 1; 
	}
	int is_case_sensitive = 1;
	char *output_file = NULL;
	char *keyword;
	int option;
	while((option = getopt(argc, argv, "io:")) != -1){ 
		switch(option){
			case 'i': 
				is_case_sensitive = 0;
				break;
			case 'o':
				output_file = optarg;
				break;
			case '?': //used for some unknown options
	            		printf("unknown option\n");
	            		return 1;
		}
	}
	
	int arg_position = 1;
	if(is_case_sensitive == 0){
		//we are taking "-i" arg into consideration
		arg_position++;
	}
	
	if(output_file != NULL){
		//we are taking "-o" arg and its value into consideration
		arg_position += 2;	
	}
	
	//keyword has to be on arg_position -> arg size should be higher
	if(argc < arg_position + 1){
		printf("keyword is missing\n");
		return 1;
	}
	
	keyword = argv[arg_position];
	
	//we are taking keyword into consideration
	arg_position++;

	//uniform case
	if(!is_case_sensitive){
		str_uppercase(keyword);
	}
	
	int ret_val = 0;
	//wether input files were provided
	if(argc == arg_position ){
		ret_val = read_from_console(keyword, is_case_sensitive, output_file);
	} else {
		ret_val = read_from_files(keyword, is_case_sensitive, output_file, arg_position, argc, argv);
	}
	return ret_val;
}
