#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include "config.h"
#include "difx2fits.h"
#include "other.h"

typedef struct 
{
	double time;
	float temp, pressure, dewPoint, windSpeed, windDir, windGust, precipitation;
} WRrow;

static int parseWeather(const char *line, WRrow *wr, char *antName)
{
	int n;

	n = sscanf(line, "%s%lf%f%f%f%f%f%f%f", antName, 
		&wr->time, &wr->temp, &wr->pressure, &wr->dewPoint,
		&wr->windSpeed, &wr->windDir, &wr->precipitation, 
		&wr->windGust);
	if(n != 9)
	{
		return 0;
	}

	return 1;
}

const DifxInput *DifxInput2FitsWR(const DifxInput *D,
	struct fits_keywords *p_fits_keys, struct fitsPrivate *out)
{
	/*  define the flag FITS table columns */
	struct fitsBinTableColumn columns[] =
	{
		{"TIME", "1D", "time of measurement", "DAYS"},
		{"TIME_INTERVAL", "1E", "time span over which data applies", 
			"DAYS"},
		{"ANTENNA_NO", "1J", "antenna id from antennas tbl", 0},
		{"TEMPERATURE", "1E", "ambient temperature", "CENTIGRADE"},
		{"PRESSURE", "1E", "atmospheric pressuresure", "MILLIBARS"},
		{"DEWPOINT", "1E", "dewpoint", "CENTIGRADE"},
		{"WIND_VELOCITY", "1E", "wind velocity", "M/SEC"},
		{"WIND_DIRECTION", "1E", "wind direction", "DEGREES"},
		{"WIND_GUST", "1E", "wind gust", "M/SEC"},
		{"PRECIPITATION", "1E", "integrated rain since midnight", "CM"},
		{"WVR_H2O", "0E", "", ""},
		{"IONOS_ELECTRON", "0E", "", ""}
	};

	WRrow wr;
	int i;
	int nColumn;
	int nRowBytes;
	char **fitsbuf, *p_fitsbuf;
	char antName[64];
	char line[1000];
	double mjd;
	int refDay;
	double time;
	float timeInt;
	FILE *in;
	int antId;
	double *mjdLast;
	/* 1-based index for FITS below */
	int32_t antId1;
	
	in = fopen("weather", "r");
	
	if(!in || D == 0)
	{
		return D;
	}

	nColumn = NELEMENTS(columns);
	
	nRowBytes = FitsBinTableSize(columns, nColumn);

	/* calloc space for storing table in FITS format */
	fitsbuf = (char **)malloc(D->nAntenna*sizeof(char *));
	for(i = 0; i < D->nAntenna; i++)
	{
		fitsbuf[i] = (char *)calloc(nRowBytes, 1);
	}

	mjdLast = (double *)calloc(D->nAntenna, sizeof(double));

	mjd2dayno((int)(D->mjdStart), &refDay);

	fitsWriteBinTable(out, nColumn, columns, nRowBytes, "WEATHER");
	arrayWriteKeys(p_fits_keys, out);
	fitsWriteInteger(out, "TABREV", 1, "");
	fitsWriteString(out, "MAPFUNC", " ", "");
	fitsWriteString(out, "WVR_TYPE", " ", "");
	fitsWriteString(out, "ION_TYPE", " ", "");
	fitsWriteEnd(out);
	
	for(;;)
	{
		fgets(line, 999, in);
		if(feof(in))
		{
			break;
		}
			
		/* ignore possible comment lines */
		if(line[0] == '#')
		{
			continue;
		}

		/* take out * from line */
		for(i = 0; line[i]; i++)
		{
			if(line[i] == '*')
			{
				line[i] = ' ';
			}
		}
		if(parseWeather(line, &wr, antName) == 0)
		{
			continue;
		}
		
		time = wr.time - refDay;
		timeInt = 0.0;
		
		antId = DifxInputGetAntennaId(D, antName);
		if(antId < 0 || antId >= D->nAntenna)
		{
			printf("skipping ant %d\n", antId);
			continue;
		}
		antId1 = antId + 1;
		
		/* see if we need to write the preceding record */
		mjd = time + (int)(D->mjdStart);
		if(mjd >= D->mjdStart && 
		   mjdLast[antId] < D->mjdStart && 
		   mjdLast[antId] > 50000.0)
		{
			fitsWriteBinRow(out, fitsbuf[antId]);
		}

		/* populate data structure for this record, regardless
		   of whether it will be written or not */
		p_fitsbuf = fitsbuf[antId];
		FITS_WRITE_ITEM(time, p_fitsbuf);
		FITS_WRITE_ITEM(timeInt, p_fitsbuf);
		FITS_WRITE_ITEM(antId1, p_fitsbuf);
		FITS_WRITE_ITEM(wr.temp, p_fitsbuf);
		FITS_WRITE_ITEM(wr.pressure, p_fitsbuf);
		FITS_WRITE_ITEM(wr.dewPoint, p_fitsbuf);
		FITS_WRITE_ITEM(wr.windSpeed, p_fitsbuf);
		FITS_WRITE_ITEM(wr.windDir, p_fitsbuf);
		FITS_WRITE_ITEM(wr.windGust, p_fitsbuf);
		FITS_WRITE_ITEM(wr.precipitation, p_fitsbuf);
		testFitsBufBytes(p_fitsbuf - fitsbuf[antId], nRowBytes, "WR");
#ifndef WORDS_BIGENDIAN
		FitsBinRowByteSwap(columns, nColumn, fitsbuf[antId]);
#endif

		/* write this record if it is withing the timerange or
		   the record immediately after the timerange */
		if( (mjd >= D->mjdStart && mjd <= D->mjdStop) ||
		    (mjd > D->mjdStop && mjdLast[antId] < D->mjdStop) )
		{
			fitsWriteBinRow(out, fitsbuf[antId]);
		}

		mjdLast[antId] = mjd;
	}

	/* close the file, free memory, and return */
	fclose(in);
	for(i = 0; i < D->nAntenna; i++)
	{
		free(fitsbuf[i]);
	}
	free(fitsbuf);
	free(mjdLast);

	return D;
}
