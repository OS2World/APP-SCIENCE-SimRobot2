#include <stdlib.h>
#include "Controller.h"

//////////////////////////////////////////////////////////////////////////
// class VIEW

VIEW::VIEW (const STRING& sName,INTEGER xSize,INTEGER ySize,INTEGER numClasses)
 : SIM3DMultiSensor(sName)
{
  SizeX = xSize;
  SizeY = ySize;
  KnowClasses = TRUE;
  for (int i = 1; i < numClasses; i++)
    Classes.AddElement (i);
  ResizeBuffer ();
}

void VIEW::CalcSensor()
{
  CalcView();
}

//////////////////////////////////////////////////////////////////////////
// class CLICKINFO

double CLICKINFO::DistanceTo(SIM3DOBJECT obj) const
{
  VECTOR v = obj->Transform.Offset;
  while(obj->Owner && obj->Owner != m_obj)
  {
    obj = obj->Owner;
    v = obj->Transform * v;
  }
  if(obj->Owner == m_obj)
  {
    v = m_m * v;
    v = v * ((m_dDist - 1) / (m_dDist - v.z));
    v.z = 0;
    return abs(v - m_v) / m_dZoom;
  }
  else
    return HUGE_VAL;
}

//////////////////////////////////////////////////////////////////////////
// class CONTROLLER

SIMULATION* CONTROLLER::m_pSim = 0;
void* CONTROLLER::m_pLastAllocated = 0;

SENSORPORT CONTROLLER::AddView(VIEW* pView)
{
  ((SIM3DGroup*) GetObject(""))->AddObject((SIM3DElement*) pView);
  return GetSensorPort(ObjectName((SIM3DElement*) pView)+STRING(".VALUE"));
}

//////////////////////////////////////////////////////////////////////////
// operator new initializes all members to zero

void* CONTROLLER::operator new(size_t nSize)
{
  m_pLastAllocated = calloc(1,nSize);
  return m_pLastAllocated;
}

//////////////////////////////////////////////////////////////////////////
// CleanUp deallocates object if exception was thrown in constructor

void CONTROLLER::CleanUp()
{
  free(m_pLastAllocated);
}

//////////////////////////////////////////////////////////////////////////
// Clean all error flags

void CONTROLLER::Clean()
{
  sim3DTakeLastError();
}

//////////////////////////////////////////////////////////////////////////
// Check WasOk for many types and throw XCONTROLLER on error

void CONTROLLER::Check()
{
  if(!m_pSim->WasOk())
    throw (XCONTROLLER(XCONTROLLER::InvalidObject));
}

INTEGER CONTROLLER::Check(INTEGER i)
{
  if(m_pSim->WasOk())
    return i;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidObject);
}

STRING CONTROLLER::Check(const STRING& s)
{
  if(m_pSim->WasOk())
    return s;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidObject);
}

SIM3DOBJECT CONTROLLER::Check(SIM3DOBJECT s)
{
  if(m_pSim->WasOk())
    return s;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidObject);
}

PSHORTREAL CONTROLLER::Check(PSHORTREAL p)
{
  if(m_pSim->WasOk())
    return p;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidObject);
}

MOVEMATRIX CONTROLLER::Check(const MOVEMATRIX& m)
{
  if(m_pSim->WasOk())
    return m;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidObject);
}

//////////////////////////////////////////////////////////////////////////
// Check WasOk for SIMULATION API and throw XCONTROLLER on error

ACTORPORT CONTROLLER::GetActorPort(const STRING& portname)
{
  Clean();
  ACTORPORT ap = m_pSim->GetActorPort(portname);
  if(m_pSim->WasOk())
    return ap;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidActorPort,portname);
}

SENSORPORT CONTROLLER::GetSensorPort(const STRING& portname)
{
  Clean();
  SENSORPORT sp = m_pSim->GetSensorPort(portname);
  if(m_pSim->WasOk())
    return sp;
  else
    throw XCONTROLLER(XCONTROLLER::InvalidSensorPort,portname);
}

INTEGER CONTROLLER::GetSensorDim(SENSORPORT sp)
{
  Clean();
  return Check(m_pSim->GetSensorDim(sp));
}

INTEGER CONTROLLER::GetSensorDimSize(SENSORPORT sp,INTEGER d)
{
  Clean();
  return Check(m_pSim->GetSensorDimSize(sp,d));
}

void CONTROLLER::GetSensorValue(SENSORPORT sp,PSHORTREAL value)
{
  Clean();
  m_pSim->GetSensorValue(sp,value);
  Check();
}

void CONTROLLER::SetActorValue(ACTORPORT ap, REAL value)
{
  Clean();
  m_pSim->SetActorValue(ap,value);
  Check();
}

PSHORTREAL CONTROLLER::NewSensorBuffer(SENSORPORT sp)
{
  Clean();
  return Check(m_pSim->NewSensorBuffer(sp));
}

void CONTROLLER::DeleteSensorBuffer(SENSORPORT sp,PSHORTREAL buffer)
{
  Clean();
  m_pSim->DeleteSensorBuffer(sp,buffer);
  Check();
}

INTEGER CONTROLLER::SensorPortCount(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->SensorPortCount(obj));
}

INTEGER CONTROLLER::ActorPortCount(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->ActorPortCount(obj));
}

STRING CONTROLLER::SensorPortName(SIM3DOBJECT obj,INTEGER n)
{
  Clean();
  return Check(m_pSim->SensorPortName(obj,n));
}

STRING CONTROLLER::ActorPortName(SIM3DOBJECT obj,INTEGER n)
{
  Clean();
  return Check(m_pSim->ActorPortName(obj,n));
}

SIM3DOBJECT CONTROLLER::GetObject(const STRING& name)
{
  Clean();
  return Check(m_pSim->GetObject(name));
}

STRING CONTROLLER::ObjectName(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->ObjectName(obj));
}

STRING CONTROLLER::ObjectRemark(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->ObjectRemark(obj));
}

STRING CONTROLLER::ObjectClass(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->ObjectClass(obj));
}

INTEGER CONTROLLER::SubObjectCount(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->SubObjectCount(obj));
}

SIM3DOBJECT CONTROLLER::SubObject(SIM3DOBJECT obj,INTEGER n)
{
  Clean();
  return Check(m_pSim->SubObject(obj,n));
}

INTEGER CONTROLLER::MacroCount()
{
  Clean();
  return Check(m_pSim->MacroCount());
}

SIM3DOBJECT CONTROLLER::GetMacro(INTEGER n)
{
  Clean();
  return Check(m_pSim->GetMacro(n));
}

SIM3DOBJECT CONTROLLER::GetMacro(const STRING& s)
{
  Clean();
  return Check(m_pSim->GetMacro(s));
}

SIM3DOBJECT CONTROLLER::AddMacroAs(SIM3DOBJECT obj,const STRING& asname)
{
  Clean();
  return Check(m_pSim->AddMacroAs(obj,asname));
}

void CONTROLLER::RemoveObject(SIM3DOBJECT obj)
{
  Clean();
  m_pSim->RemoveObject(obj);
  Check();
}

MOVEMATRIX CONTROLLER::GetLocation(SIM3DOBJECT obj)
{
  Clean();
  return Check(m_pSim->GetLocation(obj));
}

void CONTROLLER::SetLocation(SIM3DOBJECT obj,const MOVEMATRIX& m)
{
  Clean();
  m_pSim->SetLocation(obj,m);
  Check();
}
