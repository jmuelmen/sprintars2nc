#ifndef sprintars2nc_include
#define sprintars2nc_include

typedef enum { NC2, NC4 } nc_t;
typedef enum { DIM2, DIM3 } dim_t;

/* prototype for processing arguments, opts.c */
void opts (int argc, char *argv[],
	   char *in_fname, char *out_fname,
	   nc_t *format, int *compress, int *progress, int *clobber);

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
int open_nc(const char *, nc_t format, int clobber);
int close_nc();
int write_nc();

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
