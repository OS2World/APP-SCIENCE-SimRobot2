/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  ZBuffer.h                                             ****
 **** Inhalt: Implementierung einer Klasse ZBUFFER zur alternativen ****
 ****         Kamerabild-Berechnung                                 ****
 ****                                                               ****
 ***********************************************************************/

#include "Vector.h"
#include "sim3DDynArray.h"
#include "SensorClasses.h"
#include <math.h>

#ifndef _SIM3DZBUFFER_H_
#define _SIM3DZBUFFER_H_

const MaxzBufVertices = 200;    // Maximalanzahl der inklusive Clipping fuer
                                // ein Polygon zulaessigen Eckenanzahl
class zBufElem;
typedef zBufElem 
#ifdef __WIN16__
__huge
#endif // __WIN16__
  * pzBufElem;                  // Zeiger auf Z-Buffer-Eintrag

// Die Klasse ZBUFFER realisiert das Tiefenpuffer-Verfahren zur Bildberechnung.
// Weil SimRobot prinzipiell beliebig viele Sensorklassen kennt, wird die
// Bildberechnung in zwei Schritte zerlegt: 
//   1. Eintragen von Objektadresse und Flaechennummer in den z-Buffer
//   2. Eintragen der Farbewerte in einen zu uebergebenen Puffer


class ZBUFFER
{
  REAL   VXBuffer [MaxzBufVertices];    // X-Koord. der Ecken
  REAL   VYBuffer [MaxzBufVertices];    // Y-Koord. der Ecken
  INTEGER NextVertex [MaxzBufVertices]; // jeweiliger Index der nachsten Ecke
  INTEGER VertexCount;                  // Anzahl der Ecken
  INTEGER PolygonStart;                 // Startindex des Kantenzuges
  VECTOR  PrevVertex;                   // vorige Ecke
  BOOLEAN PrevVisible;                  // War vorige Ecke sichtbar?

  VECTOR ViewPoint;                     // Geometrische Daten der Sichtpyramide
  VECTOR ViewPlaneCorner;
  VECTOR ViewPlaneHorizEdge;
  VECTOR ViewPlaneVertEdge;
  VECTOR ViewPlaneNormal;
  REAL   ViewPlaneDist;

  INTEGER Width;                        // Breite und Hoehe des Bildes
  INTEGER Height;
  LONGINT Size;                         // Pixelanzahl = Breite mal Hoehe
  pzBufElem Pixel;
public:

  ZBUFFER (INTEGER awidth, INTEGER aheight);    // Initialisierung
  ~ZBUFFER ();

// Festlegen der Sichtgeometrie
  void SetViewGeometry (const VECTOR& vpoint, const VECTOR& vpcorner,
                        const VECTOR& vphorizedge, const VECTOR& vpvertedge, REAL maxdepth);
// Starten eines neuen Kantenzuges
  void NewFaceVertex (const VECTOR& v);
// Fortfuehren eines Kantenzuges
  void AddFaceVertex (const VECTOR& v);
// Eintragen des bisher gebildeten Polygons in den z-Buffer
  void EnterThisFace (BOOLEAN texture, SIM3DElement* abody,
    INTEGER afacenr, const VECTOR& facefixpoint,
    const VECTOR& facehorizedge, const VECTOR& facevertedge);
// Berechnung des eigentlichen Kamerabildes
  void CalcCamera (const SENSCLASSVECTOR& Background, const INTEGERARRAY& classes, PSHORTREAL data,
                   REAL minDepth, REAL maxDepth);
// Auslesen der geringsten Tiefe im z-Buffer
  REAL GetMinDepth(void);

private:
// Eintragen eines geclippten Eckpunktes
  void AddVertex (const VECTOR& v);
};

#endif
