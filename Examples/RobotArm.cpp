#include "Controller.h"

class ROBOTARM : public CONTROLLER
{
  ACTORPORT  m_apMove[3];
  SENSORPORT m_spExecuted[3];
  SHORTREAL  m_srDir[3];
public:
  ROBOTARM();
  BOOLEAN SetActors(STRING& sMessage);
};

CONNECT_CONTROLLER_TO_SCENE(ROBOTARM,"RobotArm");

ROBOTARM::ROBOTARM()
{
  m_srDir[0] = m_srDir[1] = m_srDir[2] = 1;
  STRING sJoint[3] = {"joint1","joint2","joint3"};
  for(INTEGER i = 0; i < 3; i++)
  {
    m_apMove[i] = GetActorPort(sJoint[i] + STRING(".MOVE"));
    m_spExecuted[i] = GetSensorPort(sJoint[i] + STRING(".EXECUTED"));
  }
}

BOOLEAN ROBOTARM::SetActors(STRING& sMessage)
{
  SHORTREAL srExecuted;
  for(INTEGER i = 0; i < 3; i++)
  {
    GetSensorValue(m_spExecuted[i],&srExecuted);
    if(!srExecuted)
    {
      m_srDir[i] = -m_srDir[i];
      sMessage = sMessage + STRING("boing... ");
	 }
    SetActorValue(m_apMove[i],m_srDir[i]);
  }
  return TRUE;
}

