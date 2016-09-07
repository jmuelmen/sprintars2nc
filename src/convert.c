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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "sprintars2nc.h"

static float *buf = 0;
static int step;
static int idim, jdim, kdim;

/* initialize conversion buffer using the field dimension */
int init_convert(int idim_, int jdim_, int kdim_)
{
     idim = idim_;
     jdim = jdim_;
     kdim = kdim_;
     buf = malloc(sizeof(float) * idim * jdim * kdim);
     step = -1;
     return 0;
}

/* convert one timestep; return timestep number */
int convert_tstep(diag_t *diag)
{
     int eof, err;
     
     assert(buf != 0);
     if (kdim == 1) {
	  read_sprintars_tstep_2d(buf, &idim, &jdim, 
				  &eof, &err);
     } else {
	  read_sprintars_tstep_3d(buf, &idim, &jdim, &kdim, 
				  &eof, &err);
     }
     /* did anything abnormal happen? */
     if (eof) {
	  return EOF;
     }
     if (err != 0) {
	  perror("During conversion");
	  exit(err);
     }
     step++;
     /* diagnostics */
     if (diag != 0) {
     	  for (int i = 0; i < idim; ++i)
     	       for (int j = 0; j < jdim; ++j)
     		    for (int k = 0; k < kdim; ++k) {
     			 float val = buf[(i * jdim + j) * kdim + k];
     			 if (val > diag->val_max)
     			      diag->val_max = val;
     			 if (val < diag->val_min)
     			      diag->val_min = val;
     			 diag->val_mean += val;
     		    }
     	  diag->val_mean /= idim * jdim * kdim;
	  diag->tstep = step;
     }
     write_nc(buf, step);
     return 0;
}
