/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  BasicGraphics.h                                       ****
 **** Inhalt: Beschreibung der Graphik-Grundklasse und zugehoeriger ****
 ****         Klassen                                               ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __BASICGRAPHICS_H_
#define __BASICGRAPHICS_H_

#include "Common.h"
#include "Vector.h"
#include "sim3DFile.h"

// Folgende Fehler koennen bei der Graphikausgabe auftreten

enum GRAPHICERROR
{
  GRAPHIC_OK,
  GRAPHIC_OUT_OF_MEMORY,
  GRAPHIC_COLOR_ERROR,
  GRAPHIC_COMP_ERROR
};

// COLOR ist eine Farbnummer

typedef INTEGER COLOR;

// COLORENTRYs sind die internen Bestandteile einer COLORLIST

class COLORENTRY;

// COLORLIST ist eine Liste von Farbeintraegen, die von SIM3DWorld verwendet
// wird. Folgende Eintraege konnen gemacht werden: Ein Farbname (Der dem
// GRAPHICPORT bekannt sein muss), ein REAL-Tripel (RGB-Werte im Bereich [0;1) 
// oder beides (Wodurch der Name automatisch die angegebene Farbe erhaelt -
// wenn sie verfuegbar ist).

class COLORLIST
{
  COLORENTRY* First;
  COLORENTRY* Actual;
  INTEGER Count;

  COLOR FindEntry (COLORENTRY*);     // Finde die angegebene Farbdef. oder
                                     // trage sie ein

public:
  COLORLIST ();
  ~COLORLIST ();

  COLOR   GetColor (const STRING&);                   // Finde die Farbe mit Namen
  COLOR   GetColor (REAL,REAL,REAL);           // Finde Farbe mit RGB-Wert
  COLOR   AddColorDef (const STRING&,REAL,REAL,REAL); // Fuege Farbdef. ein

  INTEGER HowMany ();                // Wieviele Farben sind definiert
  void    FirstColor ();             // Setze internen Zeiger auf erste Farbe
  void    NextColor ();              // Setze internen Zeiger weiter
  void    NthColor (INTEGER);        // Setze internen Zeiger auf n-te Farbe
  BOOLEAN EndOfList ();              // Ende der Liste erreicht?
  BOOLEAN RGBDefined ();             // Ist Farbe durch RGB-Wert bestimmt?
  VECTOR  ActualRGB ();              // Hole RGB-Wert
  STRING  ActualName ();             // Hole Farb-Name

  COLOR   ReadColor (SCANNER&);               // Lese Farbe aus Datei
  void    WriteColor (BACKUP&,const STRING&,COLOR);  // Schreibe Farbe zu Datei
};

// GRAPHICPORT stellt einen virtuellen Port dar, auf den (dreidimensionale)
// Zeichenoperationen (Punkt, Linie, Flaeche) moeglich sind, und dient damit
// zur Ausgabe der Simulations-Szene auf beliebige Ausgabegeraete.
// Vor der ersten Zeichenoperation sollte NewPicture aufgerufen werden
// (um das Geraet falls noetig zu initialisieren), danach EndOfPicture
// (um die Zeichenoperation abzuschliessen).
// Vor dem ersten Zeichnen muessen die Farben mit RegisterColors angemeldet
// werden. (Eine Gewaehr, dass die Farben verwendet werden, ist nicht gegeben)
//
// DrawEdge wird benutzt, um die Kanten von Polygonen separat (und wenn die
// Polygone sich beruehren, nur einmal) auszugeben. Ist dies nicht noetig,
// wird die Ausgaben von Kanten unterdrueckt.

class GRAPHICPORT
{
  GRAPHICERROR LastError;
public:
  GRAPHICPORT ()                    { LastError = GRAPHIC_OK; };
  GRAPHICERROR GetLastError ()      { return LastError; };
  void SetError (GRAPHICERROR err)  { LastError = err; };

  virtual void RegisterColors (COLORLIST&) = 0;
  virtual void NewPicture   () = 0;
  virtual void DrawLine     (const VECTOR&, const VECTOR&, COLOR) = 0;
  virtual void DrawEdge     (const VECTOR&, const VECTOR&, COLOR) = 0;
  virtual void DrawPolygon  (INTEGER, VECTOR*, COLOR) = 0;
  virtual void EndOfPicture () = 0;
};

#endif
