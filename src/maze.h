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


#define tile_floor	0
#define tile_marked	1
#define tile_wall	16
#define tile_hard	17

#define tile_floor_height	0
#define tile_wall_height	40
#define tile_hard_height	40

#define dir_right	0
#define dir_down	1
#define dir_left	2
#define	dir_up		3

#define amc(x, y)	gv.map[(y)*MAPWIDTH+(x)]		//'access map coordinates'

extern void		initmap(void);
extern Boolean	carvecell(void);
extern Boolean	markcell(void);
extern void		addbranch(const UInt8 x, const UInt8 y, const Boolean addtotop);