/*   sprintars2nc converts SPRINTARS unformatted FORTRAN data to NetCDF 
 *   Copyright (C) 2016 Johannes Muelmenstaedt 
 
 *   This program is free software: you can redistribute it and/or modify 
 *   it under the terms of the GNU General Public License as published by 
 *   the Free Software Foundation, either version 3 of the License, or 
 *   (at your option) any later version. 
 
 *   This program is distributed in the hope that it will be useful, 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *   GNU General Public License for more details. 
 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 
 *   Bug reports and feature requests are welcome.  Contact me at
 *   johannes.muelmenstaedt@uni-leipzig.de */

#define _GNU_SOURCE 500
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sprintars2nc.h"

static int verbose_ = 0;

int verbose ()
{
     return verbose_;
}

const char *version ()
{
     static char version_[1024] = "sprintars2nc 1.0";
     return version_;
}

void usage (int code)
{
     /* reassign to stderr, since we're exiting anyway */
     if (code != 0) {
	  stdout = stderr;
     }
     
     printf("\nUsage: sprintars2nc [options] infile outfile\n\n");
     printf("options:\n");
     printf("-c | --compress            (default: off)   "
            "enable compression (implies -f nc4)\n");
     printf("-f | --format nc2 | nc4    (default: nc2)   "
            "create NetCDF v2 or v4 file?\n");
     printf("-h | --help                                 "
            "print this message and exit\n");
     printf("-p | --progress            (default: off)   "
            "enable progress bar\n");
     printf("-v | --verbose                              "
            "increase verbosity; may be repeated\n");
     printf("-v                                          "
            "print version and exit\n");
     printf("--clobber                  (default: off)   "
            "overwrite output file if it exists\n");
     printf("--lonfile <file>           (mandatory)      "
            "file specifying the longitude dim\n");
     printf("--latfile <file>           (mandatory)      "
            "file specifying the latitude dim\n");
     printf("--pfile <file> | --sigmafile <file>         "
            "file specifying the vertical dim\n"
	    "                                            "
	    " (mandatory for 3D fields)\n");
     printf("--tfile <file> | --t0 <t0> --tstep <step>   "
            "specification of the time dim\n"
	    "                                            "
	    "<t0>:   start date (as \n"
	    "                                            "
	    "        'YYYY-mm-dd HH:MM:SS' UTC)\n"
	    "                                            "
	    "<step>: time step in seconds\n");
     printf("--varname <name>           (mandatory)      "
            "variable name in NetCDF output file\n");
     printf("--varunits <units>         (mandatory)      "
            "variable units in NetCDF output file\n");
     printf("\n"
            "infile:    unformatted FORTRAN big-endian SPRINTARS output\n"
            "outfile:   NetCDF output file\n"
          );
     printf("\n\nExample:\n"
            "sprintars2nc -vvv -f nc4 -c -p --clobber \\\n"
	    "  --lonfile GLON640.txt --latfile GGLA320.txt \\\n"
	    "  --t0=\"2000-01-01 00:00:00\" --tstep=$((3 * 3600)) \\\n"
	    "  --varname=ps --varunits=hPa \\\n"
	    "  ps_3hr ps_3hr.nc\n");

     exit(code);
}

long int strtotime (const char *time)
{
     struct tm tm;
     char *ret;
     
     memset(&tm, 0, sizeof(struct tm));
     if (setenv("TZ", "UTC", 1) != 0) {
	  perror("Setting UTC timezone");
	  exit(1);
     }
     ret = strptime(time, "%Y-%m-%d %H:%M:%S %Z", &tm);
     if (ret == 0) {
     	  fprintf(stderr, "time '%s' is not in the required format "
     		  "(try YYYY-mm-dd HH:MM:SS)\n", time);
     	  exit(1);
     }
     return mktime(&tm);
     /* struct tm *tm, tm_copy; */
     /* tm = getdate(time); */
     /* if (tm == 0) { */
     /* 	  fprintf(stderr, "time '%s' could not be parsed (error %d) " */
     /* 		  "(try YYYY-mm-dd HH:MM:SS)\n", time, getdate_err); */
     /* 	  exit(1); */
     /* } */
     /* tm_copy = *tm; */
     /* return mktime(&tm_copy); */
}

void opts (int argc, char *argv[],
	   char in_fname[1024], char out_fname[1024],
	   char lonfile[1024], char latfile[1024],
	   char pfile[1024], char tfile[1024],
	   long int *t0, long int *tstep,
	   char varname[1024], char varunits[1024],
	   dim_t *dimension, 
	   nc_t *format, int *compress, int *progress, int *clobber)
{
     /* defaults */
     *dimension = DIM2;
     *format = NC2;
     *compress = 0;
     *progress = 0;
     *clobber = 0;
     strncpy(lonfile, "", 1024);
     strncpy(latfile, "", 1024);
     strncpy(pfile, "", 1024);
     strncpy(tfile, "", 1024);
     strncpy(varname, "", 1024);
     strncpy(varunits, "", 1024);
     *tstep = -1;

     /* process options */
     while (1) {
	  int option_index = 0;
	  static struct option long_options[] = {
	       {"format",    required_argument, 0,  'f' },
	       {"compress",  no_argument,       0,  'c' },
	       {"progress",  no_argument,       0,  'p' },
	       {"clobber",   no_argument,       0,  0 },
	       {"help",      no_argument,       0,  'h' },
	       {"lonfile",   required_argument, 0,  0 },
	       {"latfile",   required_argument, 0,  0 },
	       {"pfile",     required_argument, 0,  0 },
	       {"sigmafile", required_argument, 0,  0 },
	       {"tfile",     required_argument, 0,  0 },
	       {"t0",        required_argument, 0,  0 },
	       {"tstep",     required_argument, 0,  0 },
	       {"varname",   required_argument, 0,  0 },
	       {"varunits",  required_argument, 0,  0 },
	       {"verbose",   no_argument,       0,  'v' },
	       {"version",   no_argument,       0,  0 },
	       {0,           0,                 0,  0 }
	  };
	  const int c = getopt_long(argc, argv, "cf:hpv",
				    long_options, &option_index);
	  if (c == -1)
	       break;
	  switch (c) {
	  case 0:
	       if (strcmp(long_options[option_index].name,
			   "clobber") == 0) {
		    *clobber = 1;
	       } else if (strcmp(long_options[option_index].name,
				 "lonfile") == 0) {
		    strncpy(lonfile, optarg, 1024);
	       } else if (strcmp(long_options[option_index].name,
				 "latfile") == 0) {
		    strncpy(latfile, optarg, 1024);
	       } else if (strcmp(long_options[option_index].name,
				 "pfile") == 0) {
		    if (strlen(pfile) != 0) {
			 fprintf(stderr,
				 "You can only specify one of pfile | "
				 "sigmafile\n");
			 usage(1);
			 exit(1);
		    }
		    strncpy(pfile, optarg, 1024);
		    *dimension = DIM3P;
	       } else if (strcmp(long_options[option_index].name,
				 "sigmafile") == 0) {
		    if (strlen(pfile) != 0) {
			 fprintf(stderr,
				 "You can only specify one of pfile | "
				 "sigmafile\n");
			 usage(1);
			 exit(1);
		    }
		    strncpy(pfile, optarg, 1024);
		    *dimension = DIM3SIGMA;
	       } else if (strcmp(long_options[option_index].name,
				 "tfile") == 0) {
		    strncpy(tfile, optarg, 1024);
	       } else if (strcmp(long_options[option_index].name,
				 "t0") == 0) {
		    *t0 = strtotime(optarg);
	       } else if (strcmp(long_options[option_index].name,
				 "tstep") == 0) {
		    errno = 0; 
		    *tstep = strtol(optarg, 0, 0);
		    if ((errno == ERANGE && (*tstep == LONG_MAX ||
					     *tstep == LONG_MIN))
			|| (errno != 0 && *tstep == 0)) {
			 fprintf(stderr, "cannot convert %s: %s\n", optarg,
				 strerror(errno));
			 exit(1);
		    }
	       } else if (strcmp(long_options[option_index].name,
				 "varname") == 0) {
		    strncpy(varname, optarg, 1024);
	       } else if (strcmp(long_options[option_index].name,
				 "varunits") == 0) {
		    strncpy(varunits, optarg, 1024);
	       } else if (strcmp(long_options[option_index].name,
				 "version") == 0) {
		    printf("%s\n", version());
		    exit(0);
	       } else {
		    usage(1);
		    exit(1);
	       }
	       break;
	  case 'c':
	       *compress = 9;
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
		    usage(1);
		    exit(1);
	       }
	       break;
	  case 'h':
	       usage(0);
	       exit(0);
	       break;
	  case 'p':
	       *progress = 1;
	       break;
	  case 'v':
	       verbose_++;
	       break;
	  case '?':
	       usage(1);
	       exit(1);
	       break;
	  default:
	       printf("?? getopt returned character code 0%o ??\n", c);
	  }
     }
     
     /* lonfile is a mandatory argument */
     if (strlen(lonfile) == 0) {
	  fprintf(stderr,
		  "lonfile is a mandatory argument\n");
	  usage(1);
	  exit(1);
     }
     if (strlen(latfile) == 0) {
	  fprintf(stderr,
		  "latfile is a mandatory argument\n");
	  usage(1);
	  exit(1);
     }

     /* variable name and units must be given */
     if (strlen(varname) == 0) {
	  fprintf(stderr,
		  "varname is a mandatory argument\n");
	  usage(1);
	  exit(1);
     }
     if (strlen(varunits) == 0) {
	  fprintf(stderr,
		  "varunits is a mandatory argument\n");
	  usage(1);
	  exit(1);
     }

     /* t0 and tstep must be given together */
     if ((*t0 != -1) != (*tstep != -1)) {
	  fprintf(stderr,
		  "tstep and t0 must be given together\n");
	  usage(1);
	  exit(1);
     }
     
     /* if t0 and tstep are given, tfile must not be */
     if ((*t0 != -1) == (strlen(tfile) != 0)) {
	  fprintf(stderr,
		  "either tfile or tstep/t0 must be given\n");
	  usage(1);
	  exit(1);
     }
     
     /* process input and output file name */
     if (optind != argc - 2) {
	  usage(1);
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
