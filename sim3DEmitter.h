/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  sim3DEmitter                                          ****
 **** Inhalt: Beschreibt die Klassen SIM3DEmitter, SIM3DRadialEmitter **
 ****         und SIM3DSpotEmitter                                  ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DEMITTER_H_
#define _SIM3DEMITTER_H_

#include "sim3DBasics.h"
#include "SensorClasses.h"

const
  S3DEmitter = 30;

// SIM3DEmitter definiert eine abstrakte Klasse fuer Emitter aller Art nach
// dem Licht- (oder besser Sensorklassen-)modell von SimRobot. Die heraus-
// ragendste Eigenschaft von Emittern ist, dass sie fuer einen bestimmten Punkt
// innerhalb der Szene eine relative Lichtstaerke und eine relative Licht-
// richtung definieren. Gaengigerweise besitzt der Emitter einen Vektor, der
// die Lichtstaerken fuer die einzelnen Sensorklassen angibt und der mit der
// relativen Lichtstaerke multipliziert wird. Alle Emitter sind miteinander in
// einer Liste verkettet, um den Aufwand zu vermeiden, der beim Suchen aller
// Emitter entstuende.

class SIM3DEmitter : public SIM3DGlobal
{
public:
  SENSCLASSVECTOR* Radiation;   // Beschreibung der 'Farbe' des Emitters
  SIM3DEmitter* NextEmitter;    // Naechster Emitter in der Szene
  REAL Factor;                  // Allgemeiner Faktor, mit dem die Lichtstaerke
                                // multipliziert wird
  REAL Intensity;               // Zweiter Emitterfaktor, der ueber einen Aktor-
                                // port im Bereich [0;1] gesetzt werden kann. 

           SIM3DEmitter (const STRING&);
           SIM3DEmitter (const SIM3DEmitter&);
          ~SIM3DEmitter ();

  virtual STRING      GetClass ()
          { return "EMITTER"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DEmitter) || SIM3DGlobal::IsA (typ)); }
  virtual SIM3DElement* Copy() = 0;

  virtual APORTDEF*   GetAPortDef ();   // Beschreibung des Emitter-Aktorports
  virtual void        RegisterAdd ();   // Bei RegisterAdd Emitter in Liste 
                                        // eintragen
  virtual void        RegisterRemove (); // Und bei RegisterRemove entfernen

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP& backup);

// Hole 'Farb'-Vektor
  SENSCLASSVECTOR*    GetRadiation ();          
// Hole die Richtung der Lichtstrahlen an Punkt p
  virtual VECTOR      LightDirectionAt (const VECTOR& p);      
// Hole den Faktor, mit dem der 'Farb'-Vektor an Punkt p malgenommen werden muss
  virtual REAL        LightStrengthAt  (const VECTOR& p) = 0;

// Emitter haben eine eigene Standardfarbe
  virtual COLOR       GetColor ();
// und eine eigene Draw-Prozedur
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL) {};
};


const
  S3DRadialEmitter = 31;

// SIM3DRadialEmitter sind punktfoermige Lichtquellen, die gleichmaessig in
// alle Richtungen strahlen. Die Abnahme der Lichtstaerke ist umgekehrt 
// proportional zum Quadrat der Entfernung. Um einen Ueberlauf zu vermeiden,
// ist die Lichtstaerke innerhalb von Radius 1 konstant gleich dem angegebenen
// Faktor.

class SIM3DRadialEmitter : public SIM3DEmitter
{
public:
          SIM3DRadialEmitter (const STRING& name)
            : SIM3DEmitter (name) {};
  virtual STRING      GetClass ()
          { return "RADIALEMITTER"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DRadialEmitter) || SIM3DEmitter::IsA (typ)); }
  virtual SIM3DElement* Copy ()
          { return new SIM3DRadialEmitter (*this); }

  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
  virtual REAL        LightStrengthAt  (const VECTOR& p);
};

const
  S3DSpotEmitter = 32;

// SIM3DSpotEmitter ist ein Emitter, der in Richtung seiner z-Achse leuchtet.
// Es gelten die Regeln, die beim SIM3DRadialEmitter angegeben wurden, aber
// zusaetzliche nimmt die Lichtstaerke mit zunehmendem Winkel zur z-Achse des
// Emitters nach folgender Regel ab: Ist der Winkel groesser als 'Angle', ist
// der Faktor null. Ansonsten ist der Faktor 
// Faktor = cos ((Winkel / Angle) * PI/2) ^ Exponent

class SIM3DSpotEmitter : public SIM3DEmitter
{
public:
  REAL Angle;           // Streuwinkel der Lampe
  REAL Exponent;        // Exponent: Streugrad der Lampe (Je groesser der
                        // Exponent, desto geringer die Streuung)

           SIM3DSpotEmitter (const STRING&);

  virtual STRING      GetClass ()
          { return "SPOTEMITTER"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DSpotEmitter) || SIM3DEmitter::IsA (typ)); }
  virtual SIM3DElement* Copy()
          { return new SIM3DSpotEmitter (*this);}

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP& backup);

  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
  virtual REAL        LightStrengthAt  (const VECTOR& p);
};

#endif
