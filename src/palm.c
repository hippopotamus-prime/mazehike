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


#include <PalmOS.h>
#ifdef SONY_CLIE
#include <sony/SonyCLIE.h>
#endif
#include "palm.h"
#include "main.h"

static void		eventhandler(void);
static UInt16	startapp(void);
static void		stopapp(void);
static UInt16	initdevice(void);
static void		setgfxfunctions(void);
static Boolean	preprocess(EventPtr eventptr);
static Int32	timeleft(void);

type_deviceinfo	device;
UInt16			globalerr;
type_time		time;

UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	UInt32 err = 0;

	if(cmd == sysAppLaunchCmdNormalLaunch)
	{
		err = startapp();
		if(!err)
		{
			eventhandler();
		}
		stopapp();
	}

	return err;
}


UInt16 startapp(void)
{
	UInt16 err;
	MemHandle mhand;
	WinHandle oldwin = WinGetDrawWindow();

	err = initdevice();
	if(err) return err;

	setgfxfunctions();

	time.interval = SysTicksPerSecond()/FPS;

	mhand = DmGetResource('BLAH', 0);
	costable = MemHandleLock(mhand);

	mhand = DmGetResource('BLAH', 1);
	angtable = MemHandleLock(mhand);

	gfx.textures = WinCreateOffscreenWindow(32, 32, screenFormat, &err);
	if(err) return err;
	WinSetDrawWindow(gfx.textures);
	mhand = DmGetResource(bitmapRsc, 0);
	WinDrawBitmap(MemHandleLock(mhand), 0, 0);
	MemHandleUnlock(mhand);
	DmReleaseResource(mhand);
	WinSetDrawWindow(oldwin);

	gfx.buffer = WinCreateOffscreenWindow(160, 160, screenFormat, &err);
	if(err) return err;

	gv.state = state_init;

	return 0;
}


void stopapp(void)
{
	if(costable)
	{
		MemPtrUnlock(costable);
		DmReleaseResource(MemPtrRecoverHandle(costable));
	}

	if(angtable)
	{
		MemPtrUnlock(angtable);
		DmReleaseResource(MemPtrRecoverHandle(angtable));
	}

	if(gfx.buffer)
	{
		WinDeleteWindow(gfx.buffer, false);
	}
	if(gfx.textures)
	{
		WinDeleteWindow(gfx.textures, false);
	}

	if((gv.state == state_init) || (gv.state == state_create)
	|| (gv.state == state_fade))
	{
		closebumpmap();
	}
}


void eventhandler(void)
{
	EventType event;

	do
	{
		EvtGetEvent(&event, timeleft());

		if(!preprocess(&event))
		{
			if(!SysHandleEvent(&event))
			{
				if(!MenuHandleEvent(NULL, &event, &globalerr))
				{
					FrmDispatchEvent(&event);
				}
			}
		}
	}while((event.eType != appStopEvent) && (!globalerr));
}


Boolean preprocess(EventPtr eventptr)
{
	Boolean handled = false;

	if(!time.wait)
	{
		doapp();
	}

	return handled;
}


UInt16 initdevice(void)
{
#ifdef SONY_CLIE
	UInt16 refnum;
#endif
	UInt32 featureval;
	UInt32 width;
	UInt32 height;
	UInt32 depth;

	//check rom version
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &featureval);
	if(featureval < MINROMVERSION)
	{
		if(featureval < ROMVERSION2)
		{
			//apparently rom 1.0 was pretty lame...
			AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
		}
		return sysErrRomIncompatible;
	}


	//check and set screen depth
	WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL);

	if(depth & 128)
	{
		depth = 8;
	}
	else
	{
		return error_displaydepth;
	}

	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);


	//check display resolution
	if((!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &featureval))
	&& (featureval >= 4))
	{
		//Palm hi-res API is present
		WinScreenGetAttribute(winScreenWidth, &width);
		WinScreenGetAttribute(winScreenHeight, &height);
	}
#ifdef SONY_CLIE
	else if((!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, &featureval))
	&& (((SonySysFtrSysInfoP)featureval)->libr & sonySysFtrSysInfoLibrHR)
	&& ((!SysLibFind(sonySysLibNameHR, &refnum)) || (!SysLibLoad('libr', sonySysFileCHRLib, &refnum)))
	&& (!HROpen(refnum)))
	{
		//Sony's hi-res library

		//this will disable hi-res assist
		width = 160;
		height = 160;
		HRWinScreenMode(refnum, winScreenModeSet, &width, &height, NULL, NULL);
		//check to see if it worked...
		HRWinScreenMode(refnum, winScreenModeGet, &width, &height, NULL, NULL);

		HRClose(refnum);
	}
#endif
	else
	{
		WinScreenMode(winScreenModeGetDefaults, &width, &height, NULL, NULL);
	}

	if((width == 160) && (height >= 160))
	{
		device.flags |= df_lores;
	}


	//check processor
	FtrGet(sysFtrCreator, sysFtrNumProcessorID, &featureval);
	if((featureval == sysFtrNumProcessor328)
	|| (featureval == sysFtrNumProcessorEZ)
	|| (featureval == sysFtrNumProcessorVZ)
	|| (featureval == sysFtrNumProcessorSuperVZ))
	{
		device.flags |= df_68k;
	}

	return 0;
}


Int32 timeleft(void)
{
	Int32 now = TimGetTicks();

	time.wait = time.next - now;

	if(time.wait <= 0)
	{
		time.delta = (now - time.previous)*1000/SysTicksPerSecond();
		time.previous = now;
		time.wait = 0;
		time.next = now + time.interval;
	}

	return time.wait;
}


void setgfxfunctions(void)
{
	if((device.flags & df_68k)
	&& (device.flags & df_lores))
	{
		device.func_copybuffer = copybuffer_asm;
	}
	else
	{
		device.func_copybuffer = copybuffer_compat;
	}
}