/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  Vector.h                                              ****
 **** Inhalt: Beschreibungen der Klassen VECTOR, MATRIX, MOVEMATRIX ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __VECTOR_H_
#define __VECTOR_H_

#include "Common.h"

const REAL 
  PI = 3.1415926535897932384626;

// VECTOR beschreibt einen Punkt oder eine Verschiebung im 3D-Raum

class VECTOR
{
public:
  REAL x,y,z;
  VECTOR ();
  ~VECTOR () {};
  VECTOR (REAL, REAL, REAL);
  BOOLEAN Norm ();
  BOOLEAN operator == (const VECTOR& v);
  BOOLEAN operator != (const VECTOR& v) {return !(*this == v);}
};

// MATRIX beschreibt als 3*3 Matrix eine Transformation im 3D-Raum

class MATRIX
{
public:
  VECTOR e1, e2, e3;
    //Anmerkung: Die Vektoren bilden die *Spalten* der Matrix!
  MATRIX ();
  MATRIX (const VECTOR&, const VECTOR&, const VECTOR&);
};

// MOVEMATRIX beschreibt eine Transformation mit anschliessender Translation 

class MOVEMATRIX
{
public:
  MATRIX Matrix;
  VECTOR Offset;
  MOVEMATRIX ();
  MOVEMATRIX (const MATRIX&, const VECTOR&);
};


// allgemeine Operationen, wie sie aus der Vektorarithmetik und Mathematik
// bekannt sind

REAL       abs (const VECTOR&);
REAL       abs (REAL r);
VECTOR     operator + (const VECTOR&, const VECTOR&);
VECTOR     operator - (const VECTOR&);
VECTOR     operator - (const VECTOR&, const VECTOR&);
VECTOR     operator * (REAL, const VECTOR&);
VECTOR     operator * (const VECTOR&, REAL);
VECTOR     operator / (const VECTOR&, REAL);
REAL       operator * (const VECTOR&, const VECTOR&);
REAL       CosAngle (const VECTOR&, const VECTOR&);
VECTOR     CrossProduct (const VECTOR&, const VECTOR&);  // Kreuzprodukt zweier Vektoren
REAL       det (const VECTOR&,const VECTOR&,const VECTOR&);     // Determinante einer Matrix

VECTOR     operator * (const MATRIX&, const VECTOR&);
MATRIX     operator * (const MATRIX&, const MATRIX&);

VECTOR     operator * (const MOVEMATRIX&, const VECTOR&);
MOVEMATRIX operator * (const MOVEMATRIX&, const MOVEMATRIX&);

// Liefert eine Matrix, die einen Punkt um den angegebenen Winkel um die Z-Achse
// dreht:

MATRIX     TurnXYMatrix (REAL);

// Drehung um Y-Achse

MATRIX     TurnXZMatrix (REAL);

// Drehung um Z-Achse

MATRIX     TurnYZMatrix (REAL);

// Bei einer Matrix, die nur eine beliebige Drehung um den Koordinatenursprung
// ausfuehrt, wird die Drehung in die Drehung um die X-Achse, Y-Achse und
// Z-Achse aufgeschluesselt (In dieser Reihenfolge). Der zurueckgegebene
// boolsche Wert gibt an, ob die Operation durchgefuehrt werden konnte.
// Fuer Matrizen, deren Spaltenvektoren nicht senkrecht aufeinander stehen,
// ist das Ergebnis nicht sinnvoll 

BOOLEAN    ExtractAngles (const MATRIX&,REAL&,REAL&,REAL&);

#endif
