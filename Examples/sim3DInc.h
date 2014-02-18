#ifndef __SIM3DINC_H__
#define __SIM3DINC_H__

#ifdef __WIN16__
#error sim3DInc does not work under Win16 (use Win32).
#endif

#include <setjmp.h>
#include "Controller.h"

// SIM3DINC_CTRL is for internal use only

class SIM3DINC;

class SIM3DINC_CTRL : public CONTROLLER
{
  friend SIM3DINC;
  BOOLEAN m_bFirst,
          m_bCont;
  void SaveStack();
  void Stop() {m_bCont = 0;}
public:
  SIM3DINC_CTRL();
  ~SIM3DINC_CTRL() {}
  BOOLEAN SetActors(STRING&);
};

// Use this class under the name SIMULATION (see #define below)
// It supports nearly all of SIMULATION's member functions
// and has one additional function (Stop)
class SIM3DINC
{
  static SIM3DINC_CTRL* c;
  friend SIM3DINC_CTRL;
public:
  SIM3DINC (const STRING&) {}
  BOOLEAN WasOk() {return TRUE;}
  ACTORPORT GetActorPort (const STRING& portname) {return c->GetActorPort(portname);}
  SENSORPORT GetSensorPort (const STRING& portname) {return c->GetSensorPort(portname);}
  INTEGER GetSensorDim (SENSORPORT sp) {return c->GetSensorDim(sp);}
  INTEGER GetSensorDimSize (SENSORPORT sp, INTEGER d) {return c->GetSensorDimSize(sp,d);}
  void GetSensorValue (SENSORPORT sp, PSHORTREAL value) {c->GetSensorValue(sp,value);}
  void SetActorValue (ACTORPORT ap, REAL value) {c->SetActorValue(ap,value);}
  PSHORTREAL NewSensorBuffer (SENSORPORT sp) {return c->NewSensorBuffer(sp);}
  void DeleteSensorBuffer (SENSORPORT sp, PSHORTREAL buffer) {c->DeleteSensorBuffer(sp,buffer);}
  INTEGER SensorPortCount (SIM3DOBJECT obj) {return c->SensorPortCount(obj);}
  INTEGER ActorPortCount (SIM3DOBJECT obj) {return c->ActorPortCount(obj);}
  STRING SensorPortName (SIM3DOBJECT obj, INTEGER n) {return c->SensorPortName(obj,n);}
  STRING ActorPortName (SIM3DOBJECT obj, INTEGER n) {return c->ActorPortName(obj,n);}
  void DoTimeStep () {c->SaveStack();}
  SIM3DOBJECT GetObject (const STRING& name) {return c->GetObject(name);}
  STRING ObjectName (SIM3DOBJECT obj) {return c->ObjectName(obj);}
  STRING ObjectRemark (SIM3DOBJECT obj) {return c->ObjectRemark(obj);}
  STRING ObjectClass (SIM3DOBJECT obj) {return c->ObjectClass(obj);}
  INTEGER SubObjectCount (SIM3DOBJECT obj) {return c->SubObjectCount(obj);}
  SIM3DOBJECT SubObject (SIM3DOBJECT obj, INTEGER n) {return c->SubObject(obj,n);}
  INTEGER MacroCount () {return c->MacroCount();}
  SIM3DOBJECT GetMacro (INTEGER n) {return c->GetMacro(n);}
  SIM3DOBJECT GetMacro (const STRING& name) {return c->GetMacro(name);}
  SIM3DOBJECT AddMacroAs (SIM3DOBJECT obj, const STRING& asname) {return c->AddMacroAs(obj,asname);}
  void RemoveObject (SIM3DOBJECT obj) {c->RemoveObject(obj);}
  MOVEMATRIX GetLocation (SIM3DOBJECT obj) {return c->GetLocation(obj);}
  void SetLocation (SIM3DOBJECT obj, const MOVEMATRIX& m) {c->SetLocation(obj,m);}
// This functions stops the simulation. The user can continue it by selecting
// the menu item "Simulation|Start"
  void Stop() {c->Stop();c->SaveStack();}
};

// This is your controller's main function. Note the capital "M" of "Main"
int Main(int argc,char* argv[]);

#define SIMULATION SIM3DINC

#endif
