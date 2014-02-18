/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  HiddenGraph.h                                         ****
 **** Inhalt: HiddenLine-Graphikausgabe fuer die Simulation         ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _HIDDENGRAPH_H_
#define _HIDDENGRAPH_H_

#include "BasicGraphics.h"

class LINENODE;
class PLANENODE;

// COORD definiert Operationen auf zweidimensionale Vektoren, die zur
// Berechnung der Hiddenline-Graphik benoetigt werden.

class COORD
{
public:
  REAL x, y;
  COORD ()                   { x = 0.0; y = 0.0; };
  COORD (REAL xx, REAL yy)   { x = xx;  y = yy;  };
  COORD (const VECTOR& v)           { x = v.x; y = v.y; };
  ~COORD ()                  {};
  COORD operator * (REAL  r) { return COORD (x * r, y * r);     };
  COORD operator / (REAL  r) { return COORD (x / r, y / r);     };
  REAL  operator * (const COORD& c) { return x * c.x + y * c.y;        };
  COORD operator + (const COORD& c) { return COORD (x + c.x, y + c.y); };
  COORD operator - (const COORD& c) { return COORD (x - c.x, y - c.y); };
};

// HIDDENGRAPH definiert einen GRAPHICPORT, der eine virtuelle Graphikausgabe
// mit verdeckten Linien realisiert. Die Felder MM, Plane und Eye bestimmen
// den Standpunkt des Betrachters. Die Ausgabe laesst sich auf schnelle
// Wireframe-Darstellung umschalten. HIDDENGRAPH ist eine virtuelle Klasse,
// Die Methoden SetPixel und SetLine muessen in einer abgeleiteten Klasse
// definiert werden.
// ACHTUNG! Flaechen werden an der Bildebene nicht geclippt sondern einfach
//          ausgelassen und werden somit in diesem Fall durchsichtig.
//
// Wichtige Methoden:
//
// SetViewPoint  - Das erste Argument gibt eine 4*3 Matrix an, mit der das 
//                 Bild gedreht und verschoben wird.
//                 Die Szene wird dann von Richtung der positiven Z-Achse 
//                 betrachtet. Der zweite Wert gibt den Abstand der Bild-
//                 ebene vom Ursprung an, der dritte Wert den Abstand des
//                 Augpunktes.
// SetHidden     - Hiddenline-Darstellung an- oder ausschalten. Wirkt erst
//                 beim naechsten NewPicture
// SetPixel      - Virtuelle Zeichenroutine, noch undefiniert
// SetLine       - dito

class HIDDENGRAPH : public GRAPHICPORT
{
  friend class PLANENODE;
  friend class VISIARR;

  LINENODE* FirstLine;
  PLANENODE* FirstPlane;
  BOOLEAN Hidden, HiddenInMind;
  INTEGER MaxVertices;
  REAL ProjectSection (const VECTOR&, const VECTOR&, REAL);
protected:
  MOVEMATRIX MM;
  REAL Plane;
  REAL Eye;
  VECTOR Transform (const VECTOR&);
  COORD Projection (const VECTOR&);
public:
           HIDDENGRAPH ();
  virtual ~HIDDENGRAPH ();

          void SetViewPoint (const MOVEMATRIX& M, REAL p, REAL e);
  virtual void SetHidden    (BOOLEAN);
  virtual void NewPicture   ();
  virtual void DrawLine     (const VECTOR&, const VECTOR&, COLOR);
  virtual void DrawEdge     (const VECTOR&, const VECTOR&, COLOR);
  virtual void DrawPolygon  (INTEGER, VECTOR*, COLOR);
  virtual void EndOfPicture ();

  virtual void SetLine  (const COORD&, const COORD&, COLOR) = 0;
};

#endif
