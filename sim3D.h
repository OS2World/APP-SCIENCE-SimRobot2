/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3D.h                                               ****
 **** Inhalt: Beschreibung der Interface-Routinen der 3D-Simulation ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3D_H_
#define _SIM3D_H_

#include "Common.h"
#include "Vector.h"
#include "BasicGraphics.h"
#include "sim3DErrors.h"

#define SimRobotVersion "1.2"

class SIM3DWorld;

class SIMULATION
{
  SIM3DWorld* World;
  BOOLEAN     Ok;
  
  // Hilfsfunktion fuer GetObject
  SIM3DOBJECT GetObject2(const STRING& name,SIM3DOBJECT obj);
public:

  // Oeffne die Definitionsdatei des angegebenen Namens und lese sie ein
  SIMULATION  (const STRING& filename);

  // Beende die Simulation und gib den Speicher frei
  ~SIMULATION ();

  // Speichere den aktuellen Zustand der Simulation in die Definitionsdatei
  // mit dem angegebenen Namen
  void        SaveSim          (const STRING& filename);

  // Konnte die letzte Operation ohne Fehler ausgefuehrt werden?
  BOOLEAN     WasOk            ();


  // Liefert den Aktorport zu dem angegebenen Namen (vollstaendigen Pfad angeb.)
  ACTORPORT   GetActorPort     (const STRING& portname);

  // dito fuer Sensorports
  SENSORPORT  GetSensorPort    (const STRING& portname);

  // Wieviele Dimensionen hat der angegebene Sensorport
  INTEGER     GetSensorDim     (SENSORPORT sp);

  // Welche Ausdehnung hat der Sensorport in der angegebenen Dimension
  INTEGER     GetSensorDimSize (SENSORPORT sp, INTEGER d);

  // Hole den Zustand des Sensors
  void        GetSensorValue   (SENSORPORT sp, PSHORTREAL value);

  // Setze den Wert des angegebenen Aktors
  void        SetActorValue    (ACTORPORT ap, REAL value);

  // Belege den Speicher fuer den Ergebnisvektor eines Sensors
  PSHORTREAL  NewSensorBuffer  (SENSORPORT sp);

  // Gib diesen Speicher wieder frei
  void        DeleteSensorBuffer (SENSORPORT sp, PSHORTREAL buffer);



  // Wieviele Sensorports hat das angegebene Objekt
  INTEGER     SensorPortCount  (SIM3DOBJECT obj);

  // Wieviele Aktorports ...
  INTEGER     ActorPortCount   (SIM3DOBJECT obj);

  // Welche Namen hat der n-te Aktorport fuer dieses Objekt
  STRING      SensorPortName   (SIM3DOBJECT obj, INTEGER n);

  // ... n-te Sensorport ...
  STRING      ActorPortName    (SIM3DOBJECT obj, INTEGER n);


  // Fuehre in der Simulation einen Zeitschritt aus
  void        DoTimeStep       ();


  // Liefert das Objekt mit dem angegebenen Pfadnamen
  SIM3DOBJECT GetObject        (const STRING& name);

  // Liefert den Namen des angegeben Objekts
  STRING      ObjectName       (SIM3DOBJECT obj);

  // Liefert einen String mit Benutzerdaten fuer das Objekt:
  STRING      ObjectRemark     (SIM3DOBJECT obj);

  // Liefert die Klasse des angegebenen Objekts (als String)
  STRING      ObjectClass      (SIM3DOBJECT obj);

  // Wieviele Subobjekte das das Objekt
  INTEGER     SubObjectCount   (SIM3DOBJECT obj);

  // Liefert das n-te Subobjekt
  SIM3DOBJECT SubObject        (SIM3DOBJECT obj, INTEGER n);

  // Zeichne das angegebene Objekt in den Graphikport bis zu einer bestimmten
  // Tiefe (INTEGER) und mit einer bestimmten Laenge (REAL) fuer Richtungs-
  // indikatoren 
  void        DrawObject       (SIM3DOBJECT obj, GRAPHICPORT* gp, 
                                INTEGER detail, REAL scale);

  // Wieviele Makroobjekte sind fuer die Simulation definiert
  INTEGER     MacroCount       ();

  // Hole das n-te Macro
  SIM3DOBJECT GetMacro         (INTEGER n);

  // Hole das Macro mit dem angegebenen Namen
  SIM3DOBJECT GetMacro         (const STRING&);

  // Fuege das angegebene Objekt unter angegebenem Namen in der Wurzel ein
  SIM3DOBJECT AddMacroAs      (SIM3DOBJECT obj, const STRING& asname);

  // Entferne das Objekt aus der Hierarchie
  void        RemoveObject     (SIM3DOBJECT obj);

  // Hole die Position des Objekts relativ zu seinem Eigner
  MOVEMATRIX  GetLocation      (SIM3DOBJECT obj);

  // Setze die Position des Objekts relativ zu seinem Eigner
  void        SetLocation      (SIM3DOBJECT obj, const MOVEMATRIX& m);
};

#endif
