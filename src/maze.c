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
#include "maze.h"
#include "main.h"

static void		dmove(UInt8* xptr, UInt8* yptr, UInt8 dir);
static Boolean	checkcarve(UInt8 x, UInt8 y, UInt8 dir);
static void		carve(UInt8* xptr, UInt8* yptr, UInt8 dir);
static Boolean	checkmark(UInt8 x, UInt8 y, UInt8 dir);
static void		deletebranch(UInt8 index);
static void		setgoals(UInt8 x, UInt8 y);
static void		drawmaptile(const UInt8 tilex, const UInt8 tiley);
static Int16	getheight(const UInt8 tilex, const UInt8 tiley);

void initmap(void)
{
	UInt8 i, j;

	gv.nbranches = 0;

	//the last columnn and row are unused -
	//the maze needs an odd number of spaces,
	//but using a power of 2 for the width/height
	//makes the calculations a little faster

	for(i = 0; i < MAPWIDTH-1; i++)
	{
		amc(i, 0) = tile_hard;
	}

	for(j = 1; j < MAPHEIGHT-2; j++)
	{
		amc(0, j) = tile_hard;

		for(i = 1; i < MAPWIDTH-2; i++)
		{
			amc(i, j) = tile_wall;
		}

		amc(MAPWIDTH-2, j) = tile_hard;
	}

	for(i = 0; i < MAPWIDTH-1; i++)
	{
		amc(i, MAPHEIGHT-2) = tile_hard;
	}

	amc(1, 1) = tile_floor;
	addbranch(1, 1, true);

	for(j = 0; j < MAPHEIGHT-1; j++)
	{
		for(i = 0; i < MAPWIDTH-1; i++)
		{
			drawmaptile(i, j);
		}
	}

	gv.progress = 0;
}


void drawmaptile(const UInt8 tilex, const UInt8 tiley)
{
	Int16	i, j;
	UInt16	offset;
	Int16	xbri, ybri;
	Int16	height		= getheight(tilex, tiley);
	Int16	dhx			= height - getheight(tilex+1, tiley);
	Int16	dhy			= height - getheight(tilex, tiley+1);
	UInt8*	bufferbits	= BmpGetBits(WinGetBitmap(gfx.buffer));

	offset = (20 + (tiley*8))*160 + (tilex*8) + 20;

	for(j = 0; j < 8; j++)
	{
		for(i = 0; i < 8; i++)
		{
			if(i == 7)
			{
				xbri = i + (tilex*8) + 20 - LIGHTX;

				if(((xbri < 0) && (dhx + xbri > 0))
				|| ((xbri > 0) && (dhx + xbri < 0)))
				{
					xbri = 32;
				}
				else
				{
					xbri = ((dhx + xbri)>>2) + 32;				
				}
			}
			else
			{
				xbri = ((i + (tilex*8) + 20 - LIGHTX)>>2) + 32;
			}
			
			if(j == 7)
			{
				ybri = j + (tiley*8) + 20 - LIGHTY;

				if(((ybri < 0) && (dhy + ybri > 0))
				|| ((ybri > 0) && (dhy + ybri < 0)))
				{
					ybri = 32;
				}
				else
				{
					ybri = ((dhy + ybri)>>2) + 32;
				}
			}
			else
			{
				ybri = ((j + (tiley*8) + 20 - LIGHTY)>>2) + 32;
			}

			if((xbri & ~63)
			|| (ybri & ~63))
			{
				bufferbits[offset++] = 0;
			}
			else
			{
				bufferbits[offset++] = gfx.lightmap[xbri + (ybri<<6)] + (height>>2);
			}
		}

		offset += (160-8);
	}
}


Int16 getheight(const UInt8 tilex, const UInt8 tiley)
{
	Int16 height;

	switch(amc(tilex, tiley))
	{
		case tile_hard:
			height = tile_hard_height;
			break;
		case tile_wall:
			height = tile_wall_height;
			break;
		default:
			height = tile_floor_height;
	}

	return height;
}



Boolean markcell(void)
{
	Boolean	finished = false;		//I haven't bothered to add an exit to the maze, so this is unused
	UInt8	x, y;
	UInt8	dir, dtries;

	if(gv.nbranches > 0)
	{
		x = gv.branchlist[gv.nbranches-1].x;
		y = gv.branchlist[gv.nbranches-1].y;

		dir = dir_right;
		dtries = 0;

		while((dtries < 4) && (!checkmark(x, y, dir)))
		{
			dir = (dir+1)&3;
			dtries++;
		}

		if(dtries < 4)
		{
			dmove(&x, &y, dir);
			amc(x, y) = tile_marked;
			gv.progress--;
			addbranch(x, y, true);
		}
		else
		{
			deletebranch(gv.nbranches-1);
		}

		setgoals(gv.branchlist[gv.nbranches-1].x, gv.branchlist[gv.nbranches-1].y);
	}

	return finished;
}


void setgoals(UInt8 x, UInt8 y)
{
	gv.goalx = (UInt32)(x*TILESIZE+TILESIZE/2)<<8;
	gv.goaly = (UInt32)(y*TILESIZE+TILESIZE/2)<<8;

	if(gv.goalx > gv.x)
	{
		gv.goalang = 0;
	}
	else if(gv.goalx < gv.x)
	{
		gv.goalang = (UInt32)Q2MAX<<8;
	}
	else if(gv.goaly > gv.y)
	{
		gv.goalang = (UInt32)Q1MAX<<8;
	}
	else
	{
		gv.goalang = (UInt32)Q3MAX<<8;
	}
}


Boolean checkmark(UInt8 x, UInt8 y, UInt8 dir)
{
	Boolean canmove = false;

	dmove(&x, &y, dir);
	if((amc(x, y) != tile_wall)
	&& (amc(x, y) != tile_hard)
	&& (amc(x, y) != tile_marked))
	{
		canmove = true;
	}

	return canmove;
}


Boolean carvecell(void)
{
	UInt8	x, y;
	UInt8	dir, dtries;

	if(gv.nbranches)
	{
		do
		{
			x = gv.branchlist[gv.nbranches-1].x;
			y = gv.branchlist[gv.nbranches-1].y;
	
			dir = SysRandom(0)&3;
			dtries = 0;

			while((dtries < 4) && (!checkcarve(x, y, dir)))
			{
				dir = (dir+1)&3;
				dtries++;
			}
	
			if(dtries < 4)
			{
				carve(&x, &y, dir);
				gv.progress += 2;
				addbranch(x, y, false);
			}
			else
			{
				deletebranch(gv.nbranches-1);
			}
		}while((dtries >= 4) && (gv.nbranches));
	}

	return gv.nbranches;
}


Boolean checkcarve(UInt8 x, UInt8 y, UInt8 dir)
{
	Boolean cancarve = false;

	dmove(&x, &y, dir);

	if(amc(x,y) != tile_hard)
	{
		dmove(&x, &y, dir);
	
		cancarve = ((amc(x, y) != tile_floor) && (amc(x, y) != tile_hard));
	}

	return cancarve;
}


void carve(UInt8* xptr, UInt8* yptr, UInt8 dir)
{
	dmove(xptr, yptr, dir);

	if(amc(*xptr, *yptr) != tile_hard)			//this check is redundant
	{
		amc(*xptr, *yptr) = tile_floor;
		drawmaptile(*xptr, *yptr);
		drawmaptile(*xptr-1, *yptr);
		drawmaptile(*xptr, *yptr-1);
		drawmaptile(*xptr-1, *yptr-1);

		dmove(xptr, yptr, dir);

		if(amc(*xptr, *yptr) != tile_hard)		//and so is this...
		{
			amc(*xptr, *yptr) = tile_floor;
			drawmaptile(*xptr, *yptr);
			drawmaptile(*xptr-1, *yptr);
			drawmaptile(*xptr, *yptr-1);
			drawmaptile(*xptr-1, *yptr-1);
		}
	}
}


void dmove(UInt8* xptr, UInt8* yptr, UInt8 dir)
{
	switch(dir)
	{
		case dir_right:
			*xptr += 1;
			break;
		case dir_up:
			*yptr -= 1;
			break;
		case dir_left:
			*xptr -= 1;
			break;
		case dir_down:
			*yptr += 1;
	}
}


void deletebranch(UInt8 index)
{
	while(index < gv.nbranches-1)
	{
		gv.branchlist[index] = gv.branchlist[index+1];
		index++;
	}

	gv.nbranches--;
}


void addbranch(const UInt8 x, const UInt8 y, const Boolean addtotop)
{
	if(gv.nbranches < MAXBRANCHES)
	{
		Int16 i;
		Int16 index;

		if((SysRandom(0)&7) || (addtotop) || (!gv.nbranches))
		{
			index = gv.nbranches;
		}
		else
		{
			index = SysRandom(0)%gv.nbranches;
		}

		for(i = gv.nbranches; i > index; i--)
		{
			gv.branchlist[i] = gv.branchlist[i-1];
		}

		gv.branchlist[index].x = x;
		gv.branchlist[index].y = y;

		gv.nbranches++;
	}
}