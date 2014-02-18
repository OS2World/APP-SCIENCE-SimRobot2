/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DMoveable.h                                       ****
 **** Inhalt: Beschreibt die Klassen SIM3DMoveable, SIM3DTurnable   ****
 ****         und SIM3DExtendable                                   ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DMOVEABLE_H_
#define _SIM3DMOVEABLE_H_

#include "sim3DBasics.h"

extern APORTDEF S3DMoveableAPort;
extern SPORTDEF S3DMoveableSPort0;
extern SPORTDEF S3DMoveableSPort;

const
  S3DMoveable = 20;

// SIM3DMoveable beschreibt alle beweglichen Elemente, die sich innerhalb
// von festen Grenzen mit einem Freiheitsgrad bewegen koennen. Es ist eine
// abstrake Klasse, die zur Realisierung von Dreh- und Teleskopgelenken
// dient.

class SIM3DMoveable : public SIM3DGroup
{
protected:
  INTEGER Axis;         // Welche Achse wird der Bewegung zugrunde gelegt
                        // (Drehachse oder Richtung der Bewegung)
  REAL ActValue;        // Stellung des Gelenks
  REAL Command, Executed;       // Angeforderte Bewegung, ausgefuehrter Anteil
  REAL Lowest, Highest;         // Grenzen der Bewegung
  REAL Factor;                  // Faktor, mit dem 'Command' malgenommen wird.
  REAL SensWhere;               // Relative Stellung des Gelenks.
public:
           SIM3DMoveable (const STRING&);
           SIM3DMoveable (const SIM3DMoveable&);

  virtual STRING      GetClass ()
          { return "MOVEABLE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DMoveable) || SIM3DGroup::IsA (typ)); }
  virtual SIM3DElement* Copy() = 0;

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP& backup);

  virtual APORTDEF*   GetAPortDef ();   // Portdef. fuer die Gelenke
  virtual SPORTDEF*   GetSPortDef ();

  virtual void        ComputeSensors ();  // Lage des Gelenks hier berechnen

// MakeTimeStep muss fuer ein Gelenk das Aktorkommando ausfuehren.
  virtual void        MakeTimeStep (const MOVEMATRIX& m);
// Die relative Stellung ist nicht nur von 'Transform' sondern auch von
// 'ActValue' und 'Axis' abhaengig, aber in der abstrakten Klasse undefiniert.
  virtual MOVEMATRIX  GetLocalTransform () = 0;

  friend  void        SetMoveActor (SIM3DElement* obj, REAL value);
  friend  void        GetMoveSensor (SIM3DElement* obj, PSHORTREAL value);
  friend  void        GetWhereSensor (SIM3DElement* obj, PSHORTREAL value);
};


const
  S3DTurnable = 21;

// SIM3DTurnable ist ein Drehgelenk. Nur GetLocalTransform muss wirklich
// neudefiniert werden.

class SIM3DTurnable : public SIM3DMoveable
{
public:
           SIM3DTurnable (const STRING&);

  virtual STRING      GetClass ()
          { return "TURNABLE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DTurnable) || SIM3DMoveable::IsA (typ)); }
  virtual SIM3DElement* Copy ()
          { return new SIM3DTurnable (*this); }

  virtual MOVEMATRIX  GetLocalTransform ();
};

const
  S3DExtendible = 22;

// SIM3DExtendible ist ein Teleskopgelenk. Nur GetLocalTransform muss wirklich
// neudefiniert werden.

class SIM3DExtendible : public SIM3DMoveable
{
public:
           SIM3DExtendible (const STRING&);

  virtual STRING      GetClass ()
          { return "EXTENDIBLE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DExtendible) || SIM3DMoveable::IsA (typ)); }
  virtual SIM3DElement* Copy ()
          { return new SIM3DExtendible (*this); }

  virtual MOVEMATRIX  GetLocalTransform ();
};


#endif
