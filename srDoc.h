#ifndef __SRDOC_H__
#define __SRDOC_H__

#include "sim3D.h"
#include "SimRobot.h"
#include "Controller.h"

class _DOCVIEWCLASS TEditor;
class _DOCVIEWCLASS TTreeView;

class _DOCVIEWCLASS TDoc : public TFileDocument
{
  private:
    SIMULATION* m_pSim;
    CONTROLLER* m_pController;
    STRING m_sObject;
    TModule* m_pModule;
    TFrameWindow* m_pMainWindow;
    TTextGadget* m_pMessage;
    BOOL m_bRunning,
         m_bStep;
    void Compile();
    void DoStep();
    char* LayoutFileName();
    int GetNumberFromWindow(TWindow* pWnd);
    TView* GetViewFromNumber(int n);
  public:
    TTreeView* m_pTreeView;
    long m_nSteps;
    int m_nActorCount,
        m_nDataCount;
    TDoc(TDocument* parent = 0);
    ~TDoc();
    BOOL CanClose();
    void WriteLayout();
    void ReadLayout1();
    void ReadLayout2(fstream& f,int nVersion);
    void Reset();
    void Start();
    void Step();
    BOOL IsRunning() {return m_bRunning;}
    void ViewTree();
    void OnMouseClick(const CLICKINFO&);
    BOOL Idle();
    SIMULATION* GetSimulation() {return m_pSim;}
    STRING& GetObject() {return m_sObject;}
};

const int vnError = vnCustomBase+0;
const int vnSuccess = vnCustomBase+1;
const int vnSetActors = vnCustomBase+2;
const int vnRepaint = vnCustomBase+3;
NOTIFY_SIG(vnError,TPoint*)
NOTIFY_SIG(vnSuccess,SIMULATION*)
NOTIFY_SIG(vnSetActors,void)
NOTIFY_SIG(vnRepaint,void)
#define EV_VN_ERROR VN_DEFINE(vnError,VnError,pointer)
#define EV_VN_SUCCESS VN_DEFINE(vnSuccess,VnSuccess,pointer)
#define EV_VN_SETACTORS VN_DEFINE(vnSetActors,VnSetActors,void)
#define EV_VN_REPAINT VN_DEFINE(vnRepaint,VnRepaint,void)

#endif

