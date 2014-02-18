/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DPortDef.h                                        ****
 **** Inhalt: Definition von Strukturen zur Definition und          ****
 ****         Speicherung von Ports fuer die Simulation             ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DPORTDEF_H_
#define _SIM3DPORTDEF_H_

#include "Common.h"

// Die hier definierten Strukturen werden dazu benutzt, auf einfache Weise
// die Aktor- und Sensorports von Objekten zu definieren. Die Methoden
// GetAktorPortDef und GetSensorPortDef liefern Zeiger auf diese Strukturen,
// die von den Methoden aus "sim3DBasics.h", die die Portzugriffe erledigen, 
// ausgewertet werden.

// Funktionstypen zum Setzen und Lesen von Ports und zur Bestimmung der
// Groesse von Sensorports

typedef void    (*SetValueProc)   (SIM3DElement*, REAL);
typedef void    (*ReadValueProc)  (SIM3DElement*, PSHORTREAL);
typedef INTEGER (*GetDimSizeProc) (SIM3DElement*, INTEGER);

// Aktorportdefinition enthalten Informationen ueber den Namen eines Ports und
// wie das Aktorkommando zu setzen ist.

class APORTDEF
{
  friend class SIM3DElement;

  APORTDEF* Prev;               // Link fuer eine Liste von Portdef.
  char* Name;                   // Name des Aktorports
  SetValueProc SetValue;        // Wie wird das Aktorkommando gesetzt
public:
  APORTDEF (APORTDEF* liste, char* name, SetValueProc svp);     // Konstruktor
};

// Sensorportdefinitionen enthalten Informationen ueber den Namen eines Ports,
// die Dimension des Ergebnisses, wie die Groesse des Ergebnisses zu lesen ist
// und wie das Ergebnis selbst zu lesen ist.

class SPORTDEF
{
  friend class SIM3DElement;

  SPORTDEF* Prev;               // Link fuer eine Liste von Portdef.
  char* Name;                   // Name des Sensorports         
  INTEGER Dimension;            // Wieviele Dimensionen?
  GetDimSizeProc GetDimSize;    // Funktion zum Lesen der Groesse einer Dim.
  ReadValueProc ReadValue;      // Funktion zum Lesen des Sensorwerts
public:
  SPORTDEF (SPORTDEF* liste, char* name, ReadValueProc rvp);    // Konstruktor
                                                                // fuer dim = 0
  SPORTDEF (SPORTDEF* liste, char* name, INTEGER dim, GetDimSizeProc gdsp, 
        ReadValueProc rvp);     //Konstruktor fuer dim >= 1
};

#endif
