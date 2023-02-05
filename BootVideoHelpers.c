/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2002-09-10  agreen@warmcat.com  created

// These are helper functions for displaying bitmap video
// includes an antialiased (4bpp) proportional bitmap font (n x 16 pixel)


#include "types.h"
#include "sysdeps.h"
#include "fontx16.h"  // brings in font struct
#include <stdarg.h>
#define WIDTH_SPACE_PIXELS 5

static inline void * scroll_memcpy(void * to, const void * from, int n)
{
int d0, d1, d2;
__asm__ __volatile__(
        "rep ; movsl\n\t"
        "movl %4,%%ecx\n\t"
        "andl $3,%%ecx\n\t"
#if 1   /* want to pay 2 byte penalty for a chance to skip microcoded rep? */
        "jz 1f\n\t"
#endif
        "rep ; movsb\n\t"
        "1:"
        : "=&c" (d0), "=&D" (d1), "=&S" (d2)
        : "0" (n/4), "g" (n), "1" ((long) to), "2" ((long) from)
        : "memory");
return (to);
}



// returns number of x pixels taken up by ascii character bCharacter

unsigned int BootVideoGetCharacterWidth(u8 bCharacter, bool fDouble)
{
	unsigned int nStart, nWidth;
	int nSpace=WIDTH_SPACE_PIXELS;
	
	if(fDouble) nSpace=8;

		// we only have glyphs for 0x21 through 0x7e inclusive

	if(bCharacter<0x21) return nSpace;
	if(bCharacter>0x7e) return nSpace;

	nStart=waStarts[bCharacter-0x21];
	nWidth=waStarts[bCharacter-0x20]-nStart;

	if(fDouble) return nWidth<<1; else return nWidth;
}

// returns number of x pixels taken up by string

unsigned int BootVideoGetStringTotalWidth(const char * szc) {
	unsigned int nWidth=0;
	bool fDouble=false;
	while(*szc) {
		if(*szc=='\2') {
			fDouble=!fDouble;
			szc++;
		} else {
			nWidth+=BootVideoGetCharacterWidth(*szc++, fDouble);
		}
	}
	return nWidth;
}

// convert pixel count to size of memory in bytes required to hold it, given the character height

// usable for direct write or for prebuffered write
// returns width of character in pixels
// RGBA .. full-on RED is opaque --> 0xFF0000FF <-- red

int BootVideoOverlayCharacter(
	u32 * pdwaTopLeftDestination,
	u32 m_dwCountBytesPerLineDestination,
	RGBA rgbaColourAndOpaqueness,
	u8 bCharacter,
	bool fDouble
) {
	int nSpace;
	unsigned int n, nStart, nWidth, y, nHeight
//		nOpaquenessMultiplied,
//		nTransparentnessMultiplied
	;
	u8 b=0, b1; // *pbColour=(u8 *)&rgbaColourAndOpaqueness;
	u8 * pbaDestStart;

		// we only have glyphs for 0x21 through 0x7e inclusive

	if(bCharacter=='\t') {
		u32 dw=((u32)pdwaTopLeftDestination) % m_dwCountBytesPerLineDestination;
		u32 dw1=((dw+1)%(32<<2));  // distance from previous boundary
		return ((32<<2)-dw1)>>2;
	}
	nSpace=WIDTH_SPACE_PIXELS;
	if(fDouble) nSpace=8;
	if(bCharacter<'!') return nSpace;
	if(bCharacter>'~') return nSpace;

	nStart=waStarts[bCharacter-(' '+1)];
	nWidth=waStarts[bCharacter-' ']-nStart;
	nHeight=uiPixelsY;

	if(fDouble) { nWidth<<=1; nHeight<<=1; }

//	nStart=0;
//	nWidth=300;

	pbaDestStart=((u8 *)pdwaTopLeftDestination);

	for(y=0;y<nHeight;y++) {
		u8 * pbaDest=pbaDestStart;
		int n1=nStart;

		for(n=0;n<nWidth;n++) {
			b=baCharset[n1>>1];
			if(!(n1&1)) {
				b1=b>>4;
			} else {
				b1=b&0x0f;
			}
			if(fDouble) {
				if(n & 1) n1++;
			} else {
				n1++;
			}

		if(b1) {
				*pbaDest=(u8)((b1*(rgbaColourAndOpaqueness&0xff))>>4); pbaDest++;
				*pbaDest=(u8)((b1*((rgbaColourAndOpaqueness>>8)&0xff))>>4); pbaDest++;
				*pbaDest=(u8)((b1*((rgbaColourAndOpaqueness>>16)&0xff))>>4); pbaDest++;
				*pbaDest++=0xff;
			} else {
				pbaDest+=4;
			}
		}
		if(fDouble) {
			if(y&1) nStart+=uiPixelsX;
		} else {
			nStart+=uiPixelsX;
		}
		pbaDestStart+=m_dwCountBytesPerLineDestination;
	}

	return nWidth;
}

// usable for direct write or for prebuffered write
// returns width of string in pixels

int BootVideoOverlayString(u32 * pdwaTopLeftDestination, u32 m_dwCountBytesPerLineDestination, RGBA rgbaOpaqueness, const char * szString)
{
	unsigned int uiWidth=0;
	bool fDouble=0;
	while((*szString != 0) && (*szString != '\n')) {
		if(*szString=='\2') {
			fDouble=!fDouble;
		} else {
			uiWidth+=BootVideoOverlayCharacter(
				pdwaTopLeftDestination+uiWidth, m_dwCountBytesPerLineDestination, rgbaOpaqueness, *szString, fDouble
				);
		}
		szString++;
	}
	return uiWidth;
}

int clean_line(u32 * s, u32 ppr)
{
int y;
uint8_t *rp=(uint8_t *) s;
for (y=0;y<uiPixelsY;++y) {
uint8_t *p=rp;
int n=ppr;
while (n--) {
*(p++)=0;
*(p++)=0;
*(p++)=0;
*(p++)=0xff;
}
rp+=4*ppr;
}


}

void BootVideoChunkedPrint(const char * szBuffer) {
	int n=0;
	int nDone=0;

	while (szBuffer[n] != 0)
	{
		if(szBuffer[n]=='\n') {
			BootVideoOverlayString(
				(u32 *)((bootparms->video.addr) + VIDEO_CURSOR_POSY * (vmode.width*4) + VIDEO_CURSOR_POSX),
				vmode.width*4, VIDEO_ATTR, &szBuffer[nDone]
			);
			nDone=n+1;
			VIDEO_CURSOR_POSY+=16; 
			VIDEO_CURSOR_POSX=vmode.xmargin<<2;
			if (VIDEO_CURSOR_POSY > (vmode.height - 16))
			{
				VIDEO_CURSOR_POSY-=16;
				scroll_memcpy((u32 *)(bootparms->video.addr),
					(u32 *) (bootparms->video.addr+16* (vmode.width*4)),
					VIDEO_CURSOR_POSY*(vmode.width*4));
			}
			clean_line( (u32 *)((bootparms->video.addr) + VIDEO_CURSOR_POSY * (vmode.width*4) ), vmode.width);
		}
		n++;
	}
	if (n != nDone)
	{
		VIDEO_CURSOR_POSX+=BootVideoOverlayString(
			(u32 *)((bootparms->video.addr) + VIDEO_CURSOR_POSY * (vmode.width*4) + VIDEO_CURSOR_POSX),
			vmode.width*4, VIDEO_ATTR, &szBuffer[nDone]
		)<<2;
		if (VIDEO_CURSOR_POSX > (vmode.width - 
			vmode.xmargin) <<2)
		{
			VIDEO_CURSOR_POSY+=16; 
			VIDEO_CURSOR_POSX=vmode.xmargin<<2;
			if (VIDEO_CURSOR_POSY > (vmode.height - 16))
			{
				VIDEO_CURSOR_POSY-=16;
				scroll_memcpy((u32 *)(bootparms->video.addr),
					(u32 *) (bootparms->video.addr+16* (vmode.width*4)),
					VIDEO_CURSOR_POSY*(vmode.width*4));
			}
			clean_line( (u32 *)((bootparms->video.addr) + VIDEO_CURSOR_POSY * (vmode.width*4) ), vmode.width);
		}
		
	}

}

int printk(const char *szFormat, ...) {  // printk displays to video
	char szBuffer[512*2];
	u16 wLength=0;
	va_list argList;
	va_start(argList, szFormat);
	wLength=(u16) vsprintf(szBuffer, szFormat, argList);
	va_end(argList);

	szBuffer[sizeof(szBuffer)-1]=0;
        if (wLength>(sizeof(szBuffer)-1)) wLength = sizeof(szBuffer)-1;
	szBuffer[wLength]='\0';
	        
	BootVideoChunkedPrint(szBuffer);
	return wLength;
}

int console_putchar(int c)
{
	char buf[2];
	buf[0] = (char)c;
	buf[1] = 0;
	BootVideoChunkedPrint(buf);
	return (int)buf[0];	
}

int putchar(int c)
{
	return console_putchar(c);
}
