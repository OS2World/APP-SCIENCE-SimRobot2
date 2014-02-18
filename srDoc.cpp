#include "pch.h"
#include <dir.h>
#include <stdio.h>
#include "srApp.h"
#include "srEditor.h"
#include "srTreeView.h"
#include "srObjectView.h"
#include "srSensorView.h"
#include "srActorView.h"

//////////////////////////////////////////////////////////////////////////
// class CONNECTION

CONNECTION* pStart = 0;

CONNECTION::CONNECTION(char* pScene)
{
  m_pScene = pScene;
  m_pNext = pStart;
  pStart = this;
}

//////////////////////////////////////////////////////////////////////////
// TDoc

TDoc::TDoc(TDocument* parent)
: TFileDocument(parent)
{
  m_pSim = 0;
  m_pController = 0;
  m_pTreeView = 0;
  m_sObject = "";
  m_pModule = GetDocManager().GetApplication()->GetMainWindow()->GetModule();
  m_pMainWindow = GetDocManager().GetApplication()->GetMainWindow();
  m_pMessage = ((TApp*) GetDocManager().GetApplication())->m_pMessage;
  m_bRunning = FALSE;
  m_nSteps = 0;
  m_nActorCount = 0;
  m_nDataCount = 0;
  m_bStep = 0;
}

TDoc::~TDoc()
{
  if (m_pController)
  {
    delete m_pController;
    m_pController = 0;
  }
  if (m_pSim)
  {
    delete m_pSim;
    m_pSim = 0;
  }
}

BOOL TDoc::CanClose()
{
  WriteLayout();
  return GetDocManager().FlushDoc(*this);
}

char* TDoc::LayoutFileName()
{
  static char buf[255];
  if(GetDocPath())
  {
    strcpy(buf,GetDocPath());
    for(int i = strlen(buf)-1;
        i >= 0 && buf[i] != '.' && buf[i] != '\\'; i--);
    if(i >= 0)
    {
      if(buf[i] == '.')
        buf[i] = 0;
      strcat(buf,".lyt");
      return buf;
    }
  }
  return 0;
}

int TDoc::GetNumberFromWindow(TWindow* pWnd)
{
  int n = 1;
  for(TView* v = NextView(0); v; v = NextView(v))
  {
    TWindow* pWnd2 = dynamic_cast<TWindow*>(v);
    if(pWnd2->Parent == pWnd)
      return n;
    else
      n++;
  }
  return 0;
}

TView* TDoc::GetViewFromNumber(int n)
{
  TView* v = NextView(0);
  for(int i = 1; i < n; i++)
    v = NextView(v);
  return v;
}

void TDoc::WriteLayout()
{
  char* p = LayoutFileName();
  if(!p)
    return;
  fstream f(p,ios::out);
  f << "SimRobot Layout\n" << 2 << "\n"; // Version 2
  (dynamic_cast<TFrame*>(m_pMainWindow))->WriteLayout(f);
  int nViews = 0;
  for(TView* v = NextView(0); v; v = NextView(v))
    nViews++;
  f << '.' << m_sObject << "\n"
    << m_bRunning << " "
    << m_nActorCount << " "
    << m_nDataCount << "\n";
  v = NextView(0);
  TEditor* pEditor = dynamic_cast<TEditor*> (v);
  pEditor->WriteLayout(f);
  f << (m_pSim ? 1 : 0) << " "
    << nViews-1 << "\n";
  for(v = NextView(v); v; v = NextView(v))
  {
    if(typeid(*v) == typeid(TTreeView))
      f << "Tree\n";
    else if(typeid(*v) == typeid(TObjectView))
      f << "Object\n";
    else if(typeid(*v) == typeid(TSensorView))
      f << "Sensor\n";
    else
      f << "Actor\n";
    ((TBaseView*) v)->WriteLayout(f);
  }
  TWindow* pWnd = GetWindowPtr(m_pMainWindow->GetClientWindow()
#ifndef __OS2__
                  ->GetFirstChild()
#endif
                  ->GetWindow(GW_HWNDFIRST));
  while(pWnd)
  {
    f << GetNumberFromWindow(pWnd) << " ";
    pWnd = GetWindowPtr(pWnd->GetWindow(GW_HWNDNEXT));
  }
  f << "\n";
}

void TDoc::ReadLayout1()
{
  TApp* pApp = (TApp*) GetDocManager().GetApplication();
  char* p = LayoutFileName();
  if(!p)
    return;
  pApp->m_pStream = new fstream(p,ios::in);
  if(pApp->m_pStream->bad())
  {
    delete pApp->m_pStream;
    pApp->m_pStream = 0;
    return;
  }
  char buf[200];
  *pApp->m_pStream >> ws;
  pApp->m_pStream->getline(buf,sizeof(buf));
  if(strcmp(buf,"SimRobot Layout"))
  {
    delete pApp->m_pStream;
    pApp->m_pStream = 0;
    return;
  }
  int nVersion;
  *pApp->m_pStream >> pApp->m_nVersion;
  (dynamic_cast<TFrame*>(m_pMainWindow))
    ->ReadLayout(*pApp->m_pStream,pApp->m_nVersion);
  *pApp->m_pStream >> ws;
   pApp->m_pStream->getline(buf,sizeof(buf));
  m_sObject = &buf[1];
  *pApp->m_pStream >> m_bRunning >> m_nActorCount >> m_nDataCount;
  m_pMainWindow->PostMessage(WM_COMMAND,CM_RESTORE);
}

void TDoc::ReadLayout2(fstream& f,int nVersion)
{
  ((TEditor*) NextView(0))->ReadLayout(f,nVersion);
  char buf[200];
  BOOL bCompile;
  int nViews;
  f >> bCompile >> nViews;
  if(bCompile)
    Compile();
  if(!m_pSim)
    m_bRunning = FALSE;
  for(int i = 0; i < nViews; i++)
  {
    TView* v;
    f >> ws;
    f.getline(buf,sizeof(buf));
    switch(buf[0])
    {
      case 'T':
        TTreeView::New(*this);
        break;
      case 'O':
        TObjectView::New(*this);
        break;
      case 'S':
        TSensorView::New(*this);
        break;
      case 'A':
        TActorView::New(*this);
    }
  }
  int nView;
  f >> nView;
  TView* pView = GetViewFromNumber(nView);
  TWindow* pWndFront = dynamic_cast<TWindow*>(pView);
  pWndFront->Parent->SetFocus();
  if(typeid(*pView) == typeid(TEditor))
    pWndFront->SetFocus();
  for(i = 1; i <= nViews; i++)
  {
    f >> nView;
    TWindow* pWnd = dynamic_cast<TWindow*>(GetViewFromNumber(nView));
    pWnd->Parent->SetWindowPos(*pWndFront->Parent,TRect(),
                               SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    pWndFront = pWnd;
  }
}

#pragma option -Od // To fix a bug in Borland C++ 2.0 for OS2

char ErrorMessage[200];
int ErrorLine,
    ErrorColumn;

void TDoc::Compile()
{
  char sPath[_MAX_PATH],
       sDrive[_MAX_DRIVE],
       sDir[_MAX_DIR],
       sFile[_MAX_FNAME],
       sExt[_MAX_EXT];
  _fullpath(sPath,GetDocPath(),_MAX_PATH);
  SetDocPath(sPath);
  _splitpath(sPath,sDrive,sDir,sFile,sExt);
  setdisk(sDrive[0] - 'A');
  if(strlen(sDir) > 1)
    sDir[strlen(sDir)-1] = 0;
  chdir(sDir);
  char buf[100];
// compile
  *ErrorMessage = 0;
  sim3DTakeLastError();
  sim3DErrorOutputOn();
  m_pSim = new SIMULATION((const char *) GetDocPath());
  sim3DErrorOutputOff();
// check for an error
  if(*ErrorMessage)
  {
    delete m_pSim;
    m_pSim = 0;
  // Write message to status bar and notify views
    m_pMessage->SetText(ErrorMessage);
    NotifyViews(vnError,(long) &TPoint(ErrorColumn,ErrorLine));
    BEEP2;
    sim3DTakeLastError();
    return;
  }
// Search for a suitable controller
  STRING sScene = m_pSim->GetObject("")->GetName();
  CONNECTION* pThis = pStart;
  while (pThis && !(sScene == pThis->m_pScene))
	 pThis = pThis->m_pNext;
  if (pThis)
  {
    CONTROLLER::m_pSim = m_pSim;
    try
    {
      m_pController = pThis->CreateController();
      m_pModule->LoadString(IDS_SUCCESS,buf,sizeof(buf));
      m_pMessage->SetText(buf);
    }
    catch(XCONTROLLER x)
    {
      switch(x.m_xType)
      {
        case XCONTROLLER::InvalidObject:
          m_pModule->LoadString(IDS_INVALIDOBJECT,buf,sizeof(buf));
          break;
        case XCONTROLLER::InvalidSensorPort:
          m_pModule->LoadString(IDS_INVALIDSENSOR,buf,sizeof(buf));
          break;
        case XCONTROLLER::InvalidActorPort:
          m_pModule->LoadString(IDS_INVALIDACTOR,buf,sizeof(buf));
          break;
      }
      STRING sMessage(buf);
      sprintf(buf,sMessage,(const char*) x.m_sName);
      m_pMessage->SetText(buf);
      m_bRunning = FALSE;
      BEEP2;
    }
  }
  else
  {
    m_pModule->LoadString(IDS_NOCONTROLLER,buf,sizeof(buf));
    m_pMessage->SetText(buf);
  }
}

#pragma option -O.

void TDoc::Reset()
{
// Save scene if necessary
  if(IsDirty() || !GetDocPath())
  {
    m_pMainWindow->SendMessage(WM_COMMAND,CM_FILESAVE);
    if(IsDirty() || !GetDocPath())
      return;
  }
// Delete controller and simulation objects
  if (m_pController)
  {
    delete m_pController;
    m_pController = 0;
  }
  if (m_pSim)
  {
    delete m_pSim;
    m_pSim = 0;
  }
  m_nSteps = 0;
  Compile();
  if(m_pSim)
  {
    NotifyViews(vnSuccess,(long)m_pSim);
    if(!NextView(NextView(0)))
      ViewTree();
  }
  sim3DTakeLastError();
}

void TDoc::Start()
{
  if(m_bRunning)
    m_bRunning = FALSE;
  else
  {
    if(IsDirty() || !GetDocPath() || !m_pSim)
      Reset();
    if(!IsDirty() && GetDocPath() && m_pSim)
      m_bRunning = TRUE;
  }
}

void TDoc::Step()
{
  if (IsDirty() || !GetDocPath() || !m_pSim)
    Reset();
  if (!IsDirty() && GetDocPath() && m_pSim)
    m_bStep = 1;
}

void TDoc::DoStep()
{
  sim3DTakeLastError();
  try
  {
    STRING sMessage;
    if(m_pController && !m_pController->SetActors(sMessage))
      m_bRunning = FALSE;
    NotifyViews(vnSetActors);
    m_pSim->DoTimeStep();
    m_nSteps++;
    NotifyViews(vnRepaint);
    m_pMessage->SetText((const char *) sMessage);
  }
  catch(XCONTROLLER& x)
  {
    char buf[100];
    switch(x.m_xType)
    {
      case XCONTROLLER::InvalidObject:
        m_pModule->LoadString(IDS_INVALIDOBJECT,buf,sizeof(buf));
        break;
      case XCONTROLLER::InvalidSensorPort:
        m_pModule->LoadString(IDS_INVALIDSENSOR,buf,sizeof(buf));
        break;
      case XCONTROLLER::InvalidActorPort:
        m_pModule->LoadString(IDS_INVALIDACTOR,buf,sizeof(buf));
        break;
    }
    STRING sMessage(buf);
    sprintf(buf,sMessage,(const char*) x.m_sName);
    m_pMessage->SetText(buf);
    delete m_pController;
    m_pController = 0;
    m_bRunning = FALSE;
    BEEP2;
  }
}

BOOL TDoc::Idle()
{
  TApp* pApp = (TApp*) GetDocManager().GetApplication();
  if((m_bStep || m_bRunning) && !pApp->m_pStream)
  {
    m_bStep = 0;
    DoStep();
  }
  return m_bRunning;
}

void TDoc::ViewTree()
{
  if(!m_pTreeView)
    TTreeView::New(*this);
  else
  {
    if(m_pTreeView->Parent->IsIconic())
      m_pTreeView->Parent->Show(SW_RESTORE);
    m_pTreeView->SetFocus();
  }
}

void TDoc::OnMouseClick(const CLICKINFO& ci)
{
  if(m_pController)
    m_pController->OnMouseClick(ci);
}

