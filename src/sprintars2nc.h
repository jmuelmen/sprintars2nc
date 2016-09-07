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

#ifndef sprintars2nc_include
#define sprintars2nc_include

#include <time.h>

typedef enum { NC2, NC4 } nc_t;
typedef enum { DIM2, DIM3P, DIM3SIGMA } dim_t;

/* prototype for processing arguments, opts.c */
void opts (int argc, char *argv[],
	   char *in_fname, char *out_fname,
	   char lonfile[1024], char latfile[1024],
	   char pfile[1024], char tfile[1024],
	   time_t *t0, int *tstep,
	   char varname[1024], char varunits[1024],
	   dim_t *,
	   nc_t *format, int *compress, int *progress, int *clobber);
int verbose();

/* prototype functions for generating dimensions/dimvars, dims.c */
void read_table (const char *fname, float **vals, int *n);

/* prototypes for fortran subroutines that read the fortran data,
 * read_gtool.f90 */
void open_sprintars (const char *, int *err); 
void read_sprintars_tstep_3d (float *,
			      const int *idim, const int *jdim, const int *kdim,
			      int *eof, int *err);
void read_sprintars_tstep_2d (float *,
			      const int *idim, const int *jdim,
			      int *eof, int *err);

/* prototypes for NetCDF output functions, nc.c */
void open_nc(const char *, nc_t format, int clobber,
	     int compress,
	     dim_t dimension,
	     int n_lon, int n_lat, int n_p, 
	     // float *vals_lon, float *vals_lat, float *vals_p,
	     const char *varname, const char *varunits);
void close_nc(dim_t dimension,
	      int n_lon, int n_lat, int n_p, int n_t,
	      float *vals_lon, float *vals_lat, float *vals_p,
	      int *vals_t);
void write_nc(float *, int step);

/* simple diagnostics while we wait for the conversion to complete,
 * diag.c */
typedef struct {
     int tstep;
     float val_min, val_max, val_mean;
} diag_t;
diag_t *init_diag (diag_t *);
void display_diag (const diag_t *);

/* functions to perform conversion, convert.c */
int init_convert(int, int, int);
int convert_tstep(diag_t *);

#endif 
