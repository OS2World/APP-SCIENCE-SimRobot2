/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DReader.h                                         ****
 **** Inhalt: Funktion ReadObject zum Einlesen eines Objektes aus   ****
 ****         dem Scanner                                           ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DREADER_H_
#define _SIM3DREADER_H_

#include "sim3DFile.h"
#include "sim3DBasics.h"

// In diese Datei muessen alle definierten Simulationsobjekt-Klassen eingetragen
// werden, deren Objekte von einer Definitionsdatei eingelesen werden sollen.
// Genauere Erlaeuterungen in "sim3DReader.cc"

SIM3DElement* ReadObject (SCANNER& scan, SIM3DWorld* World);

#endif
