#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sprintars2nc.h"

void usage ()
{
     /* reassign to stderr, since we're exiting anyway */
     stdout = stderr;
     printf("sprintars2nc [options] infile outfile\n\n");
     printf("options:\n");
     printf("-c | --compress            (default: off)  "
            "enable compression (implies -f nc4)\n");
     printf("-f | --format <nc2 | nc4>  (default: nc2)  "
            "create NetCDF v2 or v4 file?\n");
     printf("-p | --progress            (default: off)  "
            "enable progress bar\n");
     printf("--clobber                  (default: off)  "
            "overwrite output file if it exists\n");
     printf("\n"
            "infile:    unformatted FORTRAN big-endian SPRINTARS output\n"
            "outfile:   NetCDF output file\n"
          );
     exit(1);
}

void opts (int argc, char *argv[],
	   char in_fname[1024], char out_fname[1024],
	   nc_t *format, int *compress, int *progress, int *clobber)
{
     /* defaults */
     *format = NC2;
     *compress = 0;
     *progress = 0;
     *clobber = 0;

     /* process options */
     while (1) {
	  int option_index = 0;
	  static struct option long_options[] = {
	       {"format",    required_argument, 0,  'f' },
	       {"compress",  no_argument,       0,  'c' },
	       {"progress",  no_argument,       0,  'p' },
	       {"clobber",   no_argument,       0,  0 },
	       {0,           0,                 0,  0 }
	  };
	  const int c = getopt_long(argc, argv, "cf:p",
				    long_options, &option_index);
	  if (c == -1)
	       break;
	  switch (c) {
	  case 0:
	       if (strcmp(long_options[option_index].name,
			   "clobber") == 0) {
		    *clobber = 1;
	       } else {
		    usage();
		    exit(1);
	       }
	       break;
	  case 'c':
	       *compress = 1;
	       break;
	  case 'f':
	       if (strcmp(optarg, "nc2") == 0) {
		    *format = NC2;
		    break;
	       } else if (strcmp(optarg, "nc4") == 0) {
		    *format = NC4;
		    break;
	       } else {
		    fprintf(stderr, "unknown format %s\n", optarg);
		    usage();
		    exit(1);
	       }
	       break;
	  case 'p':
	       *progress = 1;
	       break;
	  case '?':
	       usage();
	       exit(1);
	       break;
	  default:
	       printf("?? getopt returned character code 0%o ??\n", c);
	  }
     }
     
     /* process input and output file name */
     if (optind != argc - 2) {
	  usage();
	  exit(1);
     }
     if (strlen(argv[optind]) < 1024 - 1) {
	  strncpy(in_fname, argv[optind++], 1024);
     } else {
	  fprintf(stderr,
		  "Sorry, input file path can only be %d characters long\n",
		  1024 - 1);
	  exit(1);
     }
     if (strlen(argv[optind]) < 1024 - 1) {
	  strncpy(out_fname, argv[optind++], 1024);
     } else {
	  fprintf(stderr,
		  "Sorry, output file path can only be %d characters long\n",
		  1024 - 1);
	  exit(1);
     }
}
