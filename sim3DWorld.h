/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  sim3DWorld.h                                           ****
 **** Inhalt: Definition der zentralen Instanz, die alle Daten      ****
 ****         enthaelt                                              ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DWORLD_H_
#define _SIM3DWORLD_H_

#include "sim3DBasics.h"
#include "sim3DEmitter.h"

const
  S3DWorld = 3;

// SIM3DWorld ist die Klasse fuer die Wurzel des Szenen-Baums. Diese Wurzel
// ist ein SIM3DGroup, die neben allen Objekten der Simulation auch Daten
// enthaelt, die fuer alle Objekte der Simulation gelten. Dies sind Daten
// fuer das Raytracing/Licht-Modell der Simulation, aber auch Defaultein-
// stellungen fuer bestimmte Klassen, die global veraendert werden koennen
// (z.B. die Standardfarben fuer die Ausgabe auf einen Graphport),
// oder auch alle Oberflaechendefinitionen.

class SIM3DWorld : public SIM3DGroup
{
  SIM3DEmitter* LastEmitter;    // Zeiger auf die Ringliste aller Emitter
  SIM3DElement* LastMacro;      // Zeiger auf die Ringliste aller Makros
  SURFACEDESCR* LastSurface;    // Zeiger auf die Liste aller Oberflaechendef.
  TEXTUREDESCR* LastTexture;    // Zeiger auf die Liste aller Texturen
  SENSCLASSVECTOR scvTemp;      // Zwischenspeicher
  LONGINT Steps;

public:
// Daten fuer das Lichtmodell der Simulation:
  SENSCLASSVECTOR Ambient;              // Ambiente Strahlungsstaerke   
  SENSCLASSVECTOR Background;           // Lichtstaerke des Hintergrundes
  SENSCLASSVECTOR StandardRadiation;    // Standardfarbe von Lampen
  SURFACEDESCR    StandardSurface;      // Standardoberflaeche von Koerpern
  BOOLEAN         Shadows;              // Werden Schatten berechnet?
  BOOLEAN         QuickShading;         // Flaechen werden gleichmaessig 
                                        // beleuchtet
  BOOLEAN         QuickMoving;          // Bei Bewegungen werden keine Kollisionen
                                        // festgestellt
// Modi der Simulation:
  BOOLEAN         CalcSensorsImmediate; // Werden Sensoren sofort berechnet?
// Farbdaten fuer die Ausgabe auf Graphicports:
  COLORLIST       ColorList;            // Liste aller benutzten Farben
  COLOR           StandardColor;        // Standardfarbe fuer Objekte
  COLOR           BodyColor;            // Standardfarbe fuer Koerper
  COLOR           EmitterColor;         // Standardfarbe fuer Emitter
  COLOR           SensorColor;          // Standardfarbe fuer Sensoren
  COLOR           WhiskerColor;         // Standardfarbe fuer Tasthaare

           SIM3DWorld (const STRING&);
           SIM3DWorld (const SIM3DWorld&);
  virtual ~SIM3DWorld ();


  virtual STRING      GetClass ()
          {return "WORLD"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DWorld) || SIM3DGroup::IsA (typ); }
  virtual SIM3DElement* Copy()
          {return new SIM3DWorld (*this);}

// Wieviele Zeitschritte wurden schon ausgefuehrt?
  virtual LONGINT    GetSteps ();

// Jeweils ein Attribut lesen
  void ReadAttr (const STRING& attr, SCANNER& scan);
// Alle Attribute schreiben
  void WriteAttrs (BACKUP& backup);

// Nimm einen Emitter in die Emitterliste auf
  void AddEmitter    (SIM3DEmitter* Obj);
// Entferne einen Emitter aus der Emitterliste
  void RemoveEmitter (SIM3DEmitter* Obj);

// Nimm eine Oberflaeche in die Oberflaechenliste auf
  void AddSurface (SURFACEDESCR* surface);
// Liefere die Oberflaeche mit dem angegebenen Namen
  SURFACEDESCR* FindSurface (const STRING& name);

// Nimm eine Textur in die Texturenliste auf
  void AddTexture (TEXTUREDESCR* surface);
// Liefere die Textur mit dem angegebenen Namen
  TEXTUREDESCR* FindTexture (const STRING& name);

// Nimm ein Objekt als Makro in die Makroliste auf
  void AddMacro (SIM3DElement* obj);
// Liefere das Makroobjekt mit dem angegebenen Namen
  SIM3DElement* FindMacro (const STRING& name);
// Wieviele Makros sind definiert
  INTEGER HowManyMacros ();
// Liefert das n-te definierte Makro
  SIM3DElement* NthMacro (INTEGER n);

// Fuehre einen Zeitschritt fuer die gesamte Simulation durch (inklusive
// Sensorberechnung, die aber bis verzoegert werden kann, bis die Sensoren
// gelesen werden).
  void DoTimeStep ();

  virtual void RegisterBoundingBox ();

private:
// Befindet sich ein Koerper zwischen den beiden Punkten (und werden ueberhaupt
// Schatten berechnet)?
  BOOLEAN ShadowCast (const VECTOR& p1, const VECTOR& p2);
public:
  virtual BOOLEAN DoesIntersect (SIM3DElement* obj);
// Berechne die Lichtverhaeltnisse am Punkt 'p' mit dem Normalenvektor 'normal'
  SENSCLASSVECTOR& LightAt (const VECTOR& p, const VECTOR& normal);
// Berechne den Wert den der Sehstrahl 'ray' in allen Sensorklasse
// in der Simulation sieht.
  SENSCLASSVECTOR& CalcCameraRay (const RAY& ray,REAL& Depth);
// Berechne ein Kamerabild ueber z-Buffer
  void CalcCameraView (const MOVEMATRIX& pos, REAL deltax, REAL deltaY,
       const INTEGERARRAY& classes, INTEGER width, INTEGER height,
       REAL minDepth, REAL maxDepth, PSHORTREAL buffer);
// Berechne den Abstandswert, den das Tasthaar 'ray' ertastet.
  REAL CalcWhiskerRay (const RAY& ray);
// Berechne den Wert, den der taktile Sensor 'ray' in allen Sensorklassen
// 'erschmeckt'
  SENSCLASSVECTOR& CalcTactileRay (const RAY& ray);
// Berechne den Wert, den der "Lichtrezeptor", dessen Ort und Richtung von 'ray'
// bestimmt ist, in den Sensorklassen feststellt.
  SENSCLASSVECTOR& CalcReceptorRay (const RAY& ray);
// Berechne die Entfernung zum naechsten Objekt in einer bestimmten Richtung
  REAL CalcUltraSonicRay(const RAY& ray);
// Berechne die Entfernung zum naechsten Objekt innerhalb einer Pyramide
  REAL CalcUltraSonic(const MOVEMATRIX &pos,REAL deltax,REAL deltay,
                      INTEGER width,INTEGER height);
};

#endif
