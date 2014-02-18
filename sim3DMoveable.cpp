/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DMoveable.cc                                      ****
 **** Inhalt: Implementiert die Klassen SIM3DMoveable, SIM3DTurnable ***
 ****         und  SIM3DExtendable                                  ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DMoveable.h"
#include "sim3DWorld.h"
#include "sim3DErrors.h"

//-- Portdefinitionen ----------------------------------------------------
//
// Die Ports fuer Gelenke heissen "MOVE" (Aktor) und "EXECUTED" (Sensor)
// Siehe auch "sim3DPortDef.h"

void SetMoveActor  (SIM3DElement* obj, REAL value)
{
  if (value < -1) value = -1;
  else if (value > 1) value = 1;

  ((SIM3DMoveable*) obj)->Command = value;
}

void GetMoveSensor (SIM3DElement* obj, PSHORTREAL value)
{
  *value = (SHORTREAL) ((SIM3DMoveable*) obj)->Executed;
}

void GetWhereSensor (SIM3DElement* obj, PSHORTREAL value)
{
  *value = (SHORTREAL) ((SIM3DMoveable*) obj)->SensWhere;
}

APORTDEF S3DMoveableAPort  (NULL, "MOVE", SetMoveActor);
SPORTDEF S3DMoveableSPort0 (NULL, "WHERE", GetWhereSensor);
SPORTDEF S3DMoveableSPort  (&S3DMoveableSPort0, "EXECUTED", GetMoveSensor);

//--- SIM3DMoveable ------------------------------------------------------

SIM3DMoveable::SIM3DMoveable (const STRING& name) : SIM3DGroup (name)
{
  Axis     = 0;
  ActValue = 0.0;
  Command  = 0.0;
  Executed = 0.0;
  Lowest   = 0.0;
  Highest  = 0.0;
  Factor   = 1.0;
  SensWhere= 0.0;
}

SIM3DMoveable::SIM3DMoveable (const SIM3DMoveable& Obj) : SIM3DGroup (Obj)
{
  Axis     = Obj.Axis;
  ActValue = Obj.ActValue;
  Command  = Obj.Command;
  Executed = Obj.Executed;
  Lowest   = Obj.Lowest;
  Highest  = Obj.Highest;
  Factor   = Obj.Factor;
  SensWhere= Obj.SensWhere;
}

APORTDEF* SIM3DMoveable::GetAPortDef ()
{
  return &S3DMoveableAPort;
}

SPORTDEF* SIM3DMoveable::GetSPortDef ()
{
  return &S3DMoveableSPort;
}

void SIM3DMoveable::ReadAttr (const STRING& attr, SCANNER& scan)
{
       if (attr == "X_AXIS")   Axis = 0;
  else if (attr == "Y_AXIS")   Axis = 1;
  else if (attr == "Z_AXIS")   Axis = 2;
  else if (attr == "FACTOR")
    (void) GetCorrectReal (scan, Factor);
  else if (attr == "START")
    (void) GetCorrectReal (scan, ActValue);
  else if (attr == "RANGE")
    (void) GetCorrectRealPair (scan, Lowest, Highest);
  else
    SIM3DGroup::ReadAttr (attr, scan);
}

void SIM3DMoveable::WriteAttrs (BACKUP& backup)
{
  SIM3DGroup::WriteAttrs (backup);
  switch (Axis) {
    case 0: backup.WriteIdent ("X_AXIS"); break;
    case 1: backup.WriteIdent ("Y_AXIS"); break;
    case 2: backup.WriteIdent ("Z_AXIS"); break;
  }
  backup.ListNext ();

  backup.WriteIdRealNext ("FACTOR", Factor);
  backup.WriteIdRealNext ("START", ActValue);

  backup.WriteIdent  ("RANGE");
  backup.WriteSymbol (sLParen);
  backup.WriteReal   (Lowest);
  backup.WriteSymbol (sComma);
  backup.WriteReal   (Highest);
  backup.WriteSymbol (sRParen);
  backup.ListNext    ();
}

void SIM3DMoveable::ComputeSensors ()
{
  REAL RangeSize = Highest - Lowest;

  if (RangeSize > EPSILON) 
    SensWhere = (ActValue - Lowest) / RangeSize;
  else
    SensWhere = 0.0;

  SIM3DGroup::ComputeSensors ();
}

void SIM3DMoveable::MakeTimeStep (const MOVEMATRIX& M)
{
// Rette den aktuellen Zustand
  REAL OldValue = ActValue;

// Das Kommando kann nur innerhalb der Grenzen fuer das Gelenk ausgefuehrt 
// werden. Ist Lowest == Highest == 0.0, sind keine Grenzen vorgegeben.
  if (Command != 0.0) {
    if (Lowest != 0.0 || Highest != 0.0) {
      if ((ActValue + Command * Factor) > Highest) 
        Command = (Highest - ActValue) / Factor;        
      if ((ActValue + Command * Factor) < Lowest)
        Command = (Lowest - ActValue) / Factor;
    }
    ActValue = ActValue + Command * Factor;

// Stelle versuchshalber die neue Position her
    MakeStep (M);
// Aktualisiere die umgebenden Rechtecke
    MakeNewBoundingBox ();
    RegisterBoundingBox ();
// Test auf Ueberlappung:
    if (World->DoesIntersect (this)) {
      StepBack ();              // Bei einer Ueberlappung muessen alle
      ActValue = OldValue;      // Schritte wieder zurueckgenommen werden
      MakeNewBoundingBox ();    // ...
      RegisterBoundingBox ();
      Executed = 0.0;           // Damit wurde das Kommando nicht ausgefuehrt
    }
    else
      Executed = abs (Command);
    Command = 0.0;
  }
  else
    Executed = 0.0;

  SIM3DGroup::MakeTimeStep (M); // Fuehre die Kommandos fuer alle Unterobjekte 
                                // aus
}

/* --- SIM3DTurnable -------------------------------------------------- */

SIM3DTurnable::SIM3DTurnable (const STRING& name) : SIM3DMoveable (name) {}

MOVEMATRIX SIM3DTurnable::GetLocalTransform ()
// Die relative Lage des Gelenks und seiner angeschlossenen Objekte ist die
// Lage, die durch Transform angegeben ist, verknuepft mit der Drehung um
// eine Achse.
{
  switch (Axis) {
    case 0:
      return Transform * MOVEMATRIX (TurnYZMatrix (ActValue), VECTOR ());
    case 1:
      return Transform * MOVEMATRIX (TurnXZMatrix (ActValue), VECTOR ());
    case 2:
      return Transform * MOVEMATRIX (TurnXYMatrix (ActValue), VECTOR ());
  }
  sim3DFatalError (sim3DInternalError);
  return MOVEMATRIX ();
}



/* --- SIM3DExtendible -------------------------------------------------- */

SIM3DExtendible::SIM3DExtendible (const STRING& name) : SIM3DMoveable (name) {}

MOVEMATRIX SIM3DExtendible::GetLocalTransform ()
// Die relative Lage des Gelenks und seiner angeschlossenen Objekte ist die
// Lage, die durch Transform angegeben ist, verknuepft mit der Translation
// entlang einer Achse.
{
  switch (Axis) {
    case 0:
      return Transform * MOVEMATRIX (MATRIX (), VECTOR (ActValue, 0.0, 0.0));
    case 1:
      return Transform * MOVEMATRIX (MATRIX (), VECTOR (0.0, ActValue, 0.0));
    case 2:
      return Transform * MOVEMATRIX (MATRIX (), VECTOR (0.0, 0.0, ActValue));
  }
  sim3DFatalError (sim3DInternalError);
  return MOVEMATRIX ();
}

