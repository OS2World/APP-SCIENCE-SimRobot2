#define COLOR COLOR2
#define BOOLEAN BOOLEAN2
#define COORD COORD2
#ifdef __OS2__
#include <os2def.h>
#define INCL_WIN
#include <pmwin.h>
#define INCL_GPI
#include <pmgpi.h>
#include <pmmle.h>
#define INCL_DOSFILEMGR
#include <bsedos.h>
#include <owl\sound.h>
#define BEEP1 TSound::Beep(TSound::Exclamation)
#define BEEP2 TSound::Beep(TSound::Hand)
#define SYSCOLOR(s) TColor().GetSysColor(s)
#define PIXEL(dc,pt) GpiSetPel(dc,(POINTL*) &pt)
#define MOVE(dc,pt) GpiMove(dc,(POINTL*) &pt)
#define LINE(dc,pt) GpiLine(dc,(POINTL*) &pt)
#define POLY(dc,pt,n) \
  { GpiMove(dc,(POINTL*) pt); \
    POLYGON poly = {n-1,(POINTL*)&(pt[1])}; \
    GpiPolygons(dc,1,&poly,POLYGON_BOUNDARY,POLYGON_EXCL);}
#else
#define STRICT
#include <windows.h>
#define BEEP1 MessageBeep(MB_ICONEXCLAMATION)
#define BEEP2 MessageBeep(MB_ICONHAND)
#define SYSCOLOR(s) GetSysColor(s)
#define PIXEL(dc,pt) (dc).SetPixel(pt,TColor(0,0,0))
#define MOVE(dc,pt) (dc).MoveTo(pt)
#define LINE(dc,pt) (dc).LineTo(pt)
#define POLY(dc,pt,n) (dc).Polygon(pt,n)
#endif
#include <owl\menu.h>
#include <owl\docview.rc>
#include <owl\docmanag.h>
#include <owl\filedoc.h>
#include <owl\editview.h>
#include <owl\decmdifr.h>
#include <owl\controlb.h>
#include <owl\statusba.h>
#include <owl\buttonga.h>
#include <owl\textgadg.h>
#include <owl\scroller.h>
#include <owl\slider.h>
#ifndef _USERCLASS
#define _USERCLASS _OWLFASTTHIS
#endif
#undef COLOR
#undef BOOLEAN
#undef COORD
#ifdef GetObject
#undef GetObject
#endif
