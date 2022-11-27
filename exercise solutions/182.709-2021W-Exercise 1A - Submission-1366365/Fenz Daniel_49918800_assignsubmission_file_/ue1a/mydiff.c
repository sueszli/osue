/**
 * @file mydiff.c
 * @author Daniel FENZ, 12019861 (daniel.fenz@tuwien.ac.at) 12019861
 * @brief A programm to compare two files, line by line
 * @version 0.1
 * @date 2021-11-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char *prog_name;
/**
 * @brief prints usage, called when -h option is provided or there is a faulty call to this programm
 * @details uses prog_name to provide the name under which this program was called
 */
static void usage(void)
{
  printf("Usage: %s [-i] [-o outfile] file1 file2\n\n", prog_name);
  printf("[-o outfile]: Specify the output file; if not provided prints output to stdout instead\n");
  printf("[-i]: Specify that the programm should not differentiate between lower and upper case letter; if not provided comparison will be sensitive\n");
  printf("file1 file2: the path to the two files that should be compared\n");
}

/**
 * @brief 
 * 
 * @param c1 
 * @param c2 
 * @param iOpt 
 * @return int 
 */
static int compChar(char c1, char c2, int iOpt)
{
  if (iOpt == 0)
  {
    return strncmp(&c1, &c2, 1);
  }
  else
  {
    return strncasecmp(&c1, &c2, 1);
  }
}

/**
 * @brief frees up resources after completion
 * 
 * @param f1 filestreams of first file for comparison
 * @param f2 filestreams of first file for comparison
 * @param line_buf1 line buffer for first file, malloc'ed in getline
 * @param line_buf2 line buffer for second file, malloc'ed in getline
 */
static void freeUp(FILE *f1, FILE *f2, char *line_buf1, char *line_buf2)
{
  free(line_buf1);
  free(line_buf2);

  fclose(f1);
  fclose(f2);
}

/**
 * @brief core method, compares two files line by line and outputs either to output file or stdout
 * @details reads global variable prog_name
 * @param path1 path of the first file
 * @param path2 path of the second file
 * @param iOpt signifies if comparison should be case sensitive (i=0), or case sensitive (otherwise)
 * @param o_arg output file path, if NULL writes to stdout instead
 */
static void compareFiles(char *path1, char *path2, int iOpt, char *out_path)
{
  FILE *f1 = NULL, *f2 = NULL, *out = NULL;

  if ((f1 = fopen(path1, "r")) == NULL)
  {
    fprintf(stderr, "%s: Error opening file %s: %s\n", prog_name, path1, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if ((f2 = fopen(path2, "r")) == NULL)
  {
    fprintf(stderr, "%s: Error opening file %s: %s\n", prog_name, path2, strerror(errno));
    fclose(f1);
    exit(EXIT_FAILURE);
  }
  if (out_path != NULL)
  {
    if ((out = fopen(out_path, "w")) == NULL)
    {
      fprintf(stderr, "%s: Error opening file %s: %s\n", prog_name, out_path, strerror(errno));
      fclose(f1);
      fclose(f2);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    out = stdout;
  }

  char *line_buf1 = NULL, *line_buf2 = NULL;
  size_t line1_buf_size, line2_buf_size, smaller_len;
  ssize_t line_len1, line_len2;
  int line_res, i, line_count = 0;

  while ((line_len1 = getline(&line_buf1, &line1_buf_size, f1)) != -1 && (line_len2 = getline(&line_buf2, &line2_buf_size, f2)) != -1)
  {
    line_count++;
    line_res = 0;
    if (line_len1 > line_len2)
    {
      smaller_len = line_len2 - 1;
    }
    else
    {
      smaller_len = line_len1 - 1;
    }

    for (i = 0; i < smaller_len; i++)
    {
      if (compChar(line_buf1[i], line_buf2[i], iOpt) != 0)
      {
        line_res++;
      }
    }

    if (line_res != 0)
    {
      fprintf(out, "Line %d, characters: %d\n", line_count, line_res);
    }
  }

  // Close out file stream only if out_path != NULL, otherwise this is stdout
  if (out_path != NULL)
  {
    fclose(out);
  }

  // close the two file streams and free the line buffers from getline
  freeUp(f1, f2, line_buf1, line_buf2);
}

/**
 * @brief 
 * @details writes argv[0] to global variable prog_name
 * @param argc should be between 3-5
 * @param argv see usage for acceptable 
 * @return int 
 */
int main(int argc, char **argv)
{
  prog_name = argv[0];
  char *o_arg = NULL;
  int opt_i = 0, opt_o = 0, error = 0, c;
  while ((c = getopt(argc, argv, "io:h")) != -1)
  {
    switch (c)
    {
    case 'i':
      opt_i++;
      break;
    case 'o':
      opt_o++;
      o_arg = optarg;
      break;
    case 'h':
      usage();
      exit(EXIT_SUCCESS);
    default:
      usage();
      exit(EXIT_FAILURE);
      break;
    }
  }
  if (opt_i > 1)
  {
    fprintf(stderr, "%s, Option -i was provided %d times, expected at most 1 time\n", prog_name, opt_i);
    error = 1;
  }
  if (opt_o > 1)
  {
    fprintf(stderr, "%s, Option -o was provided %d times, expected at most 1 time\n", prog_name, opt_o);
    error = 1;
  }
  if ((argc - optind) != 2)
  {
    printf("%s, Error: Expected 2 files after options; Got %d", prog_name, (argc - optind));
    error = 1;
  }

  if (error == 1)
  {
    usage();
    exit(EXIT_FAILURE);
  }

  compareFiles(argv[optind], argv[optind + 1], opt_i, o_arg);

  exit(EXIT_SUCCESS);
}