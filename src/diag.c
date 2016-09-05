#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "sprintars2nc.h"

diag_t *init_diag (diag_t *diag)
{
     assert(diag != 0);
     diag->val_min = 2e20;
     diag->val_max = -2e20;
     diag->val_mean = 0;
     return diag;
}

void display_diag (const diag_t *diag)
{
     assert(diag != 0);
     if (isatty(fileno(stdout))) {
	  printf("\015\033[32m ---> \033[1m\033[31mtstep %5d\t"
		 "min %5.5g\t"
		 "max %5.5g\t"
		 "mean %5.5g\t"
     	         "\033[0m\033[32m <---\033[0m\015",
		 diag->tstep,
		 diag->val_min,
		 diag->val_max,
		 diag->val_mean);
     	  fflush(stdout);
     }
}
