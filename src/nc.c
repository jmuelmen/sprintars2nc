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

/* This file borrows heavily from the NetCDF4 example program
 * http://www.unidata.ucar.edu/software/netcdf/docs/pres__temp__4D__wr_8c_source.html */

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
			((format == NC4 || compress > 0) ? NC_NETCDF4 : 0),
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
     nc_check(nc_def_var(ncid, "time", NC_INT, 1, &rec_dimid, 
			 &rec_varid));
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
	      int *vals_t)
{
     assert(ncid != -1);
     nc_check(nc_put_var_float(ncid, lat_varid, vals_lat));
     nc_check(nc_put_var_float(ncid, lon_varid, vals_lon));
     if (dimension != DIM2) {
	  nc_check(nc_put_var_float(ncid, lvl_varid, vals_p));
     }
     nc_check(nc_put_var_int(ncid, rec_varid, vals_t));
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
