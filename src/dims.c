#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sprintars2nc.h"

/* read dimension values from a text file, allocate memory (vals),
 * return values and number of values */
void read_table (const char *fname, float **vals, int *n)
{
     const int max_size = 4096;
     char errmsg[1024];
     char buf[1024];
     FILE *f = fopen(fname, "r");

     if (f == 0) {
	  snprintf(errmsg, 1024,
		   "Opening dimension table %s",
		   fname);
	  perror(errmsg);
	  exit(1);
     }

     *n = 0;
     *vals = (float *)malloc(sizeof(float) * max_size);

     while (1) {
	  int c = fgetc(f);
	  if (c == EOF) {
	       return;
	  } else if (c == '#') {
	       /* comment line; advance to newline */
	       fgets(buf, 1024, f);
	       if (verbose() > 1)
		    printf("#%s", buf);
	  } else {
	       int ret;
	       int i, dummy1, dummy2;
	       float val;
	       /* put character back, read line as four floats */
	       ungetc(c, f);
	       ret = fscanf(f, " %d %d %d %f",
			    &i, &dummy1, &dummy2, &val);
	       /* printf("%d %d %d %f -- n = %d -- ret = %d\n", */
	       /* 	      i, dummy1, dummy2, val, *n, ret); */
	       if (ret == 4) {
		    (*vals)[(*n)++] = val;
		    if (i != *n) {
			 /* something went out of order */
			 snprintf(errmsg, 1024,
				  "Reading from dimension table %s: "
				  "line beginning with '%d' is not in order",
				  fname, i);
			 perror(errmsg);
			 exit(1);
		    }
	       } else {
		    break;
	       }
	  }
     }
     if (verbose()) {
	  printf("read %d values, first %f ... last %f\n",
		 *n, (*vals)[0], (*vals)[*n - 1]);
     }

}

