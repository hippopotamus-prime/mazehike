/****************************************************************************
** Maze Hiker
** Copyright 2003 Aaron Curtis
** 
** This file is part of Maze Hiker.
** 
** Maze Hiker is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Maze Hiker is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Maze Hiker; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/


#include <PalmOS.h>
#include "main.h"
#include "palm.h"
#include "maze.h"

#define cos(a)		(costable[(a)&(Q2MAX-1)])
#define sin(a)		(costable[((a)-Q1MAX)&(Q2MAX-1)])

static void		raycast(void);
static UInt16	initbumpmap(void);
static UInt16	sqroot(const UInt32 hyp);
static Boolean	darken(const UInt8 section);
static void		lightbuffer(void);
static void		walk(void);

UInt16*		costable;
Int8*		angtable;
type_gfx	gfx;
globalvars	gv;

void doapp(void)
{
	gv.counter++;
	EvtResetAutoOffTimer();	

	switch(gv.state)
	{
		case state_init:
			globalerr = initbumpmap();
			if(!globalerr)
			{
				gv.counter	= 0;
				gv.ang		= 0;
				gv.x		= 48<<8;
				gv.y		= 48<<8;
				gv.goalang	= 0;
				gv.goalx	= 48<<8;
				gv.goaly	= 48<<8;
				initmap();
				lightbuffer();
				device.func_copybuffer();
				gv.state	= state_create;
			}
			break;

		case state_create:
			gv.waitcreate += (time.delta*CREATESPEED);
	
			if(gv.waitcreate > 1000)
			{
				gv.waitcreate = 0;

				if(carvecell())
				{
					device.func_copybuffer();
				}
				else
				{
					amc(1, 1) = tile_marked;
					addbranch(1, 1, true);
					gv.state = state_fade;
				}
			}
			break;

		case state_fade:
			if(!(gv.counter&3))
			{
				device.func_copybuffer();
			}

			if(darken(gv.counter&3))
			{
				WinHandle oldwin = WinGetDrawWindow();

				WinSetDrawWindow(WinGetDisplayWindow());	//this is probably not needed
				WinEraseWindow();
				WinSetDrawWindow(oldwin);
				closebumpmap();
				gv.state = state_walk;
			}
			break;

		case state_walk:
			raycast();
			device.func_copybuffer();

			if((gv.ang == gv.goalang)
			&& (gv.x == gv.goalx)
			&& (gv.y == gv.goaly))
			{
				markcell();

				if(!gv.progress)
				{
					//progress = 0 means all cells have been visited
					EventType quitevent;

					quitevent.eType = appStopEvent;
					EvtAddEventToQueue(&quitevent);
					gv.state = state_none;
				}
			}

			walk();
			break;
	}
}


void walk(void)
{
	if(gv.ang != gv.goalang)
	{
		Int16 da = time.delta * ROTSPEED;

		if(((gv.goalang-gv.ang+da)&(((UInt32)Q4MAX<<8)-1)) < (da<<1))
		{
			gv.ang = gv.goalang;
		}
		else if(((gv.goalang-gv.ang)&(((UInt32)Q4MAX<<8)-1)) > ((gv.ang-gv.goalang)&(((UInt32)Q4MAX<<8)-1)))
		{
			gv.ang -= da;
		}
		else
		{
			gv.ang += da;
		}
	}
	else if(gv.x != gv.goalx)
	{
		Int16 dx = time.delta * WALKSPEED;

		if(gv.goalx > gv.x + dx)
		{
			gv.x += dx;
		}
		else if(gv.goalx < gv.x - dx)
		{
			gv.x -= dx;
		}
		else
		{
			gv.x = gv.goalx;
		}
	}
	else if(gv.y != gv.goaly)
	{
		Int16 dy = time.delta * WALKSPEED;

		if(gv.goaly > gv.y + dy)
		{
			gv.y += dy;
		}
		else if(gv.goaly < gv.y - dy)
		{
			gv.y -= dy;
		}
		else
		{
			gv.y = gv.goaly;
		}
	}
}


void lightbuffer(void)
{
	Int16		i, j;
	Int16		xbri, ybri;
	UInt16		offset		= 0;
	UInt8*		bufferbits	= BmpGetBits(WinGetBitmap(gfx.buffer));
	WinHandle	oldwin		= WinGetDrawWindow();

	for(j = 0; j < 160; j++)
	{
		for(i = 0; i < 160; i++)
		{
			xbri = ((i - LIGHTX)>>2) + 32;
			ybri = ((j - LIGHTY)>>2) + 32;
			bufferbits[offset++] = gfx.lightmap[xbri + (ybri<<6)] + (tile_wall_height>>2);
		}
	}

	WinPushDrawState();
	WinSetTextColor(254);
	WinSetBackColor(0);
	WinSetDrawMode(winOverlay);
	WinSetDrawWindow(gfx.buffer);

	WinPaintChars("Maze Hiker by ARC - OSCC 2003", 29, 17, 146);

	WinPopDrawState();
	WinSetDrawWindow(oldwin);
}


Boolean darken(const UInt8 section)
{
	UInt16 i;
	Boolean finished	= true;
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.buffer));
	UInt8 fadeamt		= (time.delta*FADESPEED)>>8;

	for(i = section*40*160; i < (section+1)*40*160; i++)
	{
		if(bufferbits[i] > fadeamt)
		{
			finished = false;
			bufferbits[i] -= fadeamt;
		}
		else
		{
			bufferbits[i] = 0;
		}
	}

	return finished;
}


UInt16 sqroot(const UInt32 n)
{
	UInt32 sqrt = 0;
	UInt32 jump = 0x8000;

	while(jump)
	{
		if(n > sqrt*sqrt)
		{
			sqrt += jump;
		}
		else if(n < sqrt*sqrt)
		{
			sqrt -= jump;
		}
		else
		{
			break;
		}
		jump >>= 1;
	}

	return (UInt16)sqrt;
}


UInt16 initbumpmap(void)
{
	UInt32			i, j;
	UInt16			dist;
	RGBColorType	colors[256];
	WinHandle 		oldwin = WinGetDrawWindow();
	UInt16			err;

	//create light map
	gfx.lightmap = MemPtrNew(64*64);
	if(!gfx.lightmap) return -1;

	for(j = 0; j < 32; j++)
	{
		for(i = 0; i < 32; i++)
		{
			dist = sqroot(((i-32)*(i-32) + (j-32)*(j-32))<<6);

			if(dist > 240)
			{
				gfx.lightmap[i+(j<<6)] = 0;
				gfx.lightmap[63-i+(j<<6)] = 0;
				gfx.lightmap[i+((63-j)<<6)] = 0;
				gfx.lightmap[63-i+((63-j)<<6)] = 0;
			}
			else
			{
				gfx.lightmap[i+(j<<6)] = 240 - dist;
				gfx.lightmap[63-i+(j<<6)] = 240 - dist;
				gfx.lightmap[i+((63-j)<<6)] = 240 - dist;
				gfx.lightmap[63-i+((63-j)<<6)] = 240 - dist;
			}
		}
	}

	//set palette
	WinSetDrawWindow(WinGetDisplayWindow());

	/******* Alternate palette *********
	for(i = 0; i < 192; i++)
	{
		colors[i].r = 0;
		colors[i].g = 0;
		colors[i].b = i;
	}
	for(i = 192; i < 256; i++)
	{
		colors[i].r = ((i-192)<<2);
		colors[i].g = ((i-192)<<2);
		colors[i].b = i;
	}
	***********************************/

	for(i = 0; i < 256; i++)
	{
		colors[i].r = (i>>1) + ((i*i)>>10);
		colors[i].g = ((i*i)>>8);
		colors[i].b = i;
	}

	err = WinPalette(winPaletteSet, 0, 256, colors);

	WinSetDrawWindow(oldwin);

	return err;
}


void closebumpmap(void)
{
	WinHandle		oldwin = WinGetDrawWindow();

	if(gfx.lightmap)
	{
		MemPtrFree(gfx.lightmap);
	}

	WinSetDrawWindow(WinGetDisplayWindow());
	WinPalette(winPaletteSetToDefault, 0, 256, NULL);	

	WinSetDrawWindow(oldwin);
}


void raycast(void)
{
	UInt8* bufferbits = BmpGetBits(WinGetBitmap(gfx.buffer));
	UInt8* texbits = BmpGetBits(WinGetBitmap(gfx.textures));

	asm volatile("
		clr.w %%d6

		rayloop:
			/*******************************************************
			**	Note - the asm code does not match the C very well
			**	due to some optimizations...
			**
			**	====================================================
			**
			**	Set initial values....
			**
			**	----------------------
			**
			**	tile = ((gv.x>>5)+((gv.y>>1)&~15));
			**	a = (gv.ang + angtable[i])&(Q4MAX-1);
			**
			**	if(a < Q1MAX)
			**	{
			**		dx = 32-(gv.x&31);
			**		dy = 32-(gv.y&31);
			**		tdx = 1;
			**		tdy = 16;
			**	}
			**	else if(a < Q2MAX)
			**	{
			**		dx = (gv.x&31);
			**		dy = 32-(gv.y&31);
			**		tdx = -1;
			**		tdy = 16;
			**	}
			**	else if(a < Q3MAX)
			**	{
			**		dx = (gv.x&31);
			**		dy = (gv.y&31);
			**		tdx = -1;
			**		tdy = -16;
			**	}
			**	else
			**	{
			**		dx = 32-(gv.x&31);
			**		dy = (gv.y&31);
			**		tdx = 1;
			**		tdy = -16;
			**	}
			********************************************************/

			move.l %0,-(%%a7)				/* store bufferbits, texbits, and map */
			move.l %2,-(%%a7)
			move.l %1,-(%%a7)

			move.w %6,%%d7				/* calculate the angle of the current ray */
			movea.l %7,%1
			move.b (%1,%%d6.w),%%d0
			ext.w %%d0
			add.w %%d0,%%d7
			movea.l (%%a7),%1
			andi.w #1023,%%d7

			moveq #31,%%d0
			move.w %4,%%d1				/* get player coordinates from gv */
			and.w %%d0,%%d1
			move.w %5,%%d2
			and.w %%d0,%%d2

			cmpi.w #256,%%d7					/* check what quadrant we're in */
			bcc checkq2
				moveq #1,%%d4
				moveq #16,%%d5
				moveq #32,%%d0
				sub.w %%d1,%%d0
				move.w %%d0,-(%%a7)
				moveq #32,%%d0
				sub.w %%d2,%%d0
				move.w %%d0,-(%%a7)
				bra wallsearch
		checkq2:
			cmpi.w #512,%%d7
			bcc checkq3
				moveq #-1,%%d4
				moveq #16,%%d5	
				move.w %%d1,-(%%a7)
				moveq #32,%%d0
				sub.w %%d2,%%d0
				move.w %%d0,-(%%a7)
				bra wallsearch
		checkq3:
			cmpi.w #768,%%d7
			bcc checkq4
				moveq #-1,%%d4
				moveq #-16,%%d5	
				move.w %%d1,-(%%a7)
				move.w %%d2,-(%%a7)
				bra wallsearch
		checkq4:
				moveq #1,%%d4
				moveq #-16,%%d5
				moveq #32,%%d0
				sub.w %%d1,%%d0
				move.w %%d0,-(%%a7)
				move.w %%d2,-(%%a7)

			/*******************************************************
			**	Search for walls......
			**
			**	-------------------------------------------
			**
			**	do
			**	{
			**		if(dx * abs(sin(a)) < dy * abs(cos(a)))
			**		{
			**			tile	+= tdx;
			**			usex	= true;
			**			dx		+= 32;
			**		}
			**		else
			**		{
			**			tile	+= tdy;
			**			usex	= false;
			**			dy		+= 32;
			**		}
			**	}while(!map[tile]);
			********************************************************/

		wallsearch:
			add.w %%d7,%%d7
			move.w %%d7,%%d2			/* get the sin value in d2 */
			subi.w #512,%%d2
			andi.w #1022,%%d2
			move.w (%3,%%d2.w),%%d2
			move.w 2(%%a7),%%d0			/* get dx in d0 */
			mulu %%d2,%%d0				/* multiply dx by the sin value */
			ext.l %%d2
			lsl.l #5,%%d2				/* now multiply the sin value by the texture height */

			move.w %%d7,%%d3
			andi.w #1022,%%d3
			move.w (%3,%%d3.w),%%d3		/* same as the above, but with cos and dy */
			move.w (%%a7),%%d1
			mulu %%d3,%%d1
			ext.l %%d3
			lsl.l #5,%%d3

		loop:
			cmp %%d0,%%d1
			bcs ycheck

			/* in this case the distance to the next x-crossing is shorter */
			adda.w %%d4,%1				/* advance to the next map position */
			cmpi.b #16,(%1)
			bcc xhitwall				/* if we hit a wall, we're done here */

			add.l %%d2,%%d0				/* update dx*sin(a) */
			addi.w #32,2(%%a7)			/* also update dx */
			bra loop

		ycheck:
			/* here the distance to the next y-crossing is shorter */
			adda.w %%d5,%1				/* same as above, but with dy */
			cmpi.b #16,(%1)
			bcc yhitwall

			add.l %%d3,%%d1
			addi.w #32,(%%a7)
			bra loop

			/*******************************************************
			** Calulate column height and texture offset
			**
			** ----------------------------------------------
			**
			**	if(usex)
			**	{
			**		height = ((32*64) * (Int32)abs(cos(a))) /
			**		((Int32)(dx) * (Int32)cos(a-gv.ang));
			**	}
			**	else
			**	{
			**		height = ((32*64) * (Int32)abs(sin(a))) /
			**		((Int32)(dy) * (Int32)cos(a-gv.ang));
			**	}
			**
			**  //some more stuff...
			**
			**	if(usex)
			**	{
			**		texbits += (gv.y + ((dx)*sin(a))/cos(a))&31;
			**	}
			**	else
			**	{
			**		texbits += (gv.x + ((dy)*cos(a))/sin(a))&31;
			**	}
			********************************************************/

		xhitwall:
			move.w %%d7,%%d1
			andi.w #1022,%%d1
			move.w (%3,%%d1.w),%%d1		/* get the cos value again */
			divu %%d1,%%d0				/* calculate dx*sin(a)/cos(a) */
										/* probably should use a tangent table.... */
			cmpi.w #1024,%%d7
			bcs xtexplus
				addq.w #1,%%d0
				neg.w %%d0
		xtexplus:
			
			add.w %5,%%d0			/* add gv.y to that */
			andi.w #31,%%d0
			lsl.w #5,%%d0
	
			move.w %%d7,%%d2
			subi #512,%%d2
			andi #2046,%%d2
			cmpi #1024,%%d2
			bcc xtexplus2
				neg.w %%d0
				addi.w #992,%%d0
		xtexplus2:


			adda.w %%d0,%2			/* and that's the texture offset */

			move.l %%d1,%%d0
			move.w %6,%%d1
			add.w %%d1,%%d1
			sub.w %%d7,%%d1
			andi.w #1022,%%d1
			move.w (%3,%%d1.w),%%d1
			mulu 2(%%a7),%%d1

			bra finishcalc
			

		yhitwall:
			move.w %%d7,%%d0			/* get the sin value... */
			subi.w #512,%%d0
			andi.w #1022,%%d0
			move.w (%3,%%d0.w),%%d0
			divu %%d0,%%d1				/* dy*cos(a)/sin(a) */

			move.w %%d7,%%d2
			subi.w #512,%%d2
			andi.w #2046,%%d2
			cmpi.w #1024,%%d2
			bcc ytexplus
				addq.w #1,%%d1
				neg.w %%d1
		ytexplus:

			add.w %4,%%d1
			andi.w #31,%%d1
			lsl.w #5,%%d1

			cmpi.w #1024,%%d7
			bcc ytexplus2
				neg.w %%d1
				addi.w #992,%%d1
		ytexplus2:	

			adda.w %%d1,%2

			move.w %6,%%d1
			add.w %%d1,%%d1
			sub.w %%d7,%%d1
			andi.w #1022,%%d1
			move.w (%3,%%d1.w),%%d1
			mulu (%%a7),%%d1

		finishcalc:
			lsl.l #6,%%d1
			divu %%d0,%%d1
			ext.l %%d1

			addq.w #4,%%a7
			movea.l (%%a7)+,%1

			/***********************************************************
			**	Draw Column.....
			**
			**	--------------------------------------------------------
			**
			**	j = 0;
			**	step = (0x100000)/(height);			/* tex width * height * sum fractional part */
			**	sum = 0;
			**
			**	if(height < 160)
			**	{
			**		while(j < 80-((height)>>1))
			**		{
			**			*bufferbits = 0;
			**			bufferbits += 160;
			**			j++;
			**		}
			**	}
			**	else
			**	{
			**		sum = step*((height-159)>>1);
			**	}
			**
			**	while((j < 80+((height+1)>>1)) && (j < 160))
			**	{
			**		*bufferbits = *(texbits+texoffset+(((sum)>>10)&~31));
			**		sum += step;
			**		bufferbits += 160;
			**		j++;
			**	}
			**
			**	while(j < 160)
			**	{
			**		/*
			**		u = (((UInt32)(i>80?i-80:80-i))<<4)/(UInt32)(j-80);
			**		v = (16*64)/(UInt32)(j-80);
			**
			**		dx = gv.x + ((v*(Int32)cos(gv.ang) - u*(Int32)sin(gv.ang))>>10);
			**		dy = gv.y + ((u*(Int32)cos(gv.ang) + v*(Int32)sin(gv.ang))>>10);
			**
			**		*bufferbits = *(texbits+(dx&31)+((dy&31)<<5));
			**		*/
			**		*bufferbits = 0;
			**		bufferbits += 160;
			**		j++;
			**	}
			************************************************************/

			/* at this point we have the step in d1 */
			moveq #0x4,%%d0
			swap %%d0
			divu %%d1,%%d0				/* d1 is the step value */
			lsr.w #1,%%d0
			lsl.l #4,%%d1
		
			move.w #160,%%d7
			cmp.w %%d7,%%d0
			bcc closewall
				move.w #159,%%d3
				sub.w %%d0,%%d3
				move.w %%d3,%%d5
				subq.w #1,%%d5
				asr.w #1,%%d5
				asr.w #1,%%d3
				move.b #55,%%d2			/* blue */
		ceilingloop:
					move.b %%d2,(%0)
					adda.w %%d7,%0
					dbra %%d3,ceilingloop
				clr.l %%d4				/* d4 is the vertical texture offset */
				move.w %%d0,%%d3
				subq.w #1,%%d3
				bra walldraw
		closewall:
			move.w %%d0,%%d4
			move.w #159,%%d3
			sub.w %%d3,%%d4
			lsr.w #1,%%d4
			mulu %%d1,%%d4

		walldraw:
			swap %%d4
			move.b (%2,%%d4.w),(%0)	
			swap %%d4
			add.l %%d1,%%d4
			adda.w %%d7,%0
			dbra %%d3,walldraw

			cmpi.w #159,%%d0
			bcc finish

			move.b #206,%%d2			/* green */
		floorloop:
			move.b %%d2,(%0)
			adda.w %%d7,%0
			dbra %%d5,floorloop

		finish:
			movea.l (%%a7)+,%2
			movea.l (%%a7)+,%0
			addq.w #1,%0
			addq.w #1,%%d6
			cmpi.w #160,%%d6
			bcs rayloop
	"
	: "+a" (bufferbits)
	: "a" (gv.map+(gv.x>>(5+8))+((gv.y>>(1+8))&~15)),
		"a" (texbits),
		"a" (costable),
		"m" ((UInt16)(gv.x>>8)),
		"m" ((UInt16)(gv.y>>8)),
		"m" ((UInt16)(gv.ang>>8)),
		"m" (angtable)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7");
}



void copybuffer_compat(void)
{
	RectangleType screenrect;

	RctSetRectangle(&screenrect, 0, 0, 160, 160);
	WinCopyRectangle(gfx.buffer, NULL,
		&screenrect, 0, 0, winPaint);
}


void copybuffer_asm(void)
{
	UInt8* bufferbits	= BmpGetBits(WinGetBitmap(gfx.buffer));
	UInt8* visbits		= BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));

	asm volatile("
		movem.l %%a2-%%a6,-(%%a7)
		move.l %4, %%d0

	colorcbloop:
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1			
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d7/%%a2-%%a6
		movem.l %%d1-%%d7/%%a2-%%a6, (%1)
		lea 48(%1),%1
		movem.l (%0)+, %%d1-%%d4
		movem.l %%d1-%%d4, (%1)
		lea 16(%1),%1
		dbra %%d0, colorcbloop

		movem.l (%%a7)+,%%a2-%%a6
	"
	: "=a" (bufferbits),
		"=a" (visbits)
	: "0" (bufferbits),
		"1" (visbits),
		"i" (159)
	: "%%d0", "%%d1", "%%d2", "%%d3", "%%d4", "%%d5", "%%d6", "%%d7",
		 "%%a2", "%%a3", "%%a4", "%%a5", "%%a6");
}
