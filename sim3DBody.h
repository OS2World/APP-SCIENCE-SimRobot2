/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  sim3DBody.h                                           ****
 **** Inhalt: Die Klassen SIM3DBody (feste Koerper) und             ****
 ****         SIM3DPolyeder (polygonale Koerper) werden hier        ****
 ****         beschrieben.                                          ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __SIM3DBODY_H_
#define __SIM3DBODY_H_

#include "Vector.h"
#include "BasicGraphics.h"
#include "sim3DDynArray.h"
#include "SensorClasses.h"
#include "sim3DBasics.h"

const
  S3DBody = 10;

// SIM3DBody ist die (abstrakte) Klasse fuer alle Koerper.
// Koerper haben eine Oberflaeche mit Oberflaecheneigenschaften.
// Welche Methoden fuer eine reale Klasse neu definiert werden muessen,
// ist der Klasse SIM3DPolyeder zu entnehmen.

class SIM3DBody : public SIM3DElement
{
protected:
  SURFACEDESCR* Material;            //Oberflaechenbeschreibung

public:
// Standardmethoden
           SIM3DBody (const STRING&);
           SIM3DBody (const SIM3DBody&);
  virtual STRING      GetClass ()
          {return "BODY"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DBody) || SIM3DElement::IsA (typ); }
  virtual SIM3DElement* Copy() = 0;

// zusaetzliche Attribute
  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP&);

// Koerper werden in einer eigenen Standardfarbe angezeigt
  virtual COLOR       GetColor ();

// Lesen der Oberflaecheneigenschaften
  virtual SURFACEDESCR* GetSurface (INTEGER n);
};


//- FACEARRAY -------------------------------------------------------------
//
// Ein FACEARRAY ist ein DYNARRAY (siehe "sim3DDynArray") von FACESTRUCT.

class FACESTRUCT;

class FACEARRAY : public DYNARRAY
{
  FACESTRUCT* Data;
public:
            FACEARRAY ();
            FACEARRAY (const FACEARRAY&);
  virtual  ~FACEARRAY ();
  virtual  void Resize (INTEGER by);
  void     AddElement (const FACESTRUCT& f);
  FACESTRUCT& operator [] (INTEGER);
};

//- SIM3DPolyeder -----------------------------------------------------------

const
  S3DPolyeder = 11;

// SIM3DPolyeder definiert Polyeder.

class SIM3DPolyeder : public SIM3DBody
{
protected:
  VECTORARRAY  BasicPoints,     // Alle Eckpunkte des Polyeders relativ zum
                                // Bezugssystem  
               NewPoints,       // Alle Eckpunkte des Polyeders absolut
               OldPoints;       // Zwischenspeicher (fuer MakeStep und TakeBack)
  VECTORARRAY  BasicFaceDir,    // Normalenvektoren der Flaechen (relativ zum
                                // Bezugssystem
               NewFaceDir,      // Normalenvektoren absolut
               OldFaceDir;      // Zwischenspeicher (fuer MakeStep und TakeBack)
  INTEGERARRAY Edges;           // Die Indizes der Eckpunkte aller Kanten.
                                // Edges enthaelt paarweise Indizes in das
                                // Array der Eckpunkte des Koerpers. Keine 
                                // Kante ist doppelt enthalten.
  INTEGERARRAY Vertices;        // Die Indizes der Eckpunkte aller Flaechen.
                                // Vertices enthaelt die Indizes der Eckpunkte 
                                // aller Begrenzungsflaechen. Kantenzuege werden
                                // durch einen negativen Index abgeschlossen.
                                // (Der Index ist deshalb immer um 1 erhoeht)
  FACEARRAY Faces;              // Indexliste aller Flaechen (Indizes in
                                // Vertices)
  INTEGER PointCount;           // Anzahl der Eckpunkte des Polyeders
  BOOLEAN Closed;               // Definition des Polyeders ist abgeschlossen.
  SENSCLASSVECTOR scvTemp;      // Zwischenspeicher
public:
  BOOLEAN  IsTransparent;

           SIM3DPolyeder (const STRING&);
           SIM3DPolyeder (const SIM3DPolyeder&);
  virtual STRING      GetClass ()
          {return "POLYEDER"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DPolyeder) || SIM3DBody::IsA (typ); }
  virtual SIM3DElement* Copy()
          { return new SIM3DPolyeder (*this); }

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP&);

// Bei einem RegisterAdd muessen die absoluten Eckpunkte (NewPoints)
// berechnet werden
  virtual void        RegisterAdd ();
// Dazu wird NewPosition benutzt
  virtual void        NewPosition (const MOVEMATRIX& m);
// MakeTimeStep hat keine Auswirkungen, die absolute Lage der Eckpunkte wurden
// schon von MakeStep korrekt gesetzt.
  virtual void        MakeTimeStep (const MOVEMATRIX& m) {};
// MakeStep berechnet die Eckpunkte neu und speichert die alten Werte in
// 'OldPoints'
  virtual void        MakeStep (const MOVEMATRIX& m);
// StepBack setzt 'NewPoints' = 'OldPoints'
  virtual void        StepBack ();
// Test, ob ein Objekt diesen Polyeder schneidet.
  virtual BOOLEAN     DoesIntersect (SIM3DElement* obj);
// Berechne das neue umgebende Rechteck
  virtual void        MakeNewBoundingBox ();
// Zeichne den Polyeder
  virtual void        Draw (GRAPHICPORT* gp, const MOVEMATRIX& m, INTEGER dl, REAL sc);
// Berechne den Schnitt eines Strahls mit dem Polyeder
  virtual BOOLEAN     CalcRayIntersection (const RAY& ray, BOOLEAN infinite, 
                          REAL& where, SIM3DElement*& obj, INTEGER& facenr);
// Zeichne den Koerper in einen z-Buffer (fuer Kamerabild-Berechnung)
  virtual void        DrawBodyOnZBuffer (ZBUFFER&);

// Prozeduren zum Definieren des Polyeders:
// Ihr Aufruf ist nur zulaessig, solange LastFace noch nicht aufgerufen und das
// Objekt somit noch nicht 'Closed' ist.
// Fuege einen neuen Eckpunkt ein
  void        AddPoint (const VECTOR& p);
// Fuege einen Index auf die Ecke einer Flaeche ein
  void        AddFaceEdge (INTEGER edge);
// Beginne eine neue Flaeche
  void        NewFace (const FILEPOS& pos);
// Beende die Definition des Polyeders
  void        LastFace (const FILEPOS& pos);
// Bilde einen Sweep-Koerper aus den definierten Eckpunkten. Es duerfen noch
// keine Flaechen definiert sein.
  void        Sweep (const VECTOR& delta,const FILEPOS& pos);
// Bilde einen Rotations-Koerper aus den definierten Eckpunkten. Es duerfen noch
// keine Flaechen definiert sein.
  void        Rotate (INTEGER turns, BOOLEAN torus,const FILEPOS& pos);

// Liefert die Farbe der angegebenen Stelle an Punkt p
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, REAL TextureX, REAL TextureY);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p, const VECTOR& v);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p, const VECTOR& v,
                                             REAL TextureX, REAL TextureY);
  SENSCLASSVECTOR& GetColorOfFaceAt2 (INTEGER n, const VECTOR& p, const VECTOR& v,
                                      REAL TextureX, REAL TextureY);
// Lesen der Oberflaecheneigenschaften
  virtual   SURFACEDESCR* GetSurface (INTEGER n);
// Bestimme die Flaechenkoordinaten der Flaeche n an Punkt p
  virtual     BOOLEAN     GetPlaneCoord (INTEGER n, const VECTOR& p, REAL& x, REAL& y);
 
private:
// Test auf korrekte Flaechennummer
  void        CheckFaceNr (INTEGER facenr);
// Liefert den n-ten Eckpunkt von Flaeche facenr (original)
  VECTOR      GetFaceVertex (INTEGER facenr, INTEGER n);
// Liefert den Normalenvektor der angegebenen Flaeche (transformiert)
  VECTOR      NormalVectorOfFace (INTEGER facenr);
// Liefert die Eckpunkte der angegebenen Kante (Indizes aus Vertices, Punkte aus// NewPoints)
  void        GetEdge (INTEGER edgenr, VECTOR& p1, VECTOR& p2);
// Liefert die Eckpunkte der angegebenen Kante, aber keine Kante ist doppelt
// angegeben (Indizes aus Edges, Punkte aus NewPoints)
  void        GetEdgeOnce (INTEGER edgenr, VECTOR& p1, VECTOR& p2);
// Trage eine Kante in 'Edges' ein. Doppelte werden gestrichen.
  void        AddEdgeOnce (INTEGER e1, INTEGER e2);
// Bestimmung der Flaechenausdehnung und des Aufhaengepunktes
  void RecalcFaceGeometry (INTEGER facenr);
// Festlegung der Orientierung einer Flaeche
  void SetFaceOrientation (INTEGER facenr, REAL winkela, REAL winkelb);

// Liegt der angegebene Punkt innerhalb der bezeichneten Flaeche (Es wird 
// angenommen, dass der Punkt innerhalb der Ebene der Flaeche liegt).
  BOOLEAN     DoesPointLieInFaceNr (const VECTOR& p, INTEGER facenr);
// Schneidet der angegebene Strahl die Flaeche und wo schneidet er sie.
  void        WhereDoesRayCutPlaneNr (const RAY& ray, INTEGER planenr, BOOLEAN& does,
                  REAL& where);

// Liefert das umgebende Rechteck der Flaeche
  BOUNDINGBOX GetBoundingBoxOfFace (INTEGER facenr);
// Wo schneidet der angegebene Strahl den Polyeder zum ersten Mal und wie sieht
// der Normalenvektor der geschnittenen Flaeche aus
  void        FindFirstCutWith (const RAY& ray, BOOLEAN& exist, REAL& where, 
                  INTEGER& facenr);

// Gibt es eine Ueberschneidung mit dem angegebenen Polyeder
  BOOLEAN     IntersectionFound (SIM3DPolyeder* poly);

};

#endif
