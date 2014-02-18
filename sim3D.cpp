/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3D.cc                                              ****
 **** Inhalt: Implementation der Interface-Routinen der             ****
 ****         3D-Simulation                                         ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3D.h"
#include "sim3DWorld.h"
#include "sim3DReader.h"

// Siehe auch "sim3D.h"

SIMULATION::SIMULATION  (const STRING& FileName)
{
  SCANNER scan (FileName);
  Ok = !scan.ErrorOccured;
  if (Ok)
  {
    scan.NextSymbol ();
    World = (SIM3DWorld*) ReadObject(scan,NULL);
    Ok = World != NOELEMENT;
    if (Ok)
      scan.Expect (sPeriod, sEof);
  }
}

SIMULATION::~SIMULATION ()
{
  delete World;
}

BOOLEAN SIMULATION::WasOk ()
{
  BOOLEAN result = (Ok && (sim3DGetLastError () == sim3DNoError));
  Ok = TRUE;
  return result;
}

void SIMULATION::SaveSim (const STRING& FileName)
{
  BACKUP backup (FileName);
  Ok = !backup.ErrorOccured;
  if (Ok)
  {
    backup.WriteComment ("SaveFile...");
    backup.WriteLine ();
    World->WriteObj (backup);
    backup.WriteSymbol(sPeriod);
  }
}

ACTORPORT SIMULATION::GetActorPort (const STRING& path)
{
  return World->GetActorPort (path);
}

SENSORPORT SIMULATION::GetSensorPort (const STRING& path)
{
  return World->GetSensorPort (path);
}

INTEGER SIMULATION::GetSensorDim (SENSORPORT sp)
{
  Ok = (sp.PortObject != NOELEMENT);
  if (Ok)
    return sp.PortObject->GetSensorDim (sp.PortNr);
  else {
    sim3DOtherError (sim3DInvalidObject);
    return 0;
  }
}

INTEGER SIMULATION::GetSensorDimSize (SENSORPORT sp, INTEGER dim)
{
  Ok = (sp.PortObject != NOELEMENT);
  if (Ok)
    return sp.PortObject->GetSensorDimSize (sp.PortNr, dim);
  else {
    sim3DOtherError (sim3DInvalidObject);
    return 1;
  }
}

void SIMULATION::GetSensorValue (SENSORPORT sp, PSHORTREAL value)
{
  Ok = (sp.PortObject != NOELEMENT);
  if (Ok)
    sp.PortObject->GetSensorValue (sp.PortNr, value);
  else
    sim3DOtherError (sim3DInvalidObject);
}

void SIMULATION::SetActorValue (ACTORPORT ap, REAL value)
{
  Ok = (ap.PortObject != NOELEMENT);
  if (Ok)
    ap.PortObject->SetActorValue (ap.PortNr, value);
  else
    sim3DOtherError (sim3DInvalidObject);
}

PSHORTREAL SIMULATION::NewSensorBuffer (SENSORPORT sp)
{
  INTEGER i, 
          dim;
  LONGINT size = 1;

  dim = GetSensorDim (sp);
  for (i=0;i<dim;i++) size = size * GetSensorDimSize (sp, i);
  return ALLOC_VALUE(size);
}

void SIMULATION::DeleteSensorBuffer (SENSORPORT, PSHORTREAL buffer)
{
  FREE_VALUE(buffer);
}

INTEGER SIMULATION::SensorPortCount (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->SensorPortCount ();
  else {
    sim3DOtherError (sim3DInvalidObject);
    return 0;
  }
}

INTEGER SIMULATION::ActorPortCount (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->ActorPortCount ();
  else {
    sim3DOtherError (sim3DInvalidObject);
    return 0;
  }
}

STRING SIMULATION::SensorPortName (SIM3DOBJECT obj, INTEGER nr)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->SensorPortName (nr);
  else {
    sim3DOtherError (sim3DInvalidObject);
    return "";
  }
}

STRING SIMULATION::ActorPortName (SIM3DOBJECT obj, INTEGER nr)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->ActorPortName (nr);
  else {
    sim3DOtherError (sim3DInvalidObject);
    return "";
  }
}

void SIMULATION::DoTimeStep ()
{
  Ok = TRUE;
  World->DoTimeStep ();
}

SIM3DOBJECT SIMULATION::GetObject2(const STRING& name,SIM3DOBJECT obj)
{
  STRING RealName = name;
  if (obj->IsA(S3DGroup))
    RealName = ((SIM3DGroup*) obj)->ExpandPath (RealName);
  INTEGER pos = RealName.Pos('.');
  STRING head = RealName.First(pos);
  for (INTEGER i = 0; i < SubObjectCount(obj); i++)
    if (ObjectName(SubObject(obj,i)) == head)
      if (pos == RealName.Length())
        return SubObject(obj,i);
      else
        return GetObject2(RealName.ButFirst(pos+1),SubObject(obj,i));
  sim3DOtherError (sim3DInvalidObject);
  Ok = FALSE;
  return NOELEMENT;
}

SIM3DOBJECT SIMULATION::GetObject(const STRING& name)
{
  if (name == "")
    return World;
  else
    return GetObject2(name,World);
}

STRING SIMULATION::ObjectName (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->ElementName;
  else {
    sim3DOtherError (sim3DInvalidObject);
    return "";
  }
}

STRING SIMULATION::ObjectRemark (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->Remark;
  else {
    sim3DOtherError (sim3DInvalidObject);
    return "";
  }
}

STRING SIMULATION::ObjectClass (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->GetClass ();
  else {
    sim3DOtherError (sim3DInvalidObject);
    return "";
  }
}

INTEGER SIMULATION::SubObjectCount (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->SubObjectCount ();
  else {
    sim3DOtherError (sim3DInvalidObject);
    return 0;
  }
}

SIM3DOBJECT SIMULATION::SubObject (SIM3DOBJECT obj, INTEGER nr)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->SubObject (nr);
  else {
    sim3DOtherError (sim3DInvalidObject);
    return NOELEMENT;
  }
}

void SIMULATION::DrawObject (SIM3DOBJECT obj, GRAPHICPORT* gp, INTEGER dl, REAL scale)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    obj->DoDraw (gp, dl, scale);
  else
    sim3DOtherError (sim3DInvalidObject);
}

INTEGER SIMULATION::MacroCount ()
{
  Ok = TRUE;
  return World->HowManyMacros ();
}

SIM3DOBJECT SIMULATION::GetMacro (INTEGER nr)
{
  Ok = TRUE;
  return World->NthMacro (nr);
}

SIM3DOBJECT SIMULATION::GetMacro (const STRING& name)
{
  Ok = TRUE;
  return World->FindMacro (name);
}

SIM3DOBJECT SIMULATION::AddMacroAs (SIM3DOBJECT obj, const STRING& name)
{
  if (obj != NOELEMENT) {
    const char* p = name;
    INTEGER i;
    for(i = name.Length()-1; i >= 0 && p[i] != '.'; i--);
    SIM3DOBJECT objGroup = GetObject(name.First(i < 0 ? 0 : i));
    if (objGroup != NOELEMENT)
      if (objGroup->IsA(S3DGroup)) {
        SIM3DOBJECT obj2 = obj->Copy ();
        ((SIM3DGroup*) objGroup)->AddObject (obj2);
        obj2->ElementName = name.ButFirst(i+1);
        Ok = TRUE;
        return obj2;
      }
      else {
        Ok = FALSE;
        sim3DOtherError (sim3DGroupExpected);
        return NOELEMENT;
      }
  }
  Ok = FALSE;
  sim3DOtherError (sim3DInvalidObject);
  return NOELEMENT;
}

void SIMULATION::RemoveObject (SIM3DOBJECT obj)
{
  if (obj != NOELEMENT && World->RemoveObject (obj))
  {
    Ok = TRUE;
    delete obj;
  }
  else
  {
    Ok = FALSE;
    sim3DOtherError (sim3DInvalidObject);
  }
}

MOVEMATRIX SIMULATION::GetLocation (SIM3DOBJECT obj)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    return obj->Transform;
  else {
    sim3DOtherError (sim3DInvalidObject);
    return MOVEMATRIX ();
  }
}

void SIMULATION::SetLocation (SIM3DOBJECT obj, const MOVEMATRIX& m)
{
  Ok = (obj != NOELEMENT);
  if (Ok)
    obj->Transform = m;
  else 
    sim3DOtherError (sim3DInvalidObject);
}
