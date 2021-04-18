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


#define ROMVERSION2		sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0)
#define MINROMVERSION	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)
#define CREATOR			'MZHK'
#define FPS				50			//frame rate cap
#define error_displaydepth	-6001

typedef struct
{
	UInt16		flags;
	void		(*func_copybuffer)(void);
}type_deviceinfo;

typedef struct
{
	Int32 interval;			//minimum ticks per frame
	Int32 previous;			//time (in ticks) of the last update
	Int32 delta;			//ms between the last update and the current
	Int32 wait;				//ticks to wait before updating, e.g. next-now
	Int32 next;				//time (ideally, in ticks) of the next update 
}type_time;

#define df_68k		1
#define df_lores	2

extern type_deviceinfo	device;
extern UInt16			globalerr;
extern type_time		time;