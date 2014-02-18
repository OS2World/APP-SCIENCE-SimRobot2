#include "examples\sim3DInc.h"

static jmp_buf buf1,
               buf2;
static char* pStack = 0;
static unsigned nSize;

SIM3DINC_CTRL* SIM3DINC::c = 0;

CONNECT_CONTROLLER_TO_SCENE(SIM3DINC_CTRL,"sim3DInc");

SIM3DINC_CTRL::SIM3DINC_CTRL()
{
  SIM3DINC::c = this;
  m_bFirst = 1;
}

void SIM3DINC_CTRL::SaveStack()
{
  if(!setjmp(buf2))
  {
    nSize = buf1[0].j_esp - buf2[0].j_esp;
    pStack = new char[nSize];
    memcpy(pStack,(void*) buf2[0].j_esp,nSize);
    longjmp(buf1,1);
  }
  else
  {
    memcpy((void*) buf2[0].j_esp,pStack,nSize);
    delete pStack;
    pStack = 0;
  }
}

char* sFileName[] = {"sim3DInc.scn",0};

BOOLEAN SIM3DINC_CTRL::SetActors(STRING&)
{
  m_bCont = 1;
  if(m_bFirst)
  {
    if(!setjmp(buf1))
    {
      m_bFirst = FALSE;
      Main(2,sFileName);
      m_bFirst = TRUE;
      return FALSE;
    }
  }
  else
    longjmp(buf2,1);
  return m_bCont;
}

