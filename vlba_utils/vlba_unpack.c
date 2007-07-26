#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "vlba_stream.h"

void initmodbits();

struct VLBA_format *new_VLBA_format(int nbit, int nchan, int fanout,
        int format)
{
	struct VLBA_format *vf;

	int c, delta;

	if(!modbits) initmodbits();

	if(nbit < 1 || nbit > 2) return 0;
	if(fanout != 1 && fanout != 2 && fanout != 4) return 0;
	if(nchan > 32) return 0;
	
	vf = (struct VLBA_format *)malloc(sizeof(struct VLBA_format));
	vf->nbit   = nbit;
	vf->nchan  = nchan;
	vf->fanout = fanout;
	vf->format = format;
	vf->tracks = nbit*nchan*fanout;
	
	if(vf->format == FORMAT_VLBA)
	{
		vf->firstvalid = 0;
		vf->lastvalid = PAYLOADSIZE-1;
		vf->payloadoffset = 12*vf->tracks;
		vf->framesize = FRAMESIZE;
	}
	else if(vf->format == FORMAT_MARK4)
	{
		vf->firstvalid = 96;
		vf->lastvalid = PAYLOADSIZE-1-64;
		vf->payloadoffset = 0;
		vf->framesize = PAYLOADSIZE;
	}
	else 
	{
		free(vf);
		return 0;
	}

	switch(vf->tracks)
	{
	case 8:
	case 16:
		delta = nbit*fanout;
		for(c = 0; c < nchan; c++)
			vf->basebits[c] = c*delta;
		break;
	case 32:
	case 64:
		delta = 2*nbit*fanout;
		for(c = 0; c < nchan; c++)
		{
			if(c < nchan/2)		/* Evens */
				vf->basebits[c] = c*delta;
			else			/* Odds */
				vf->basebits[c] = (c-nchan/2)*delta+1;
		}
		break;
	default:
		free(vf);
		return 0;
	}

	return vf;
}

void delete_VLBA_format(struct VLBA_format *vf)
{
	if(!vf) return;

	free(vf);
}

static int VLBA_unpack_data_2bit8(const struct VLBA_format *vf, 
	const char *indata, float **data, int nsamp)
{
	const double lut[4] = {-3.3359, 1.0, -1.0, 3.3359};
	uint8_t *buf, p;
	int i, o, s, c, m, f;
	int index;

	buf = (uint8_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+f;
					m = s + vf->fanout;
					index = ((p>>s)&1) + (((p>>m)&1)<<1);
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_1bit8(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[2] = {1.0, -1.0};
	uint8_t *buf, p;
	int i, o, s, c, f;
	int index;

	buf = (uint8_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+f;
					index = (p>>s)&1;
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_2bit16(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[4] = {-3.3359, 1.0, -1.0, 3.3359};
	uint16_t *buf, p;
	int i, o, s, c, m, f;
	int index;

	buf = (uint16_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+f;
					m = s + vf->fanout;
					index = ((p>>s)&1) + (((p>>m)&1)<<1);
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_1bit16(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[2] = {1.0, -1.0};
	uint16_t *buf, p;
	int i, o, s, c, f;
	int index;

	buf = (uint16_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+f;
					index = (p>>s)&1;
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_2bit32(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[4] = {-3.3359, 1.0, -1.0, 3.3359};
	uint32_t *buf, p;
	int i, o, s, c, m, f;
	int index;

	buf = (uint32_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+2*f;
					m = s + 2*vf->fanout;
					index = ((p>>s)&1) + (((p>>m)&1)<<1);
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_1bit32(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[2] = {1.0, -1.0};
	uint32_t *buf, p;
	int i, o, s, c, f;
	int index;

	buf = (uint32_t *)indata;

	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+2*f;
					index = (p>>s)&1;
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_2bit64(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[4] = {-3.3359, 1.0, -1.0, 3.3359};
	uint64_t *buf, p;
	int f, o, i, m, s, c;
	int index;

	buf = (uint64_t *)indata;
	
	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits64[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+2*f;
					m = s + 2*vf->fanout;
					index = ((p>>s)&1) + (((p>>m)&1)<<1);
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

static int VLBA_unpack_data_1bit64(const struct VLBA_format *vf,
	const char *indata, float **data, int nsamp)
{
	const double lut[2] = {1.0, -1.0};
	uint64_t *buf, p;
	int f, o, i, s, c;
	int index;

	buf = (uint64_t *)indata;
	
	for(i = o = 0; i < nsamp; i+=vf->fanout)
	{
		if(o < vf->firstvalid || o > vf->lastvalid)
		{
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
					data[c][i+f] = 0.0;
		}
		else
		{
			if(vf->format == 0)
				p = buf[o] ^ modbits64[o];
			else 
				p = buf[o];
			for(f = 0; f < vf->fanout; f++) 
		    		for(c = 0; c < vf->nchan; c++)
				{
					s = vf->basebits[c]+2*f;
					index = (p>>s)&1;
					data[c][i+f] = lut[index];
				}
		}

		o++;
		if(o >= PAYLOADSIZE)
		{
			buf += vf->framesize;
			o = 0;
		}
	}

	return 0;
}

void VLBA_demodulate(const struct VLBA_format *vf, char *indata, int nsamp)
{
	int nframe, f, i, j;

	if(!modbits) initmodbits();

	nframe = nsamp / (PAYLOADSIZE*vf->fanout);

	for(f = 0; f < nframe; f++)
	{
		j = 0;
		switch(vf->tracks)
		{
		case 8:
			{ 
				uint8_t *dat;
				dat = (uint8_t *)indata + f*vf->framesize;
				for(i = 0; i <= PAYLOADSIZE; i++)
					dat[i] ^= modbits[i];
			}
			break;
		case 16:
			{ 
				uint16_t *dat;
				dat = (uint16_t *)indata + f*vf->framesize;
				for(i = 0; i <= PAYLOADSIZE; i++)
					dat[i] ^= modbits[i];
			}
			break;
		case 32:
			{ 
				uint32_t *dat;
				dat = (uint32_t *)indata + f*vf->framesize;
				for(i = 0; i <= PAYLOADSIZE; i++)
					dat[i] ^= modbits[i];
			}
			break;
		case 64:
			{ 
				uint64_t *dat;
				dat = (uint64_t *)indata + f*vf->framesize;
				for(i = 0; i <= PAYLOADSIZE; i++)
					dat[i] ^= modbits64[i];
			}
			break;
		}
	}
}

/* Note -- must start at frame beginning. */
int VLBA_unpack(const struct VLBA_format *vf, const char *indata, float **data, int nsamp)
{
	indata += vf->payloadoffset;

	if(vf->nbit == 2) switch(vf->tracks)
	{
		case 8 : return VLBA_unpack_data_2bit8( vf, indata, data, nsamp);
		case 16: return VLBA_unpack_data_2bit16(vf, indata, data, nsamp);
		case 32: return VLBA_unpack_data_2bit32(vf, indata, data, nsamp);
		case 64: return VLBA_unpack_data_2bit64(vf, indata, data, nsamp);
		default: return -1;                               
	}                                                        
	else if(vf->nbit == 1) switch(vf->tracks)                
	{                                                        
		case 8 : return VLBA_unpack_data_1bit8( vf, indata, data, nsamp);
		case 16: return VLBA_unpack_data_1bit16(vf, indata, data, nsamp);
		case 32: return VLBA_unpack_data_1bit32(vf, indata, data, nsamp);
		case 64: return VLBA_unpack_data_1bit64(vf, indata, data, nsamp);
		default: return -1;
	}

	return 0;
}
