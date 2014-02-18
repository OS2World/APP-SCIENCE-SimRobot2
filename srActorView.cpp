#include "pch.h"
#include "srApp.h"
#include "srActorView.h"

DEFINE_DOC_TEMPLATE_CLASS(TDoc,TActorView,ActorTemplate);
ActorTemplate tplActor("Scenes (*.scn)","*.scn",0,"scn",
                       dtAutoDelete |
                       dtFileMustExist |
                       dtHidden |
                       dtOverwritePrompt |
                       dtUpdateDir);

TActorView* TActorView::New(TDoc& doc)
{
  return (TActorView*) tplActor.CreateView(doc);
}

DEFINE_RESPONSE_TABLE1(TActorView,TBaseView)
  EV_CHILD_NOTIFY_ALL_CODES(1,EvSlider),
  EV_VN_ERROR,
  EV_VN_SUCCESS,
  EV_VN_SETACTORS,
  EV_COMMAND(CM_SNAP,CmSnap),
  EV_COMMAND_ENABLE(CM_SNAP,CeSnap),
#ifndef __OS2__
  EV_WM_CTLCOLOR,
#endif
END_RESPONSE_TABLE;

TActorView::TActorView(TDoc& doc,TWindow* parent)
: TBaseView(doc,IDP_ACTOR,parent),
#ifndef __OS2__
  m_BkBrush(SYSCOLOR(COLOR_BTNFACE)),
#endif
  m_Slider(this,1,0,0,0,0)
{
  SetViewMenu(new TMenuDescr(IDM_ACTOR,0,1,0,0,0,0));
  m_sObject = m_pDoc->GetObject();
  m_bSnap = TRUE;
}

void TActorView::WriteLayout(fstream& f)
{
  TBaseView::WriteLayout(f);
  f << '.' << m_sObject << "\n"
    << m_bSnap << " "
    << (m_Slider.GetPosition()-100) << "\n";
}

void TActorView::ReadLayout(fstream& f,int nVersion)
{
  TBaseView::ReadLayout(f,nVersion);
  char buf[200];
  int pos;
  f >> ws;
  f.getline(buf,sizeof(buf));
  f >> m_bSnap >> pos;
  pos += 100;
  m_sObject = &buf[1];
#ifdef __OS2__
  m_Slider.SetPosition(pos);
#else
  m_Slider.SetRange(0,500);
  m_Slider.SetPosition(pos+300);
#endif
  m_Slider.SetRuler(10,m_bSnap);
  if(m_pSim)
    VnSuccess(m_pSim);
}

void TActorView::SetupWindow()
{
  TBaseView::SetupWindow();
  SetFocus();
#ifdef __OS2__
  m_Slider.SetRange(0,200);
  m_Slider.SetPosition(100);
#else
  m_Slider.SetRange(0,500);
  m_Slider.SetPosition(400);
#endif
  m_Slider.SetRuler(10,m_bSnap);
  if(m_pSim && !((TApp*) GetApplication())->m_pStream)
    VnSuccess(m_pSim);
}

BOOL TActorView::SetDocTitle(const char*, int index)
{
  if(index >= 0)
    Parent->SetCaption(STRING(GetViewName()) + STRING(" - ") + m_sObject);
  return TRUE;
}

void TActorView::Paint(TDC& dc,BOOL,TRect&)
{
  if(m_pSim)
  {
    TRect rect = GetClientRect();
    int pos = m_Slider.GetPosition();
    m_Slider.MoveWindow(0,0,rect.right,30,TRUE);
    m_Slider.SetRange(0,200);
#ifndef __OS2__
    if(pos > 200)
      pos -= 300;
    m_Slider.SetPosition(pos ^ 64);
#endif
    m_Slider.SetPosition(pos);
    rect.top = 30;
    dc.FillRect(rect,TBrush(SYSCOLOR(COLOR_BTNFACE)));
    EvSlider(0);
  }
}

#ifndef __OS2__
HBRUSH TActorView::EvCtlColor(HDC hDC, HWND /*hWndChild*/, UINT /*ctlType*/)
{
  ::SetBkColor(hDC,::GetSysColor(COLOR_BTNFACE));
  return m_BkBrush;
}
#endif

void TActorView::EvSlider(UINT)
{
#ifndef __OS2__
  if(GetFocus() == m_Slider.HWindow)
    SetFocus();
#endif
  TRect rect = GetClientRect();
  char c[7];
  itoa(m_Slider.GetPosition()-100,c,10);
  strcat(c,"%");
  TClientDC dc(*this);
  rect.top = 30;
  rect.bottom = rect.top + dc.GetTextExtent("1",1).cy;
  dc.FillRect(rect,TBrush(SYSCOLOR(COLOR_BTNFACE)));
  dc.SetBkMode(TRANSPARENT);
  dc.TextOut(0,rect.top,"-100%");
  dc.TextOut(rect.right - dc.GetTextExtent("100%",5).cx,rect.top,"100%");
  dc.TextOut((rect.right - dc.GetTextExtent(c,strlen(c)).cx) / 2,rect.top,c);
}

BOOL TActorView::VnError(TPoint* pt)
{
  m_Slider.EnableWindow(FALSE);
  return TBaseView::VnError(pt);
}

BOOL TActorView::VnSuccess(SIMULATION* pSim)
{
  m_pSim = pSim;
  m_apActor = m_pSim->GetActorPort(m_sObject);
  if(!m_apActor.PortObject)
  {
    m_pSim = 0;
    m_Slider.EnableWindow(FALSE);
    sim3DTakeLastError();
  }
  else
    m_Slider.EnableWindow(TRUE);
  Invalidate();
  return TRUE;
}

BOOL TActorView::VnSetActors()
{
  if(!Parent->IsIconic())
    m_pSim->SetActorValue(m_apActor,(m_Slider.GetPosition()-100) / 100.0);
  return TRUE;
}

void TActorView::CmSnap()
{
  m_bSnap = 1 - m_bSnap;
  m_Slider.SetRuler(10,m_bSnap);
}

void TActorView::CeSnap(TCommandEnabler& ce)
{
  ce.SetCheck(m_bSnap);
}

