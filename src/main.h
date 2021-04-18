/****************************************************************************
** Maze Hiker
** Copyright 2003 Aaron Curtis
** 
** This file is part of Maze Hiker.
** 
** Maze Hiker is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** Maze Hiker is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with Maze Hiker; if not, see <https://www.gnu.org/licenses/>.
****************************************************************************/


#define MAPWIDTH		16
#define MAPHEIGHT		16
#define MAPSIZE			(MAPWIDTH*MAPHEIGHT)
#define MAXBRANCHES		MAPSIZE

#define TILESIZE		32
#define Q1MAX			256
#define Q2MAX			(Q1MAX*2)
#define Q3MAX			(Q1MAX*3)
#define Q4MAX			(Q1MAX*4)

#define state_none		0
#define state_init		1
#define state_create	2
#define state_fade		3
#define state_walk		4

#define LIGHTX			80
#define LIGHTY			80

#define WALKSPEED		10				//256ths of a distance unit / ms; 32 du = 1 map tile
#define ROTSPEED		(WALKSPEED*8)	//256ths of an angle unit / ms; 256 au = 90 degrees
#define FADESPEED		50				//256ths of a color index / ms
#define CREATESPEED		12				//1000ths of maze cell / ms

typedef struct
{
	UInt8 x;
	UInt8 y;
}mapcoord;

typedef struct
{
	UInt32		counter;
	UInt16		state;
	UInt16		waitcreate;
	UInt16		progress;
	UInt32		x;
	UInt32		y;
	Int32		ang;
	UInt32		goalx;
	UInt32		goaly;
	Int32		goalang;
	mapcoord	branchlist[MAXBRANCHES];
	UInt8		map[MAPSIZE];
	UInt8		nbranches;
}globalvars;

typedef struct
{
	WinHandle	buffer;
	WinHandle	textures;
	UInt8*		lightmap;
}type_gfx;



extern void doapp(void);
extern void	closebumpmap(void);

extern void copybuffer_asm(void);
extern void copybuffer_compat(void);

extern UInt16*		costable;
extern Int8*		angtable;
extern type_gfx		gfx;
extern globalvars	gv;