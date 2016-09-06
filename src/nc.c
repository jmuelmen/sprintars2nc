#include <assert.h>
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sprintars2nc.h"

static int ncid = -1;
static int lon_dimid, lat_dimid, lvl_dimid, rec_dimid;
static int lat_varid, lon_varid, lvl_varid, rec_varid, out_varid;
static int ndims;
static int dimids[4];
static size_t start[4];
static size_t count[4];

static int retval;

#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(2);}
#define nc_check(expr) { if (retval = expr) ERR(retval) }

void open_nc(const char *out_fname, nc_t format, int clobber,
	     int compress,
	     dim_t dim,
	     int n_lon, int n_lat, int n_p, 
	     // float *vals_lon, float *vals_lat, float *vals_p,
	     const char *varname, const char *varunits)
{
     /* create file */
     nc_check(nc_create(out_fname,
			(clobber ? NC_CLOBBER : NC_NOCLOBBER) |
			(format == NC4 ? NC_NETCDF4 : 0),
			&ncid));

     /* Define the dimensions. The record dimension is defined to have
      * unlimited length - it can grow as needed. In this example it is
      * the time dimension.*/
     if (dim == DIM3P) {
	  nc_check(nc_def_dim(ncid, "pressure", n_p, &lvl_dimid));
     } else if (dim == DIM3SIGMA) {
	  nc_check(nc_def_dim(ncid, "sigma", n_p, &lvl_dimid));
     }
     nc_check(nc_def_dim(ncid, "lat", n_lat, &lat_dimid)); 
     nc_check(nc_def_dim(ncid, "lon", n_lon, &lon_dimid)); 
     nc_check(nc_def_dim(ncid, "time", NC_UNLIMITED, &rec_dimid));

     /* Define the coordinate variables. */
     nc_check(nc_def_var(ncid, "lat", NC_FLOAT, 1, &lat_dimid, 
			 &lat_varid));
     nc_check(nc_def_var(ncid, "lon", NC_FLOAT, 1, &lon_dimid, 
			 &lon_varid));
     if (dim == DIM3P) {
	  nc_check(nc_def_var(ncid, "pressure", NC_FLOAT, 1, &lvl_dimid, 
			 &lvl_varid));
     } else if (dim == DIM3SIGMA) {
	  nc_check(nc_def_var(ncid, "sigma", NC_FLOAT, 1, &lvl_dimid, 
			 &lvl_varid));
     }
     if (format == NC4) {
	  nc_check(nc_def_var(ncid, "time", NC_INT64, 1, &rec_dimid, 
			      &rec_varid));
     } else {
	  nc_check(nc_def_var(ncid, "time", NC_INT, 1, &rec_dimid, 
			      &rec_varid));
     }
     /* Assign units attributes to coordinate variables. */
     nc_check(nc_put_att_text(ncid, lat_varid, "units", 
			      strlen("degrees north"), "degrees north"));
     nc_check(nc_put_att_text(ncid, lon_varid, "units", 
			      strlen("degrees east"), "degrees east"));
     if (dim == DIM3P) {
	  nc_check(nc_put_att_text(ncid, lvl_varid, "units", 
				   strlen("Pa"), "Pa"));
     } else if (dim == DIM3SIGMA) {
	  nc_check(nc_put_att_text(ncid, lvl_varid, "units", 
				   strlen("[0-1]"), "[0-1]"));
     }
     nc_check(nc_put_att_text(ncid, rec_varid, "units", 
			      strlen("seconds since 1970-01-01 00:00:00 UTC"),
			      "seconds since 1970-01-01 00:00:00 UTC"));

     /* define output variable */
     if (dim == DIM2) {
	  const int dimids_[3] = {
	       rec_dimid, lat_dimid, lon_dimid
	  };
	  const size_t count_[3] = {
	       1, n_lat, n_lon
	  };
	  const size_t start_[3] = {
	       0, 0, 0
	  };
 	  ndims = 3;
	  memcpy(dimids, dimids_, sizeof(dimids_));
	  memcpy(count, count_, sizeof(count_));
	  memcpy(start, start_, sizeof(start_));
     } else {
	  const int dimids_[4] = {
	       rec_dimid, lvl_dimid, lat_dimid, lon_dimid
	  };
	  const size_t count_[4] = {
	       1, n_p, n_lat, n_lon
	  };
	  const size_t start_[4] = {
	       0, 0, 0, 0
	  };
 	  ndims = 4;
	  memcpy(dimids, dimids_, sizeof(dimids_));
	  memcpy(count, count_, sizeof(count_));
	  memcpy(start, start_, sizeof(start_));
     }
     nc_check(nc_def_var(ncid, varname, NC_FLOAT, ndims, 
			 dimids, &out_varid));
     if (compress > 0) {
	  nc_check(nc_def_var_deflate(ncid, out_varid, 1, 1, compress));
     }

     /* Assign units attributes to the netCDF variables. */
     nc_check(nc_put_att_text(ncid, out_varid, "units", 
			      strlen(varunits), varunits));

     /* End define mode. */
     nc_check(nc_enddef(ncid));
}

void close_nc(dim_t dimension, 
	      int n_lon, int n_lat, int n_p, int n_t,
	      float *vals_lon, float *vals_lat, float *vals_p,
	      long int *vals_t)
{
     assert(ncid != -1);
     nc_check(nc_put_var_float(ncid, lat_varid, vals_lat));
     nc_check(nc_put_var_float(ncid, lon_varid, vals_lon));
     if (dimension != DIM2) {
	  nc_check(nc_put_var_float(ncid, lvl_varid, vals_p));
     }
     nc_check(nc_put_var_long(ncid, rec_varid, vals_t));
     /* } else { */
     /* 	  /\* /\\* do a quick conversion... *\\/ *\/ */
     /* 	  /\* int *tmp = (int *)malloc(sizeof(int) * n_t); *\/ */
     /* 	  nc_check(nc_put_var_long(ncid, rec_varid, vals_t)); */
     /* } */
     
     nc_check(nc_close(ncid));
}

void write_nc(float *buf, int step)
{
     assert(ncid != -1);
     assert(buf != 0);
     start[0] = step;
     nc_check(nc_put_vara_float(ncid, out_varid, start, count, 
				buf));
     
}

#if 0
/* IDs for the netCDF file, dimensions, and variables. */
int ncid, lon_dimid, lat_dimid, lvl_dimid, rec_dimid;
int lat_varid, lon_varid, pres_varid, temp_varid;
int dimids[NDIMS];

/* The start and count arrays will tell the netCDF library where to
   write our data. */
size_t start[NDIMS], count[NDIMS];

/* Program variables to hold the data we will write out. We will only
   need enough space to hold one timestep of data; one record. */
float pres_out[NLVL][NLAT][NLON];
float temp_out[NLVL][NLAT][NLON];

/* These program variables hold the latitudes and longitudes. */
float lats[NLAT], lons[NLON];

/* Loop indexes. */
int lvl, lat, lon, rec, i = 0;

/* Error handling. */
int retval;

/* Create some pretend data. If this wasn't an example program, we
 * would have some real data to write, for example, model
 * output. */
for (lat = 0; lat < NLAT; lat++)
   lats[lat] = START_LAT + 5.*lat;
for (lon = 0; lon < NLON; lon++)
   lons[lon] = START_LON + 5.*lon;

for (lvl = 0; lvl < NLVL; lvl++)
   for (lat = 0; lat < NLAT; lat++)
  for (lon = 0; lon < NLON; lon++)
  {
     pres_out[lvl][lat][lon] = SAMPLE_PRESSURE + i;
     temp_out[lvl][lat][lon] = SAMPLE_TEMP + i++;
  }

/* Create the file. */
if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
   ERR(retval);

/* Define the dimensions. The record dimension is defined to have
 * unlimited length - it can grow as needed. In this example it is
 * the time dimension.*/
if ((retval = nc_def_dim(ncid, LVL_NAME, NLVL, &lvl_dimid)))
   ERR(retval);
if ((retval = nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)))
   ERR(retval);
if ((retval = nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)))
   ERR(retval);
if ((retval = nc_def_dim(ncid, REC_NAME, NC_UNLIMITED, &rec_dimid)))
   ERR(retval);

/* Define the coordinate variables. We will only define coordinate
   variables for lat and lon.  Ordinarily we would need to provide
   an array of dimension IDs for each variable's dimensions, but
   since coordinate variables only have one dimension, we can
   simply provide the address of that dimension ID (&lat_dimid) and
   similarly for (&lon_dimid). */
if ((retval = nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid, 
             &lat_varid)))
   ERR(retval);
if ((retval = nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid, 
             &lon_varid)))
   ERR(retval);

/* Assign units attributes to coordinate variables. */
if ((retval = nc_put_att_text(ncid, lat_varid, UNITS, 
              strlen(DEGREES_NORTH), DEGREES_NORTH)))
   ERR(retval);
if ((retval = nc_put_att_text(ncid, lon_varid, UNITS, 
              strlen(DEGREES_EAST), DEGREES_EAST)))
   ERR(retval);

/* The dimids array is used to pass the dimids of the dimensions of
   the netCDF variables. Both of the netCDF variables we are
   creating share the same four dimensions. In C, the
   unlimited dimension must come first on the list of dimids. */
dimids[0] = rec_dimid;
dimids[1] = lvl_dimid;
dimids[2] = lat_dimid;
dimids[3] = lon_dimid;

/* Define the netCDF variables for the pressure and temperature
 * data. */
if ((retval = nc_def_var(ncid, PRES_NAME, NC_FLOAT, NDIMS, 
             dimids, &pres_varid)))
   ERR(retval);
if ((retval = nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS, 
             dimids, &temp_varid)))
   ERR(retval);

/* Assign units attributes to the netCDF variables. */
if ((retval = nc_put_att_text(ncid, pres_varid, UNITS, 
              strlen(PRES_UNITS), PRES_UNITS)))
   ERR(retval);
if ((retval = nc_put_att_text(ncid, temp_varid, UNITS, 
              strlen(TEMP_UNITS), TEMP_UNITS)))
   ERR(retval);

/* End define mode. */
if ((retval = nc_enddef(ncid)))
   ERR(retval);

/* Write the coordinate variable data. This will put the latitudes
   and longitudes of our data grid into the netCDF file. */
if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
   ERR(retval);
if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
   ERR(retval);

/* These settings tell netcdf to write one timestep of data. (The
  setting of start[0] inside the loop below tells netCDF which
  timestep to write.) */
count[0] = 1;
count[1] = NLVL;
count[2] = NLAT;
count[3] = NLON;
start[1] = 0;
start[2] = 0;
start[3] = 0;

/* Write the pretend data. This will write our surface pressure and
   surface temperature data. The arrays only hold one timestep worth
   of data. We will just rewrite the same data for each timestep. In
   a real application, the data would change between timesteps. */
for (rec = 0; rec < NREC; rec++)
{
   start[0] = rec;
   if ((retval = nc_put_vara_float(ncid, pres_varid, start, count, 
                   &pres_out[0][0][0])))
  ERR(retval);
   if ((retval = nc_put_vara_float(ncid, temp_varid, start, count, 
                   &temp_out[0][0][0])))
  ERR(retval);
}

/* Close the file. */
if ((retval = nc_close(ncid)))
   ERR(retval);
#endif
