/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  Geometry.h                                            ****
 **** Inhalt: Komplexe Vektor-Operation (umgebende Rechtecke,       ****
 ****         Intervalle, Schnitte zwischen geometrischen Objekten) ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "Vector.h"

// INTERVALL definiert ein Intervall auf dem Zahlenstrahl das auch leer 
// sein kann. Ein Schnitt zwischen Intervallen ist moeglich.

class INTERVALL
{
public:
  BOOLEAN Empty;
  REAL Min, Max;

  INTERVALL ();                 // leeres Intervall
  INTERVALL (REAL,REAL);        // Intervall mit oberer und unterer Grenze
  void Intersect (const INTERVALL&);   // Schnitt mit angegebenem Intervall
};

class RAY;

// BOUNDINGBOX ist im Prinzip ein INTERVALL^3, das leer ist, wenn das Intervall
// in einer Dimension leer ist.
//
// Clear leert die Box.
// AddPoint erweitert die Box so, dass sie den Punkt enthaelt.
// AddBox erweitert die Box so, dass sie die andere Box enthaelt.
// DoesIntersect gibt an, ob sich zwei Kaesten schneiden.
// Intersect bildet die Schnittmenge mit dem angegebenen Kasten.

class BOUNDINGBOX
{
public:
  BOOLEAN Empty;
  VECTOR Min, Max;

  BOUNDINGBOX ();                       // Leere Menge
  void Clear ();
  void AddPoint (const VECTOR&);
  void AddBox (const BOUNDINGBOX&);
  BOOLEAN DoesIntersect (const BOUNDINGBOX&);
  void Intersect (const BOUNDINGBOX&);

  friend INTERVALL Intersection (const RAY&,const BOUNDINGBOX&);
};

// RAY realisiert Sehstrahlen, die mit zwei Punkten auf dem Strahl angegeben
// werden.

class RAY
{
public:
  VECTOR Start, Direction;

  RAY (const VECTOR& v1, const VECTOR& v2);
  BOOLEAN DoesIntersect (const BOUNDINGBOX&);

  friend INTERVALL Intersection (const RAY&,const BOUNDINGBOX&);
};

// Intersection bestimmt das Intervall, in dem ein Strahl einen Kasten 
// durchlaeuft. Der erste bei RAY angegebene Punkt erhaelt dabei den Wert 0, 
// der zweite Punkt den Wert 1. Ein Abschnitt a auf dem Strahl wird so
// mit p = v1 + a * (v2 - v1) als Punkt berechnet.

INTERVALL Intersection (const RAY&,const BOUNDINGBOX&);

#endif
