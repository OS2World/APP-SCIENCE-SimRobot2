/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  Common.h                                              ****
 **** Inhalt: Definition von verlaesslichen Datentypen              ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __COMMON_H_
#define __COMMON_H_

#include <stddef.h>

typedef unsigned char BOOLEAN; // Wahrheitswert
typedef int       INTEGER;  // 16 bit-Integer
typedef long int  LONGINT;  // 32 bit-Integer
typedef double    REAL;     // Fliesskommazahlen
typedef float     SHORTREAL; 

#ifdef __WIN16__
typedef SHORTREAL __huge* PSHORTREAL;
#ifdef _MSC_VER
#include <malloc.h>
#define ALLOC_BLOCKS(number,type) (type __huge *) _halloc(number,sizeof(type))
#define FREE_BLOCKS(p) _hfree((void __huge*) p)
#else
#include <malloc.h>
#define ALLOC_BLOCKS(number,type) (type __huge *) farmalloc((long) number * sizeof(type))
#define FREE_BLOCKS(p) farfree((void*) p)
#endif
#else
typedef SHORTREAL* PSHORTREAL;
#define ALLOC_BLOCKS(number,type) (type*) new char[number * sizeof(type)]
#define FREE_BLOCKS(p) delete [] p
#if defined(_MSC_VER) && defined(_DEBUG) && !defined(DEBUG_NEW)
void* __cdecl operator new(unsigned,const char*,int);
#define DEBUG_NEW new(__FILE__,__LINE__)
#define new DEBUG_NEW
#endif
#endif // __WIN16__

#define ALLOC_VALUE(size) ALLOC_BLOCKS(size,SHORTREAL)
#define FREE_VALUE(p) FREE_BLOCKS(p)

#ifndef TRUE
const TRUE  = 1;            // boolsche Konstanten
const FALSE = 0;
#endif // TRUE
const REAL
      EPSILON = 1.0e-12;    // Genauigkeitsschranke fuer Test auf 0.0

class SIM3DElement;         // Ein Teilelement der Simulation

typedef SIM3DElement* SIM3DOBJECT;

struct PORT {               // Ueber Ports werden Actoren gesteuert und Sensoren
  SIM3DOBJECT PortObject;   // gelesen
  INTEGER PortNr;

  PORT ()
         { PortObject = NULL; PortNr = 0; };
  PORT (SIM3DOBJECT obj, INTEGER nr)
         { PortObject = obj; PortNr = nr; };
};

typedef PORT ACTORPORT;
typedef PORT SENSORPORT;

#include "mystring.h"

#endif
