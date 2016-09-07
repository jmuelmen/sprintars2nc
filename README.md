# sprintars2nc
Convert SPRINTARS unformatted Fortran output to NetCDF

![Screenshot](screenshot.png)

## Building

###Obtaining the code
```bash
git clone git@github.com:jmuelmen/sprintars2nc
```

or

```bash
wget https://api.github.com/repos/jmuelmen/sprintars2nc/tarball/v1.0 -O sprintars.tar.gz
tar xf sprintars.tar.gz
```

###Configuring

Prerequisites

   1. NetCDF library and headers, v4.0.0 or higher
   1. Fortran compiler that supports the `CONVERT` specifier in `OPEN` and the
   `BIND (C)` interoperability keywords from the Fortran 2003 standard;
   `gfortran` v4 or higher is a good choice.

If your NetCDF installation includes the `nc-config` utility, the `Makefile`
will use it to determine the necessary compiler and linker flags.  Otherwise,
you will need to specify them yourself by editing `src/Makefile`:

```Makefile
# if nc-config does not exist in your NetCDF installation, set
# this by hand, e.g.:
# NCFLAGS = -I/opt/netcdf/include
# NCLIBS = -L/opt/netcdf/lib -lnetcdf

# if nc-config does exist, these flags can be determined automatically
NCFLAGS = $(shell nc-config --cflags)
NCLIBS = $(shell nc-config --libs)
```

The `Makefile` also contains settings for the C and Fortran compilers.  By
default, it uses `gcc` and `gfortran`.

###Compiling
```bash
cd src
make
```

## Running

**Usage:**  
`sprintars2nc [options] infile outfile`

|Option                                        |Meaning|
|:---                                          |:---|
|`-c | --compress`            (default: off)   |enable compression (implies `-f nc4`)|
|`-f | --format nc2 | nc4`    (default: nc2)   |create NetCDF v2 or v4 file?|
|`-h | --help`                                 |print this message and exit|
|`-p | --progress`            (default: off)   |enable progress bar|
|`-v | --verbose`                              |increase verbosity; may be repeated|
|`-v`                                          |print version and exit|
|`--clobber`                  (default: off)   |overwrite output file if it exists|
|`--lonfile <file>`           (mandatory)      |file specifying the longitude dim|
|`--latfile <file>`           (mandatory)      |file specifying the latitude dim|
|`--pfile <file> | --sigmafile <file>`         |file specifying the vertical dim (mandatory for 3D fields)|
|`--tfile <file> | --t0 <t0> --tstep <step>`   |specification of the time dim|
|                                              |`<t0>`: start date (as 'YYYY-mm-dd HH:MM:SS' UTC)|
|                                              |`<step>`: time step in seconds|
|`--varname <name>`           (mandatory)      |variable name in NetCDF output file|
|`--varunits <units>`         (mandatory)      |variable units in NetCDF output file|

`infile`:    unformatted FORTRAN big-endian SPRINTARS output  
`outfile`:   NetCDF output file

**Example:**
```bash
sprintars2nc -vvv -f nc4 -c -p --clobber \
  --lonfile GLON640.txt --latfile GGLA320.txt \
  --t0="2000-01-01 00:00:00" --tstep=$((3 * 3600)) \
  --varname=ps --varunits=hPa \
  ps_3hr ps_3hr.nc
```

**Layout of the converted file:**
```
netcdf ps_3hr {
dimensions:
        lat = 320 ;
        lon = 640 ;
        time = UNLIMITED ; // (1456 currently)
variables:
        float lat(lat) ;
                lat:units = "degrees north" ;
        float lon(lon) ;
                lon:units = "degrees east" ;
        int time(time) ;
                time:units = "seconds since 1970-01-01 00:00:00 UTC" ;
        float ps(time, lat, lon) ;
                ps:units = "hPa" ;
}
```

## Contact
Johannes Mülmenstädt  
https://github.com/jmuelmen

## License
GNU General License Version 3  
For more information, see LICENSE
