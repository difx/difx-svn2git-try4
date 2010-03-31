/***************************************************************************
 *   Copyright (C) 2008-2010 by Walter Brisken                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id$
// $HeadURL$
// $LastChangedRevision$
// $Author$
// $LastChangedDate$
//
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "difxio/difx_input.h"
#include "difxio/difx_write.h"


DifxAntenna *newDifxAntennaArray(int nAntenna)
{
	DifxAntenna* da;
	int a;

	da = (DifxAntenna *)calloc(nAntenna, sizeof(DifxAntenna));
	for(a = 0; a < nAntenna; a++)
	{
		da[a].spacecraftId = -1;
	}
	
	return da;
}

void allocateDifxAntennaFiles(DifxAntenna *da, int nFile)
{
	int i;

	if(da->file)
	{
		for(i = 0; i < da->nFile; i++)
		{
			if(da->file[i])
			{
				free(da->file[i]);
				da->file[i] = 0;
			}
		}
		free(da->file);
		da->file = 0;
	}
	
	da->nFile = nFile;
	da->file = (char **)calloc(nFile, sizeof(char *));
}

void deleteDifxAntennaArray(DifxAntenna *da, int nAntenna)
{
	int a, i;

	if(da)
	{
		for(a = 0; a < nAntenna; a++)
		{
			if(da[a].nFile > 0)
			{
				for(i = 0; i < da[a].nFile; i++)
				{
					if(da[a].file[i])
					{
						free(da[a].file[i]);
						da[a].file[i] = 0;
					}
				}
				free(da[a].file);
				da[a].file = 0;
			}
		}
		free(da);
	}
}

void fprintDifxAntenna(FILE *fp, const DifxAntenna *da)
{
	fprintf(fp, "  DifxAntenna [%s] : %p\n", da->name, da);
	fprintf(fp, "    Delay = %f us\n", da->delay);
	fprintf(fp, "    Rate = %e us/s\n", da->rate);
	fprintf(fp, "    Clock Epoch = %14.8f\n", da->clockEpoch);
	fprintf(fp, "    Mount = %s\n", da->mount);
	fprintf(fp, "    Offset = %f, %f, %f m\n", 
		da->offset[0], da->offset[1], da->offset[2]);
	fprintf(fp, "    X, Y, Z = %f, %f, %f m\n", da->X, da->Y, da->Z);
	fprintf(fp, "    VSN = %s\n", da->vsn);
	fprintf(fp, "    nFile = %d\n", da->nFile);
	fprintf(fp, "    SpacecraftId = %d\n", da->spacecraftId);
}

void printDifxAntenna(const DifxAntenna *da)
{
	fprintDifxAntenna(stdout, da);
}

void fprintDifxAntennaSummary(FILE *fp, const DifxAntenna *da)
{
	fprintf(fp, "  %s\n", da->name);
	fprintf(fp, "    Clock: Delay = %f us  Rate = %e us/s  Epoch = %12.6f\n", 
		da->delay, da->rate, da->clockEpoch);
	fprintf(fp, "    Mount = %s\n", da->mount);
	fprintf(fp, "    Offset = %f, %f, %f m\n", 
		da->offset[0], da->offset[1], da->offset[2]);
	fprintf(fp, "    X, Y, Z = %f, %f, %f m\n", da->X, da->Y, da->Z);
	fprintf(fp, "    VSN = %s\n", da->vsn);
	fprintf(fp, "    nFile = %d\n", da->nFile);
	if(da->spacecraftId >= 0)
	{
		fprintf(fp, "    SpacecraftId = %d\n", da->spacecraftId);
	}
}

void printDifxAntennaSummary(const DifxAntenna *da)
{
	fprintDifxAntennaSummary(stdout, da);
}

int isSameDifxAntenna(const DifxAntenna *da1, const DifxAntenna *da2)
{
	if(strcmp(da1->name, da2->name) == 0 &&
	   fabs(da1->X - da2->X) < 1.0 &&
	   fabs(da1->Y - da2->Y) < 1.0 &&
	   fabs(da1->Z - da2->Z) < 1.0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int isSameDifxAntennaClock(const DifxAntenna *da1, const DifxAntenna *da2)
{
	double deltat;

	if(da1->rate != da2->rate)
	{
		return 0;
	}

	deltat = (da1->delay - da2->delay) + da1->rate*(da2->clockEpoch - da1->clockEpoch)*86400.0;

	if(fabs(deltat) > 1.0e-10)	/* give a 10^-16 sec tolerance. */
	{
		return 0;
	}

	return 1;
}

void copyDifxAntenna(DifxAntenna *dest, const DifxAntenna *src)
{
	int i;
	
	strcpy(dest->name, src->name);
	dest->delay      = src->delay;
	dest->rate       = src->rate;
	dest->clockEpoch = src->clockEpoch;
	strcpy(dest->mount, src->mount);
	for(i = 0; i < 3; i++)
	{
		dest->offset[i] = src->offset[i];
	}
	dest->X  = src->X;
	dest->Y  = src->Y;
	dest->Z  = src->Z;
	dest->dX = src->dX;
	dest->dY = src->dY;
	dest->dZ = src->dZ;
	dest->networkPort = src->networkPort;
	dest->windowSize  = src->windowSize;
	strcpy(dest->vsn, src->vsn);
	if(src->nFile > 0)
	{
		allocateDifxAntennaFiles(dest, src->nFile);
		for(i = 0; i < src->nFile; i++)
		{
			dest->file[i] = strdup(src->file[i]);
		}
	}
}

DifxAntenna *mergeDifxAntennaArrays(const DifxAntenna *da1, int nda1,
	const DifxAntenna *da2, int nda2, int *antennaIdRemap, int *nda)
{
	int i, j;
	DifxAntenna *da;

	*nda = nda1;

	/* first identify entries that differ and assign new antennaIds */
	for(j = 0; j < nda2; j++)
	{
		for(i = 0; i < nda1; i++)
		{
			if(isSameDifxAntenna(da1 + i, da2 + j))
			{
				antennaIdRemap[j] = i;
				break;
			}
		}
		if(i == nda1)
		{
			antennaIdRemap[j] = *nda;
			(*nda)++;
		}
	}

	da = newDifxAntennaArray(*nda);
	
	/* now copy da1 */
	for(i = 0; i < nda1; i++)
	{
		copyDifxAntenna(da + i, da1 + i);
	}

	/* now copy unique members of da2 */
	for(j = 0; j < nda2; j++)
	{
		if(antennaIdRemap[j] >= nda1)
		{
			copyDifxAntenna(da + antennaIdRemap[j], da2 + j);
		}
	}

	return da;
}

int writeDifxAntennaArray(FILE *out, int nAntenna, const DifxAntenna *da, double mjdStart, 
	int doMount, int doOffset, int doCoords, int doClock, int doShelf)
{
	int n;	/* number of lines written */
	int i;
	double delay;	/* clock delay corrected for start time */

	if(doClock)
	{
		writeDifxLineInt(out, "TELESCOPE ENTRIES", nAntenna);
	}
	else
	{
		writeDifxLineInt(out, "NUM TELESCOPES", nAntenna);
	}
	n = 1;

	for(i = 0; i < nAntenna; i++)
	{
		if(doClock)
		{
			writeDifxLine1(out, "TELESCOPE NAME %d", i, da[i].name);
		}
		else
		{
			writeDifxLine1(out, "TELESCOPE %d NAME", i, da[i].name);
		}
		n++;
		if(doMount)
		{
			writeDifxLine1(out, "TELESCOPE %d MOUNT", i,
				da[i].mount);
			n++;
		}
		if(doOffset)
		{
			writeDifxLineDouble1(out, "TELESCOPE %d OFFSET (m)", i, 
				"%8.6f", da[i].offset[0]);
			n++;
		}
		if(doCoords)
		{
			writeDifxLineDouble1(out, "TELESCOPE %d X (m)", i,
				"%8.6f", da[i].X);
			writeDifxLineDouble1(out, "TELESCOPE %d Y (m)", i,
				"%8.6f", da[i].Y);
			writeDifxLineDouble1(out, "TELESCOPE %d Z (m)", i,
				"%8.6f", da[i].Z);
			n += 3;
		}
		if(doClock)
		{
			delay = da[i].delay + da[i].rate*(mjdStart - da[i].clockEpoch)*86400.0;
			writeDifxLineDouble1(out, "CLOCK DELAY (us) %d", i,
				"%17.15f", delay);
			writeDifxLineDouble1(out, "CLOCK RATE(us/s) %d", i,
				"%10.8e", da[i].rate);
			n += 2;
		}
		if(doShelf)
		{
			writeDifxLine1(out, "TELESCOPE %d SHELF", i,
				da[i].shelf);
			n++;
		}
	}

	return n;
}
