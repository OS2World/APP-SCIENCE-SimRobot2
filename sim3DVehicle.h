/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DVehicle.h                                        ****
 **** Inhalt: Beschreibt die Klassen SIM3DVehicle (Fahrzeuge) und   ****
 ****         SIM3DManoeuvrable (Fahrzeuge mit einem lenkbaren Rad) ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DVEHICLE_H_
#define _SIM3DVEHICLE_H_

#include "sim3DBasics.h"

extern APORTDEF S3DVehicleAPortRZ;
extern APORTDEF S3DVehicleAPortRY;
extern APORTDEF S3DVehicleAPortRX;
extern APORTDEF S3DVehicleAPortTZ;
extern APORTDEF S3DVehicleAPortTY;
extern APORTDEF S3DVehicleAPortTX;
extern SPORTDEF S3DVehicleSPort;

// Ein SIM3DVehicle ist ein Fahrzeug, dass sich beliebig im Raum bewegen kann
// Das Fahrzeug hat sechs Aktorports fuer alle sechs Freiheitsgrade
// und einen Sensorport, der mitteilt, ob die Bewegung ausgefuehrt wurde.
// Fuer jeden Aktorport kann ein REAL-Tripel angegeben werden, dass die 
// ausgefuehrten Kommandos fuer (-1/0/+1) angibt. Dazwischen liegende Werte
// werden linear interpoliert. Damit kann man die Eigenschaften der (gedachten)
// Motoren des Fahrzeugs manipulieren.
// Der Sensorport liefert entweder nur 0 oder 1.

const
  S3DVehicle = 60;

class SIM3DVehicle : public SIM3DGroup
{
protected:
  REAL Executed;        // Wurde die Bewegung ausgefuehrt?
  REAL CmdTransX;       // Kommando Translation in X-Richtung
  REAL CmdTransY;       // Kommando Translation in Y-Richtung
  REAL CmdTransZ;       // Kommando Translation in Z-Richtung
  REAL CmdRotX;         // Kommando Rotation um X-Achse
  REAL CmdRotY;         // Kommando Rotation um Y-Achse
  REAL CmdRotZ;         // Kommando Rotation um Z-Achse
  VECTOR RestrTransX;   // Motorenbeschreibungen (siehe oben)
  VECTOR RestrTransY;   // - " -
  VECTOR RestrTransZ;   // - " -
  VECTOR RestrRotX;     // - " -
  VECTOR RestrRotY;     // - " -
  VECTOR RestrRotZ;     // - " -
public:
           SIM3DVehicle (const STRING&);
           SIM3DVehicle (const SIM3DVehicle&);

  virtual STRING      GetClass ()
          { return "VEHICLE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DVehicle) || SIM3DGroup::IsA (typ)); }
  virtual SIM3DElement* Copy()
          { return new SIM3DVehicle (*this); }

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP& backup);

  virtual APORTDEF*   GetAPortDef ();
  virtual SPORTDEF*   GetSPortDef ();

  virtual void        MakeTimeStep (const MOVEMATRIX&);

  friend  void        SetVehicleActorTX (SIM3DElement*, REAL);
  friend  void        SetVehicleActorTY (SIM3DElement*, REAL);
  friend  void        SetVehicleActorTZ (SIM3DElement*, REAL);
  friend  void        SetVehicleActorRX (SIM3DElement*, REAL);
  friend  void        SetVehicleActorRY (SIM3DElement*, REAL);
  friend  void        SetVehicleActorRZ (SIM3DElement*, REAL);
  friend  void        GetVehicleSensor (SIM3DElement*, PSHORTREAL);
};

// Ein SIM3DManoeuvrable ist ein Fahrzeug, dass ueber ein lenkbares Rad verfuegt.
// Das Fahrzeug hat zwei Aktorports fuer das Fahren und Lenken und einen
// Sensorport, der mitteilt, ob die Bewegung ausgefuehrt wurde. Mit dem Port
// SPEED legt man die Geschwindigkeit des Dreirads fest, mit STEERING den
// Lenkwinkel. Fuer SPEED kann ein REAL-Tripel angegeben werden, dass die
// ausgefuehrten Kommandos fuer (-1/0/+1) angibt. Dazwischen liegende Werte
// werden linear interpoliert. Damit kann man die Eigenschaften des (gedachten)
// Motors manipulieren.
// Der Sensorport EXECUTED liefert entweder nur 0 oder 1. Der Sensorport DISTANCE
// liefert die Odometer-Messungen der beiden Raeder fuer den letzten Simulations-
// schritt.

const
  S3DManoeuvrable = 61;

class SIM3DManoeuvrable : public SIM3DVehicle
{
protected:
  REAL CmdSteering;     // Kommando Lenkeinschlag
  REAL DistanceLeft,    // In einem Schritt gefahrene Strecke
       DistanceRight;
public:
  REAL WheelBase;       // Radstand
  REAL AxleWidth;       // Abstand der Antriebsraeder
  REAL Factor;          // maximaler Lenkwinkel
public:
           SIM3DManoeuvrable (const STRING&);
           SIM3DManoeuvrable (const SIM3DManoeuvrable&);

  virtual STRING      GetClass ()
          { return "MANOEUVRABLE"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          { return ((typ == S3DManoeuvrable) || SIM3DVehicle::IsA (typ)); }
  virtual SIM3DElement* Copy()
          { return new SIM3DManoeuvrable (*this); }

  virtual void        ReadAttr (const STRING&, SCANNER&);
  virtual void        WriteAttrs (BACKUP& backup);

  virtual APORTDEF*   GetAPortDef ();
  virtual SPORTDEF*   GetSPortDef ();

  virtual void        MakeTimeStep (const MOVEMATRIX&);

  friend  void        SetManoeuvrable0 (SIM3DElement*, REAL);
  friend  void        SetManoeuvrable1 (SIM3DElement*, REAL);
  friend  void        GetManoeuvrable (SIM3DElement*, PSHORTREAL);
};

#endif
