#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "sim3D.h"
#include "sim3DSensors.h"

//////////////////////////////////////////////////////////////////////////
// class VIEW

class VIEW : public SIM3DMultiSensor
{
  public:
    VIEW(const STRING& sName,INTEGER xSize = 1,INTEGER ySize = 1,
         INTEGER numClasses = 1);
    virtual ~VIEW() {};
    virtual void CalcView() = 0;
  private:
    STRING GetClass () {return "VIEW";}
    SIM3DElement* Copy() {return 0;}
    void Draw (GRAPHICPORT*, const MOVEMATRIX&, INTEGER, REAL) {};
    void CalcSensor();
};

//////////////////////////////////////////////////////////////////////////
// class XCONTROLLER

class XCONTROLLER
{
  public:
    enum TYPE
    {
      InvalidObject,
      InvalidSensorPort,
      InvalidActorPort
    };
    TYPE m_xType;
    STRING m_sName;
    XCONTROLLER(TYPE xType,const char* sName = 0) : m_sName(sName)
      {m_xType = xType;}
};

//////////////////////////////////////////////////////////////////////////
// class CLICKINFO

class CLICKINFO 
{
  private:
    SIM3DOBJECT m_obj;
    VECTOR m_v;
    MOVEMATRIX m_m;
    double m_dZoom,
           m_dDist;
  public:
    CLICKINFO(SIM3DOBJECT obj,const VECTOR& v,const MOVEMATRIX& m,double dZoom,double dDist)
    : m_v(v),m_m(m) {m_obj = obj; m_dZoom = dZoom,m_dDist = dDist;}
    double DistanceTo(SIM3DOBJECT obj) const;
};

//////////////////////////////////////////////////////////////////////////
// class CONTROLLER

class CONTROLLER
{
  public:
    virtual ~CONTROLLER() {}
    virtual BOOLEAN SetActors(STRING& sMessage) {return TRUE;}
    virtual void OnMouseClick(const CLICKINFO&) {}
    SENSORPORT AddView(VIEW* pView);
    ACTORPORT   GetActorPort     (const STRING& portname);
    SENSORPORT  GetSensorPort    (const STRING& portname);
    INTEGER     GetSensorDim     (SENSORPORT sp);
    INTEGER     GetSensorDimSize (SENSORPORT sp, INTEGER d);
    void        GetSensorValue   (SENSORPORT sp, PSHORTREAL value);
    void        SetActorValue    (ACTORPORT ap, REAL value);
    PSHORTREAL  NewSensorBuffer  (SENSORPORT sp);
    void        DeleteSensorBuffer (SENSORPORT sp, PSHORTREAL buffer);
    INTEGER     SensorPortCount  (SIM3DOBJECT obj);
    INTEGER     ActorPortCount   (SIM3DOBJECT obj);
    STRING      SensorPortName   (SIM3DOBJECT obj, INTEGER n);
    STRING      ActorPortName    (SIM3DOBJECT obj, INTEGER n);
    SIM3DOBJECT GetObject        (const STRING& name);
    STRING      ObjectName       (SIM3DOBJECT obj);
    STRING      ObjectRemark     (SIM3DOBJECT obj);
    STRING      ObjectClass      (SIM3DOBJECT obj);
    INTEGER     SubObjectCount   (SIM3DOBJECT obj);
    SIM3DOBJECT SubObject        (SIM3DOBJECT obj, INTEGER n);
    INTEGER     MacroCount       ();
    SIM3DOBJECT GetMacro         (INTEGER n);
    SIM3DOBJECT GetMacro         (const STRING&);
    SIM3DOBJECT AddMacroAs       (SIM3DOBJECT obj, const STRING& asname);
    void        RemoveObject     (SIM3DOBJECT obj);
    MOVEMATRIX  GetLocation      (SIM3DOBJECT obj);
    void        SetLocation      (SIM3DOBJECT obj, const MOVEMATRIX& m);
  private:
    static void* m_pLastAllocated;
    void Clean();
    void Check();
    INTEGER Check(INTEGER i);
    STRING Check(const STRING& s);
    SIM3DOBJECT Check(SIM3DOBJECT s);
    PSHORTREAL Check(PSHORTREAL p);
    MOVEMATRIX Check(const MOVEMATRIX& m);
  public:
    static SIMULATION* m_pSim;
    static void* operator new(size_t nSize);
    static void CleanUp();
};

//////////////////////////////////////////////////////////////////////////
// class CONNECTION

class CONNECTION
{
  public:
    char* m_pScene;
    CONNECTION* m_pNext;
    CONNECTION(char* pScene);
    virtual CONTROLLER* CreateController() = 0;
};

template <class T> class CONNECTIONTEMPLATE : public CONNECTION
{
  public:
    CONNECTIONTEMPLATE(char* pScene) : CONNECTION(pScene) {}
    CONTROLLER* CreateController() {return new T;}
};

#define CONNECT_CONTROLLER_TO_SCENE(ClassName,pScene) \
  CONNECTIONTEMPLATE<ClassName> dummy##ClassName(pScene);

#endif
