/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DSensors.h                                        ****
 **** Inhalt: Implementation der Sensor-Elemente der Simulation     ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DSENSORS_H_
#define _SIM3DSENSORS_H_

#include "sim3DBasics.h"

// Es gibt eigentlich zwei Sorten von Sensoren: Einzelwertsensoren die einen
// einzelnen Real als Ergebnis haben (SIM3DSingleSensor) wie z.B. Tasthaare
// oder Rezeptoren, und zweidimensionale Sensoren (SIM3DMultiSensor). Von 
// dieser Sorte gibt es nur Kameras und Tasthaarfelder (die dann sozusagen
// eine Reliefkarte liefern).

const
  S3DSensor = 40;

class SIM3DSensor : public SIM3DGlobal
{
  BOOLEAN IsComputed;           // Gibt an, ob der Wert des Sensors schon
                                // berechnet wurde.
protected:
  static REAL ErrorFunc[201];   // Fehlerfunktion als Tabelle fuer NOISE
  BOOLEAN ClassesDefined;       // Wurde eine Klasse angegeben?
  BOOLEAN KnowClasses;          // Soll das Array verwendet werden?
  LONGINT Size;                 // Groesse des Vektors
  BOOLEAN KnowLength;           // Wird die Laenge verwendet?
  REAL Noise;                   // Rauschen auf den Sensorsignalen
public:
  INTEGERARRAY Classes;         // Sensorklassen-Array
  PSHORTREAL Value;             // Ergebnis-Vektor
  REAL minDepth,
       maxDepth;                // Minimale und maximale Entfernung
  REAL Length;                  // Laenge des Sensors
public:
          SIM3DSensor (const STRING&);
          SIM3DSensor (const SIM3DSensor&);
          ~SIM3DSensor ();

  virtual STRING      GetClass ()
          {return "SENSOR"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DSensor) || SIM3DGlobal::IsA (typ);}

  virtual SPORTDEF*   GetSPortDef ();

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP&);

// Bestimme die Groesse in einer Dimension
  virtual INTEGER     DimSize (INTEGER i);
          void        ResizeBuffer ();

// Setzt 'IsComputed' auf FALSE und berechnet den Sensor evtl. neu (wenn
// World->CalcSensorsImmediate == TRUE ist)
  virtual void        ComputeSensors ();
// Berechnet den Sensorwert, wenn 'IsComputed' FALSE ist
          void        CalcSensorIfNec ();
// Berechnen des Sensorwertes
  virtual void        CalcSensor () = 0;
// Standardfarbe fuer Sensoren
  virtual COLOR       GetColor ();

  friend INTEGER GetMultiDim   (SIM3DElement*, INTEGER);
  friend void    GetMultiValue (SIM3DElement*, PSHORTREAL);
};

const
  S3DSingleSensor = 41;

class SIM3DSingleSensor : public SIM3DSensor
{
public:
           SIM3DSingleSensor (const STRING&);

  virtual STRING      GetClass ()
          {return "SINGLESENSOR"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DSingleSensor) || SIM3DSensor::IsA (typ);}
};

const
  S3DMultiSensor = 42;

class SIM3DMultiSensor : public SIM3DSensor
{
public:
  INTEGER SizeX, SizeY; // ...mit einer Ausdehnung in Breite und Hoehe
  REAL DeltaX, DeltaY;  // DeltaX und DeltaY bestimmen die Geometrie des Sensors

           SIM3DMultiSensor (const STRING&);
           SIM3DMultiSensor (const SIM3DMultiSensor&);

  virtual STRING      GetClass ()
          {return "MULTISENSOR"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DMultiSensor) || SIM3DSensor::IsA (typ);}

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP&);

// Bestimme die Groesse in einer Dimension
  virtual INTEGER     DimSize (INTEGER i);
};

const
  S3DWhisker = 43;

// Ein SIM3DWhisker ist ein Tasthaar, das auf die Anwesenheit eines Koerpers
// testet. Die Richtung des Sensors wird durch die Lage im Raum ('Transform')
// bestimmt, die Laenge ist in 'Length' gegeben.

class SIM3DWhisker : public SIM3DSingleSensor
{
public:
           SIM3DWhisker (const STRING&);

  virtual STRING      GetClass ()
          {return "WHISKER"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DWhisker) || SIM3DSingleSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DWhisker (*this); }

  virtual void        CalcSensor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
  virtual COLOR       GetColor ();      // Eigene Standardfarbe
};

const
  S3DTactile = 44;

// Ein SIM3DTactile beschreibt ein 'Geschmackstasthaar', der die Farbe einer
// Oberflaeche 'erschmecken' kann. Statt einer Entfernung wird die Staerke
// einer Sensorklasse auf einer Oberflaeche (falls vorhanden, sonst null)
// geliefert.

class SIM3DTactile : public SIM3DWhisker
{
public:
           SIM3DTactile (const STRING&);

  virtual STRING      GetClass ()
          {return "TACTILE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DTactile) || SIM3DWhisker::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DTactile (*this); }

  virtual void        CalcSensor ();
};

const
  S3DReceptor = 45;

// Ein SIM3DReceptor ist im Prinzip ein Photowiederstand, der auf eine
// bestimmte Sensorklasse anspricht.

class SIM3DReceptor : public SIM3DSingleSensor
{
public:
           SIM3DReceptor (const STRING&);

  virtual STRING      GetClass ()
          {return "RECEPTOR"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DReceptor) || SIM3DSingleSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DReceptor (*this); }

  virtual void        CalcSensor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
};

const
  S3DWhiskerField = 46;

// Ein SIM3DWhiskerfield ist ein rechteckiges Feld von Tasthaaren, dessen
// Groesse durch 'SIM3DMultiSensor::DeltaX' und '...::DeltaY' bestimmt ist.
// Die Laenge der Tasthaare ist in Length gegeben.

class SIM3DWhiskerField : public SIM3DMultiSensor
{
public:
           SIM3DWhiskerField (const STRING&);

  virtual STRING      GetClass ()
          {return "WHISKERFIELD"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DWhiskerField) || SIM3DMultiSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DWhiskerField (*this); }

  virtual void        CalcSensor ();
  virtual COLOR       GetColor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
};

const
  S3DCamera = 47;

// SIM3DCamera definiert eine virtuelle Kamera, die ein Bild in einer Sensor-
// klasse liefert. Der Augpunkt liegt im relativen Koordinatenursprung, die
// Bildebene liegt im Abstand eins in Richtung der positiven z-Achse. Die
// Groesse der Bildebene ist durch 'DeltaX' und 'DeltaY' gegeben.

class SIM3DCamera : public SIM3DMultiSensor
{
public:
  BOOLEAN UseZBuffer;

           SIM3DCamera (const STRING&);

  virtual STRING      GetClass ()
          {return "CAMERA"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DCamera) || SIM3DMultiSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DCamera (*this); }

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP&);

  virtual void        CalcSensor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
};

const
  S3DFacette = 48;

// SIM3DFacette: Wie SIM3DCamera, jedoch wird statt der Groesse der Bildebene
// bei 'SIZE' der Oeffnungswinkel angegeben. Dieser Sensor entspricht dem
// Facettenauge von Insekten oder einer Weitwinkel-Kamera.

class SIM3DFacette : public SIM3DMultiSensor
{
public:
          SIM3DFacette (const STRING&);

  virtual STRING      GetClass ()
          {return "FACETTE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DFacette) || SIM3DMultiSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DFacette (*this); }

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        CalcSensor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
};

const
  S3DUltraSonic = 49;

// Ein SIM3DUltraSonic ist ein Ultraschallsensor.
// Der Oeffnungswinkel wird in SIZE angegeben, die Aufloesung in
// RESOLUTION und die Reichweiten (min und max) in DEPTH.

class SIM3DUltraSonic : public SIM3DCamera
{
public:
          SIM3DUltraSonic(const STRING&);
          SIM3DUltraSonic(const SIM3DUltraSonic&);

  virtual STRING GetClass()
          {return "ULTRASONIC";}
  virtual BOOLEAN IsA(INTEGER typ)
          {return (typ == S3DUltraSonic) || SIM3DCamera::IsA(typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DUltraSonic(*this);}
  virtual INTEGER DimSize (INTEGER)
          {return 1;}
  virtual void ReadAttr(const STRING& attr, SCANNER& scan);
  virtual void WriteAttrs (BACKUP& backup);

  virtual void CalcSensor();
};

const
  S3DCompass = 50;

// Ein SIM3DCompass realisiert einen Kompass auf der x-y-Ebene.

class SIM3DCompass : public SIM3DSingleSensor
{
public:
           SIM3DCompass (const STRING&);

  virtual STRING      GetClass ()
          {return "COMPASS"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DCompass) || SIM3DSingleSensor::IsA (typ);}
  virtual SIM3DElement* Copy()
          {return new SIM3DCompass (*this); }

  virtual void        CalcSensor ();
  virtual void        Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL);
};

#endif

