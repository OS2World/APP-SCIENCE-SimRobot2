#include "Controller.h"

class MOTION : public CONTROLLER
{
  public:
    MOTION();
};

CONNECT_CONTROLLER_TO_SCENE(MOTION,"Motion")

class MOTION_VIEW : public VIEW
{
  private:
    SHORTREAL *m_pImage1,
              *m_pImage2,
              *m_pImage3,
              *Ix;
    long m_nStep;
    SENSORPORT m_sp;
    MOTION& m_Ctrl;
  public:  
    MOTION_VIEW(MOTION& m,SENSORPORT sp);
    ~MOTION_VIEW();
    void CalcView();
};

//////////////////////////////////////////////////////////////////////////
// class MotionView

MOTION_VIEW::MOTION_VIEW(MOTION& Ctrl,SENSORPORT sp)
: VIEW("flow",Ctrl.GetSensorDimSize(sp,0)),
  m_Ctrl(Ctrl)
{
  m_sp = sp;
  m_pImage1 = new SHORTREAL [SizeX];
  m_pImage2 = new SHORTREAL [SizeX];
  m_pImage3 = new SHORTREAL [SizeX];
  Ix = new SHORTREAL [SizeX];
  m_nStep = 0;
}

MOTION_VIEW::~MOTION_VIEW()
{
  delete [] m_pImage1;
  delete [] m_pImage2;
  delete [] m_pImage3;
  delete [] Ix;
}

void MOTION_VIEW::CalcView()
{
  SHORTREAL* pTemp = m_pImage1;
  m_pImage1 = m_pImage2;
  m_pImage2 = m_pImage3;
  m_pImage3 = pTemp;

  m_Ctrl.GetSensorValue (m_sp, m_pImage3);

  for (int i = 1; i < SizeX-1; i++)
      m_pImage3[i] = (SHORTREAL) ((m_pImage3[i-1] * 0.5 +
                                   m_pImage3[i] +
                                   m_pImage3[i+1] * 0.5) / 3);
  if (++m_nStep > 2)
  {
    SHORTREAL tmp = 0;
    for (i = 1; i < SizeX - 1; i++)
    {
      Ix[i] = m_pImage2[i+1] - m_pImage2[i-1];
      tmp += (SHORTREAL) abs(Ix[i]);
    }

    SHORTREAL IxCut = (SHORTREAL) (0.2 * tmp / (SizeX-2));
    Value[0] = Value[SizeX-1] = 0;

    for (i = 1; i < SizeX - 1; i++)
    {
      if (abs(Ix[i]) <= IxCut)
        Value[i] = Value[i-1];
      else
      {
        SHORTREAL It = m_pImage3[i] - m_pImage1[i];
        Value[i] = (SHORTREAL) ((1 - (It / Ix[i])) * 0.5);
      }
    }
  }
  else
    for (i = 0; i < SizeX; i++)
      Value[i] = 0;
}

//////////////////////////////////////////////////////////////////////////
// class MOTION

MOTION::MOTION()
{
  SENSORPORT sp;
  if (ObjectRemark(GetObject("")) == "fac")
    sp = GetSensorPort("robbi.line-fac.VALUE");
  else
    sp = GetSensorPort("robbi.line-cam.VALUE");
  AddView(new MOTION_VIEW(*this,sp));
}
