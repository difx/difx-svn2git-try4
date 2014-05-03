/***************************************************************************
 *   Copyright (C) 2014 by Walter Brisken, Adam Deller                     *
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
/*===========================================================================
 * SVN properties (DO NOT CHANGE)
 *
 * $Id: stripVDIF.c 2006 2010-03-04 16:43:04Z AdamDeller $
 * $HeadURL:  $
 * $LastChangedRevision: 2006 $
 * $Author: AdamDeller $
 * $LastChangedDate: 2010-03-04 09:43:04 -0700 (Thu, 04 Mar 2010) $
 *
 *==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include "vdifio.h"

const char program[] = "printVDIF";
const char author[]  = "Adam Deller <adeller@nrao.edu>";
const char version[] = "0.1";
const char verdate[] = "20140501";

static void usage()
{
  fprintf(stderr, "\n%s ver. %s  %s  %s\n\n", program, version,
          author, verdate);
  fprintf(stderr, "A program to dump some basic info about VDIF packets to the screen\n");
  fprintf(stderr, "\nUsage: %s <VDIF input file> <Mbps> [<prtlev>]\n", program);
  fprintf(stderr, "\n<VDIF input file> is the name of the VDIF file to read\n");
  fprintf(stderr, "\n<Mbps> is the data rate in Mbps expected for this file\n");
  fprintf(stderr, "\n<prtlev> is output type: hex short long\n");
}

int main(int argc, char **argv)
{
  const int MaxFrameSize = 16*MAX_VDIF_FRAME_BYTES;
  char buffer[MaxFrameSize];
  FILE * input;
  int readbytes, framebytes, framemjd, framesecond, framenumber, frameinvalid, datambps, framespersecond;
  long long framesread;
  vdif_header *header;
  enum VDIFHeaderPrintLevel lev = VDIFHeaderPrintLevelShort;

  if(argc < 3 || argc > 4)
  {
    usage();

    return EXIT_FAILURE;
  }
  
  input = fopen(argv[1], "r");
  if(input == NULL)
  {
    fprintf(stderr, "Cannot open input file %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  datambps = atoi(argv[2]);
  if(argc == 4)
  {
  	if(strcmp(argv[3], "hex") == 0)
	{
		lev = VDIFHeaderPrintLevelHex;
	}
	else if(strcmp(argv[3], "short") == 0)
	{
		lev = VDIFHeaderPrintLevelShort;
	}
  	else if(strcmp(argv[3], "long") == 0)
	{
		lev = VDIFHeaderPrintLevelLong;
	}
	else
	{
		fprintf(stderr, "Print level must be one of hex, short or long.\n");
		exit(EXIT_FAILURE);
	}
  }
  readbytes = fread(buffer, 1, MaxFrameSize, input); //read the VDIF header
  header = (vdif_header*)buffer;
  framebytes = getVDIFFrameBytes(header);
  if(framebytes > MaxFrameSize) {
    fprintf(stderr, "Cannot read frame with %d bytes > max (%d), formal max %d\n", framebytes, MaxFrameSize, MAX_VDIF_FRAME_BYTES);
    exit(EXIT_FAILURE);
  }
  framemjd = getVDIFFrameMJD(header);
  framesecond = getVDIFFrameSecond(header);
  framenumber = getVDIFFrameNumber(header);
  frameinvalid = getVDIFFrameInvalid(header);
  framespersecond = (int)((((long long)datambps)*1000000)/(8*(framebytes-VDIF_HEADER_BYTES)));
  printf("Frames per second is %d\n", framespersecond);
 
  rewind(input); //go back to the start

  framesread = 0;
  while(!feof(input)) {
    readbytes = fread(buffer, 1, framebytes, input); //read the whole VDIF packet
    if (readbytes < framebytes) {
      fprintf(stderr, "Header read failed - probably at end of file.\n");
      break;
    }
    header = (vdif_header*)buffer;
    framemjd = getVDIFFrameMJD(header);
    framesecond = getVDIFFrameSecond(header);
    framenumber = getVDIFFrameNumber(header);
    frameinvalid = getVDIFFrameInvalid(header);
    if(lev == VDIFHeaderPrintLevelShort && framesread % 20 == 0)
    {
      printVDIFHeader(header, VDIFHeaderPrintLevelColumns);
    }
    printVDIFHeader(header, lev);
    if(getVDIFFrameBytes(header) != framebytes) { 
      fprintf(stderr, "Framebytes has changed! Can't deal with this, aborting\n");
      break;
    }
    framesread++;
  }

  printf("Read %lld frames\n", framesread);
  fclose(input);

  return EXIT_SUCCESS;
}