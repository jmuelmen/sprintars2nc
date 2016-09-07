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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sprintars2nc.h"

const int idim=640, jdim=320, kdim=57, tdim=1472;

int main (int argc, char *argv[])
{
     /* file names */
     char in_fname[1024];
     char out_fname[1024];
     char lonfile[1024], latfile[1024],
	  pfile[1024], tfile[1024];
     char varname[1024], varunits[1024];
     char strftime_buf[1024];

     /* direct time dimension specifications */
     long int tstep;
     long int t0;

     /* options with their defaults */
     nc_t out_format;
     int compress;
     int progress;
     int clobber;

     /* dimensions */
     int n_lon = 0, n_lat = 0, n_p = 0, n_t = 0;
     float *vals_lon = 0, *vals_lat = 0, *vals_p = 0;
     float *vals_t_tmp = 0;
     long int *vals_t = 0;
     dim_t dimensions = DIM2;

     /* status flags for input file */
     int err = 0;

     /* diagnostics */
     diag_t *diag = 0;

     /* process options */
     opts(argc, argv, in_fname, out_fname,
	  lonfile, latfile, pfile, tfile,
	  &t0, &tstep,
	  varname, varunits,
	  &dimensions,
	  &out_format, &compress, &progress, &clobber);

     if (verbose()) {
	  printf("\nin: %s\nout: %s\nformat: %s\ncompress: %d\n"
		 "clobber: %s\n",
		 in_fname, out_fname,
		 (out_format == NC4 || compress > 0) ? "NetCDF4" : "NetCDF2",
		 compress, 
		 clobber ? "yes" : "no");
	  
	  printf("lon: %s\nlat: %s\nlev: %s\nt: %s\t",
		 lonfile, latfile,
		 strlen(pfile) > 0 ? pfile : "2D field",
		 strlen(tfile) > 0 ? tfile : "");
	  if (t0 != -1 && tstep != -1) {
	       strftime(strftime_buf, 1024, "%Y-%m-%d %H:%M:%S UTC",
			gmtime(&t0));
	       printf("t0: %ld (%s)\ttstep: %ld s", t0, strftime_buf,
		      tstep);
	       printf("\n");
	  }
     }

     /* read dimension files */
     if (verbose()) 
	  printf("reading lon file %s\n", lonfile);
     read_table(lonfile, &vals_lon, &n_lon);
     /* if longitudes are too periodic, adjust */
     if (vals_lon[n_lon - 1] - vals_lon[0] == 360)
	  n_lon--;
     if (verbose()) 
	  printf("reading lat file %s\n", latfile);
     read_table(latfile, &vals_lat, &n_lat);
     if (strlen(pfile) != 0) {
	  if (verbose()) 
	       printf("reading lvl file %s\n", pfile);
	  read_table(pfile, &vals_p, &n_p);
     }
     if (strlen(tfile) != 0) {
	  if (verbose()) 
	       printf("reading t file %s\n", tfile);
	  read_table(pfile, &vals_t_tmp, &n_t);
	  vals_t = (long int *)malloc(sizeof(long int) * n_t);
	  for (int i = 0; i < n_t; vals_t[i] = vals_t_tmp[i++]);
	  free(vals_t_tmp);
     } else {
	  /* nothing: generate them on the fly while converting */
     }
     
     /* open input file */
     open_sprintars(in_fname, &err);
     if (err != 0) {
	  perror("Opening input file");
     }

     /* define output file */
     open_nc(out_fname, out_format, clobber, compress,
	     dimensions,
	     n_lon, n_lat, n_p, 
	     // vals_lon, vals_lat, vals_p,
	     varname, varunits);

     /* allocate transfer buffer */
     init_convert(n_lon, n_lat, dimensions == DIM2 ? 1 : n_p);
     
     /* read from input file and write to output file until the input
      * file ends */
     diag = init_diag(malloc(sizeof(diag_t)));
     while (1) {
	  if (progress) {
	       /* clear statistics */
	       diag = init_diag(diag);
	  }
	  int res = convert_tstep(diag);
	  if (res == EOF) /* EOF is expected at some point */
	       break;
	  else if (res != 0) { /* anything else is an error */
	       perror("Error during conversion, aborting");
	       exit(1);
	  }
	  if (strlen(tfile) == 0) {
	       /* keep track of time dimension */
	       n_t++;
	  }
	  if (progress)
	       display_diag(diag);
     }
     if (progress)
	  printf("\n");

     if (strlen(tfile) == 0) {
	  vals_t = (long int *)malloc(sizeof(long int) * n_t);
	  for (int i = 0; i < n_t; vals_t[i] = i++ * tstep + t0);
     }

     /* close output file (system takes care of closing input file) */
     close_nc(dimensions,
	      n_lon, n_lat, n_p, n_t,
	      vals_lon, vals_lat, vals_p, vals_t);
     
     return 0;
}
