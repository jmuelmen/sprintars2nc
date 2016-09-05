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
     read_sprintars_tstep_3d(buf, &idim, &jdim, &kdim, 
			     &eof, &err);
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
     return 0;
}
