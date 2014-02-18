/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DVehicle.cc                                       ****
 **** Inhalt: Implementiert die Klasse SIM3DVehicle                 ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DVehicle.h"
#include "sim3DWorld.h"
#include "sim3DErrors.h"

//-- Portdefinitionen ----------------------------------------------------

void SetVehicleActorTX (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdTransX = value;
}

void SetVehicleActorTY (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdTransY = value;
}

void SetVehicleActorTZ (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdTransZ = value;
}

void SetVehicleActorRX (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdRotX = value;
}

void SetVehicleActorRY (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdRotY = value;
}

void SetVehicleActorRZ (SIM3DElement* obj, REAL value)
{
  ((SIM3DVehicle*) obj)->CmdRotZ = value;
}

void GetVehicleSensor (SIM3DElement* obj, PSHORTREAL value)
{
  *value = (SHORTREAL) ((SIM3DVehicle*) obj)->Executed;
}

APORTDEF S3DVehicleAPortRZ (NULL,               "ROT_Z",   SetVehicleActorRZ);
APORTDEF S3DVehicleAPortRY (&S3DVehicleAPortRZ, "ROT_Y",   SetVehicleActorRY);
APORTDEF S3DVehicleAPortRX (&S3DVehicleAPortRY, "ROT_X",   SetVehicleActorRX);
APORTDEF S3DVehicleAPortTZ (&S3DVehicleAPortRX, "TRANS_Z", SetVehicleActorTZ);
APORTDEF S3DVehicleAPortTY (&S3DVehicleAPortTZ, "TRANS_Y", SetVehicleActorTY);
APORTDEF S3DVehicleAPortTX (&S3DVehicleAPortTY, "TRANS_X", SetVehicleActorTX);
SPORTDEF S3DVehicleSPort   (NULL,               "EXECUTED", GetVehicleSensor);

//--- SIM3DVehicle ------------------------------------------------------
// 
// Siehe auch "sim3DVehicle.h"

SIM3DVehicle::SIM3DVehicle (const STRING& name) : SIM3DGroup (name),
  RestrTransX (-1, 0, 1),
  RestrTransY (-1, 0, 1),
  RestrTransZ (-1, 0, 1),
  RestrRotX (-1, 0, 1),
  RestrRotY (-1, 0, 1),
  RestrRotZ (-1, 0, 1)
{
  Executed = 0.0;
  CmdTransX = 0.0;
  CmdTransY = 0.0;
  CmdTransZ = 0.0;
  CmdRotX = 0.0;
  CmdRotY = 0.0;
  CmdRotZ = 0.0;
}

SIM3DVehicle::SIM3DVehicle (const SIM3DVehicle& Obj) : SIM3DGroup (Obj),
  RestrTransX (Obj.RestrTransX),
  RestrTransY (Obj.RestrTransY),
  RestrTransZ (Obj.RestrTransZ),
  RestrRotX (Obj.RestrRotX),
  RestrRotY (Obj.RestrRotY),
  RestrRotZ (Obj.RestrRotZ)
{
  Executed = Obj.Executed;
  CmdTransX = Obj.CmdTransX;
  CmdTransY = Obj.CmdTransY;
  CmdTransZ = Obj.CmdTransZ;
  CmdRotX = Obj.CmdRotX;
  CmdRotY = Obj.CmdRotY;
  CmdRotZ = Obj.CmdRotZ;
}

APORTDEF* SIM3DVehicle::GetAPortDef ()
{
  return &S3DVehicleAPortTX;
}

SPORTDEF* SIM3DVehicle::GetSPortDef ()
{
  return &S3DVehicleSPort;
}

void SIM3DVehicle::ReadAttr (const STRING& attr, SCANNER& scan)
{
       if (attr == "RESTR_TRANS_X")
    (void) GetCorrectVector (scan, RestrTransX);
  else if (attr == "RESTR_TRANS_Y")
    (void) GetCorrectVector (scan, RestrTransY);
  else if (attr == "RESTR_TRANS_Z")
    (void) GetCorrectVector (scan, RestrTransZ);
  else if (attr == "RESTR_ROT_X")
    (void) GetCorrectVector (scan, RestrRotX);
  else if (attr == "RESTR_ROT_Y")
    (void) GetCorrectVector (scan, RestrRotY);
  else if (attr == "RESTR_ROT_Z")
    (void) GetCorrectVector (scan, RestrRotZ);
  else
    SIM3DGroup::ReadAttr (attr, scan);
}

void SIM3DVehicle::WriteAttrs (BACKUP& backup)
{
  SIM3DGroup::WriteAttrs (backup);

  if (RestrTransX != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_TRANS_X");
    backup.WriteVector (RestrTransX);
    backup.ListNext ();
  }
  if (RestrTransY != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_TRANS_Y");
    backup.WriteVector (RestrTransY);
    backup.ListNext ();
  }
  if (RestrTransZ != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_TRANS_Z");
    backup.WriteVector (RestrTransZ);
    backup.ListNext ();
  }
  if (RestrRotX != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_ROT_X");
    backup.WriteVector (RestrRotX);
    backup.ListNext ();
  }
  if (RestrRotY != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_ROT_Y");
    backup.WriteVector (RestrRotY);
    backup.ListNext ();
  }
  if (RestrRotZ != VECTOR(-1,0,1)) {
    backup.WriteIdent ("RESTR_ROT_Z");
    backup.WriteVector (RestrRotZ);
    backup.ListNext ();
  }
}

REAL CalcRestrValue (VECTOR restr, REAL v)
// Berechnung der Interpolation fuer die Motorenbeschraenkung
{
       if (v < -1) v = -1;
  else if (v > 1) v = 1;

  if (v < 0)
    return restr.y - (restr.x - restr.y) * v;
  else if (v > 0)
    return restr.y + (restr.z - restr.y) * v;
  else
    return restr.y;
}

void SIM3DVehicle::MakeTimeStep (const MOVEMATRIX& M)
{
  // Berechnung der Ortsaenderung
  REAL transX = CalcRestrValue (RestrTransX, CmdTransX);
  REAL transY = CalcRestrValue (RestrTransY, CmdTransY);
  REAL transZ = CalcRestrValue (RestrTransZ, CmdTransZ);
  REAL rotX = CalcRestrValue (RestrRotX, CmdRotX);
  REAL rotY = CalcRestrValue (RestrRotY, CmdRotY);
  REAL rotZ = CalcRestrValue (RestrRotZ, CmdRotZ);

  if (transX || transY || transZ || rotX || rotY || rotZ) {

    // Berechnung der neuen Position
    MOVEMATRIX OldTransf = Transform;
    MATRIX Turn = TurnXYMatrix (rotZ) * TurnXZMatrix (rotY) * 
                  TurnYZMatrix (rotX);
    Transform = Transform * 
                MOVEMATRIX (Turn, Turn * VECTOR (transX, transY, transZ));

    // Neue Position antesten und bei Bedarf zuruecknehemen
    MakeStep (M);
    MakeNewBoundingBox ();
    RegisterBoundingBox ();
    if (World->DoesIntersect (this)) {
      StepBack ();
      Transform = OldTransf;
      MakeNewBoundingBox ();
      RegisterBoundingBox ();
      Executed = 0.0;
    }
    else
      Executed = 1;
  }
  else
    Executed = 1;

  CmdTransX = 0.0;
  CmdTransY = 0.0;
  CmdTransZ = 0.0;
  CmdRotX = 0.0;
  CmdRotY = 0.0;
  CmdRotZ = 0.0;

  SIM3DGroup::MakeTimeStep (M);
}

//-- Portdefinitionen ----------------------------------------------------

void SetManoeuvrable0 (SIM3DElement* obj, REAL value)
{
  ((SIM3DManoeuvrable*) obj)->CmdTransX = value;
}

void SetManoeuvrable1 (SIM3DElement* obj, REAL value)
{
  ((SIM3DManoeuvrable*) obj)->CmdSteering = value;
}

INTEGER GetManoeuvrableDim (SIM3DElement*, INTEGER dim)
{
  return dim ? 1 : 2;
}

void GetManoeuvrable (SIM3DElement* obj, PSHORTREAL value)
{
  value[0] = (SHORTREAL) ((SIM3DManoeuvrable*) obj)->DistanceLeft;
  value[1] = (SHORTREAL) ((SIM3DManoeuvrable*) obj)->DistanceRight;
}

APORTDEF S3DManoeuvrableAPort1 (NULL, "STEERING", SetManoeuvrable1);
APORTDEF S3DManoeuvrableAPort0 (&S3DManoeuvrableAPort1, "SPEED", SetManoeuvrable0);
SPORTDEF S3DManoeuvrableSPort(&S3DVehicleSPort,"DISTANCE",1,GetManoeuvrableDim,GetManoeuvrable);

//--- SIM3DManoeuvrable --------------------------------------------------
//
// Siehe auch "sim3DVehicle.h"

SIM3DManoeuvrable::SIM3DManoeuvrable (const STRING& name) : SIM3DVehicle (name)
{
  Factor = 1;
  WheelBase = 1;
  AxleWidth = 1;
  DistanceLeft = 0;
  DistanceRight = 0;
}

SIM3DManoeuvrable::SIM3DManoeuvrable (const SIM3DManoeuvrable& Obj)
: SIM3DVehicle (Obj)
{
  Factor = Obj.Factor;
  WheelBase = Obj.WheelBase;
  AxleWidth = Obj.AxleWidth;
  DistanceLeft = Obj.DistanceLeft;
  DistanceRight = Obj.DistanceRight;
}

APORTDEF* SIM3DManoeuvrable::GetAPortDef ()
{
  return &S3DManoeuvrableAPort0;
}

SPORTDEF* SIM3DManoeuvrable::GetSPortDef ()
{
  return &S3DManoeuvrableSPort;
}

void SIM3DManoeuvrable::ReadAttr (const STRING& attr, SCANNER& scan)
{
       if (attr == "RESTR_SPEED")
    (void) GetCorrectVector (scan, RestrTransX);
  else if (attr == "FACTOR")
    (void) GetCorrectReal (scan, Factor);
  else if (attr == "WHEEL_BASE")
    (void) GetCorrectReal (scan, WheelBase);
  else if (attr == "AXLE_WIDTH")
    (void) GetCorrectReal (scan, AxleWidth);
  else
    SIM3DGroup::ReadAttr (attr, scan); // nicht sim3DVehicle!
}

void SIM3DManoeuvrable::WriteAttrs (BACKUP& backup)
{
  SIM3DGroup::WriteAttrs (backup); // nicht sim3DVehicle!

  if (RestrTransX != VECTOR(1,0,1)) {
    backup.WriteIdent ("RESTR_SPEED");
    backup.WriteVector (RestrTransX);
    backup.ListNext ();
  }
  if(Factor != 1)
    backup.WriteIdRealNext ("FACTOR", Factor);
  if(WheelBase != 1)
    backup.WriteIdRealNext ("WHEEL_BASE", WheelBase);
  if(AxleWidth != 1)
    backup.WriteIdRealNext ("AXLE_WIDTH", AxleWidth);
}

void SIM3DManoeuvrable::MakeTimeStep (const MOVEMATRIX& M)
{
  DistanceLeft = 0;
  DistanceRight = 0;
  if (CmdSteering < -1)
    CmdSteering = -1;
  else if (CmdSteering > 1)
    CmdSteering = 1;
  REAL steering = CmdSteering * Factor;
  while (steering > 90) steering -= 180;
  while (steering <= -90) steering += 180;
  steering = (steering < 0 ? -90 : 90) - steering;
  if (CmdTransX && Factor) { // Wenn gefahren werden soll
    if (!steering) // Lenkung blockiert
      Executed = 0;
    else if (steering == 90) { // Wenn Lenkung nicht eingeschlagen
      REAL r = CmdTransX;
      SIM3DVehicle::MakeTimeStep(M);
      if(Executed)
      {
        DistanceLeft = r;
        DistanceRight = r;
      }
      return;
    }
    else { // Wenn Lenkung eingeschlagen
      // Berechnung der neuen Position
      REAL transX = CalcRestrValue (RestrTransX, CmdTransX);
      REAL r = tan(steering / 180 * PI) * WheelBase;
      REAL a = transX / r;
      MOVEMATRIX OldTransf = Transform;
      Transform = Transform *
                  MOVEMATRIX (TurnXYMatrix (a / PI * 180),
                              VECTOR (sin(a) * r, r - cos(a) * r,0));
      // Neue Position antesten und bei Bedarf zuruecknehemen
      MakeStep (M);
      MakeNewBoundingBox ();
      RegisterBoundingBox ();
      if (World->DoesIntersect (this)) {
        StepBack ();
        Transform = OldTransf;
        MakeNewBoundingBox ();
        RegisterBoundingBox ();
        Executed = 0.0;
      }
      else
      {
        Executed = 1;
        if(cos(a) > 0)
        {
          DistanceLeft = (r - AxleWidth / 2) / r * CmdTransX;
          DistanceRight = (r + AxleWidth / 2) / r * CmdTransX;
        }
        else
        {
          DistanceLeft = (r + AxleWidth / 2) / r * CmdTransX;
          DistanceRight = (r - AxleWidth / 2) / r * CmdTransX;
        }
      }
    }
  }
  else
    Executed = 1;

  CmdTransX = 0.0;
  SIM3DGroup::MakeTimeStep (M); // nicht sim3DVehicle!
}
