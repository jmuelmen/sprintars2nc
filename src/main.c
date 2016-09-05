#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sprintars2nc.h"

const int idim=640, jdim=320, kdim=57, tdim=1472;

int main (int argc, char *argv[])
{
     /* file names */
     char in_fname[1024];
     char out_fname[1024];

     /* options with their defaults */
     nc_t out_format;
     int compress;
     int progress;
     int clobber;

     /* status flags for input file */
     int err = 0;

     /* diagnostics */
     diag_t *diag = 0;

     /* process options */
     opts(argc, argv, in_fname, out_fname, &out_format,
	  &compress, &progress, &clobber);

     printf("in: %s\nout: %s\nformat: %d\ncompress: %d\nprogress: %d\n"
	    "clobber: %d\n",
	    in_fname, out_fname, out_format, compress, progress, clobber);

     /* open input file */
     open_sprintars(in_fname, &err);
     if (err != 0) {
	  perror("Opening input file");
     }

     /* define output file */
     if (open_nc(out_fname, out_format, clobber) != 0) {
	  perror("Opening output file");
     }

     init_convert(idim, jdim, kdim);
     /* read from input file and write to output file until the input
      * file ends */
     while (1) {
	  if (progress) {
	       diag = init_diag(malloc(sizeof(diag_t)));
	  }
	  int res = convert_tstep(diag);
	  if (res == EOF) /* EOF is expected at some point */
	       break;
	  else if (res != 0) { /* anything else is an error */
	       perror("Error during conversion, aborting");
	       exit(1);
	  }
	  if (progress)
	       display_diag(diag);
     }

     /* close output file (system takes care of closing input file) */
     if (close_nc() != 0) {
	  perror("Closing output file");
     }
     
     return 0;
}
