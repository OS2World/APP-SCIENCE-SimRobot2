#ifndef __SRACTORVIEW_H__
#define __SRACTORVIEW_H__

#include "srBaseView.h"

class _DOCVIEWCLASS TActorView : public TBaseView
{
  private:
    THSlider m_Slider;
    STRING m_sObject;
    ACTORPORT m_apActor;
    BOOL m_bSnap;
#ifndef __OS2__
    TBrush m_BkBrush;
#endif
  public:
    TActorView(TDoc& doc,TWindow* parent = 0);
    static TActorView* New(TDoc&);
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    void Paint(TDC& dc, BOOL, TRect&);
    static const char far* StaticName() {return "Actor";}
    const char far* GetViewName(){return StaticName();}
  protected:
    void SetupWindow();
    BOOL SetDocTitle(const char*, int);
    void EvSlider(UINT);
    BOOL VnError(TPoint*);
    BOOL VnSuccess(SIMULATION*);
    BOOL VnSetActors();
    void CmSnap();
    void CeSnap(TCommandEnabler&);
#ifndef __OS2__
    HBRUSH EvCtlColor(HDC hDC,HWND,UINT);
#endif
  DECLARE_RESPONSE_TABLE(TActorView);
};

#endif

