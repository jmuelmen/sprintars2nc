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
	  printf("\015\033[32m --->   \033[1m\033[31mtstep %5d\t"
		 "min %8.3g, "
		 "max %8.3g, "
		 "mean %8.3g. "
     	         "\033[0m\033[32m   <---\033[0m\015",
		 diag->tstep,
		 diag->val_min,
		 diag->val_max,
		 diag->val_mean);
     	  fflush(stdout);
     }
}
