#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SimRobot.h"
#include "srEditor.h"
#include "srTreeView.h"
#include "srObjectView.h"
#include "srSensorView.h"
#include "srActorView.h"
#include "srApp.h"

static char sPath[_MAX_PATH];
static char sHelp[_MAX_PATH];

//////////////////////////////////////////////////////////////////////////
// TStepsGadget

class _USERCLASS TStepsGadget : public TTextGadget
{
  private:
    long m_nLast;
  public:
    TStepsGadget()
    : TTextGadget(CM_STEPS,TGadget::Embossed,TTextGadget::Right,6)
      {m_nLast = -1;}
    void CommandEnable();
};

void TStepsGadget::CommandEnable()
{
  TDoc* pDoc = (TDoc*)
  Window->GetApplication()->GetDocManager()->GetCurrentDoc();
  long nThis = pDoc ? pDoc->m_nSteps : -1;
  if(nThis != m_nLast)
  {
    m_nLast = nThis;
    char buf[20];
    SetText(nThis == -1 ? "" : itoa(nThis,buf,10));
  }
}

//////////////////////////////////////////////////////////////////////////
// TFrame

DEFINE_RESPONSE_TABLE1(TFrame, TDecoratedMDIFrame)
  EV_WM_INITMENUPOPUP,
#ifdef __OS2__
  EV_WM_MENUSELECT,
  EV_WM_SIZE,
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 0,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 1,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 2,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 3,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 4,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 5,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 6,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 7,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 8,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 9,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 10,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 11,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 12,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 13,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 14,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 15,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 16,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 17,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 18,CmActivateChild),
  EV_COMMAND_AND_ID(IDW_FIRSTMDICHILD + 19,CmActivateChild),
#endif
END_RESPONSE_TABLE;

TFrame::TFrame(TMDIClient& clientWnd)
#ifdef __OS2__
: TDecoratedMDIFrame("SimRobot",IDM_APP,clientWnd,TRUE),
  m_hWndHelp(0)
#else
: TDecoratedMDIFrame("SimRobot",0,clientWnd,TRUE)
#endif
{
  Attr.AccelTable = IDM_APP;
}

#ifdef __OS2__
TFrame::~TFrame()
{
  if(m_hWndHelp)
     WinDestroyHelpInstance(m_hWndHelp);
}
#endif

void TFrame::SetupWindow()
{
  char buf[300];
  TDecoratedMDIFrame::SetupWindow();
#ifdef __OS2__
  HELPINIT hini;
  memset(&hini,0,sizeof(HELPINIT));
  hini.cb = sizeof(HELPINIT);
  hini.pszHelpWindowTitle = "SimRobot Help";
  hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
  hini.pszHelpLibraryName = sHelp;
  m_hWndHelp = WinCreateHelpInstance((HAB) GetModule()->GetInstance(),
                                     &hini);
  if(m_hWndHelp)
    WinAssociateHelpInstance(m_hWndHelp,*this);
  else
  {
    sprintf(buf,"Help file '%s' not found!",sHelp);
    MessageBox(buf);
  }
#endif // __OS2__
  if(sPath[0])
  {
    FILE* f = fopen(sPath,"r");
    if(f)
    {
      fclose(f);
      TDocTemplate* tpl = GetApplication()->GetDocManager()
                          ->MatchTemplate(sPath);
      if(tpl)
        tpl->CreateDoc(sPath);
    }
    else
    {
      sprintf(buf,"File '%s' not found!",sPath);
      MessageBox(buf);
    }
  }
}

void TFrame::ShowHelp(char* pTopic)
{
  char buf[40];
  for(char* p = pTopic; *p; p++)
    *p = toupper(*p);
  for(int i = 2001; GetModule()->LoadString(i,buf,sizeof(buf)); i++)
    for(int j = 0; GetModule()->LoadString(i+j,buf,sizeof(buf)); j += 200)
      if(!strcmp(pTopic,buf))
      {
#ifdef __OS2__
        WinSendMsg(m_hWndHelp,HM_DISPLAY_HELP,MPFROMLONG(MAKELONG(i,NULL)),
                   MPFROMSHORT(HM_RESOURCEID));
#else
        WinHelp(sHelp,HELP_CONTEXT,i);
#endif
        return;
      }
  BEEP1;
}

void TFrame::EvInitMenuPopup(HMENU hPopupMenu,UINT index,BOOL sysMenu)
{
  if(hPopupMenu == GetMenu())
    return;
  TDecoratedMDIFrame::EvInitMenuPopup(hPopupMenu,index,sysMenu);
  TMenu menu(hPopupMenu);
#ifdef __OS2__
  for(int i = 0; i < menu.GetMenuItemCount(); i++)
    if(menu.GetMenuItemID(i) == CM_CLOSECHILDREN)
#else
  BOOL bExit = FALSE;
  for(int i = 0; i < menu.GetMenuItemCount(); i++)
    if(menu.GetMenuItemID(i) == CM_EXIT)
      bExit = TRUE;
    else if(menu.GetMenuItemID(i) == CM_CLOSECHILDREN && bExit)
#endif
    {
      for(int j = menu.GetMenuItemCount()-1; j > i; j--)
        menu.DeleteMenu(j,MF_BYPOSITION);
      TDocument* pDoc = GetApplication()->GetDocManager()->GetCurrentDoc();
      if(pDoc)
      {
#ifdef __OS2__
        MENUITEM mi;
        mi.iPosition = i+1;
        mi.afStyle = MF_SEPARATOR;
        mi.afAttribute = 0;
        mi.id = IDW_FIRSTMDICHILD+20;
        mi.hwndSubMenu = 0;
        mi.hItem = 0;
        ::WinSendMsg(menu,MM_INSERTITEM,(MPARAM)&mi,(MPARAM) 0);
        STRING s0("~");
#else
        menu.AppendMenu(MF_SEPARATOR);
        STRING s0("&");
#endif
        j = 0;
        for(TView* pView = pDoc->NextView(0); pView;
            pView = pDoc->NextView(pView), j++)
        {
          TWindow* pWnd = dynamic_cast<TWindow*>(pView);
          char buf[20];
	  STRING s = s0 + STRING(itoa(j+1,buf,10)) + STRING(" ")
                     + STRING(pWnd->Parent->Title);
          menu.AppendMenu(MF_ENABLED |
            (pWnd->Parent == GetWindowPtr(GetClientWindow()->GetTopWindow())
             ? MF_CHECKED : 0),
            IDW_FIRSTMDICHILD+j,s);
        }
      }
      break;
    }
  for(i = 0; i < menu.GetMenuItemCount(); i++)
    if(menu.GetMenuState(i,MF_BYPOSITION) &0x10)
#ifdef __OS2__
      TDecoratedMDIFrame::EvInitMenuPopup(menu.GetSubMenu(i),0,FALSE);
#else
    {
      menu.EnableMenuItem(i,MF_BYPOSITION | MF_ENABLED);
      menu.CheckMenuItem(i,MF_BYPOSITION | MF_UNCHECKED);
    }
#endif
}

#ifdef __OS2__
void
TFrame::EvMenuSelect(UINT menuItemId, UINT, HMENU hMenu)
{
  TDecoratedMDIFrame::EvMenuSelect(menuItemId,hMenu ? 0 : 0xffff, hMenu);
}

void TFrame::ScreenToClient(TPoint& point) const
{
  SWP swp;
  ::WinQueryWindowPos(HWND_DESKTOP, &swp);
  point.y = swp.cy - point.y;
  ::WinMapWindowPoints(HWND_DESKTOP, HWindow, (POINTL*)&point, 1);
  ::WinQueryWindowPos(*this, &swp);
  point.y = swp.cy - point.y;
}

void EnableAllChilds(TWindow* pWnd,void *pState)
{
  pWnd->ShowWindow(*(int*) pState);
}

void TFrame::EvSize(UINT sizeType,TSize& size)
{
  TDecoratedMDIFrame::EvSize(sizeType,size);
  int nState = sizeType == SIZE_MINIMIZED ? SW_HIDE : SW_SHOW;
  ForEach(EnableAllChilds,(void*) &nState);
}

void TFrame::CmActivateChild(WPARAM id)
{
  id -= IDW_FIRSTMDICHILD;
  TDocument* pDoc = GetApplication()->GetDocManager()->GetCurrentDoc();
  if(pDoc)
  {
    for(TView* pView = pDoc->NextView(0); id;
        pView = pDoc->NextView(pView), id--);
    TWindow* pWnd = dynamic_cast<TWindow*>(pView);
    if(pWnd->Parent->IsIconic())
       pWnd->Parent->Show(SW_RESTORE);
    pWnd->SetFocus();
  }
}
#endif

void TFrame::WriteLayout(fstream& f)
{
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(&wp);
#ifdef __OS2__
  SWP swp;
  ::WinQueryWindowPos(HWND_DESKTOP,&swp);
  int h = swp.cy;
  if(wp.showCmd == SW_SHOWNORMAL)
  {
    ::WinQueryWindowPos(*this,&swp);
    wp.rcNormalPosition.left = swp.x;
    wp.rcNormalPosition.right = swp.x + swp.cx;
    wp.rcNormalPosition.top = h - swp.y - swp.cy;
    wp.rcNormalPosition.bottom = h - swp.y;
  }
  else
  {
    int top = wp.rcNormalPosition.top;
    wp.rcNormalPosition.top = h - wp.rcNormalPosition.bottom;
    wp.rcNormalPosition.bottom = h - top;
  }
#endif
  f << (wp.showCmd == SW_SHOWNORMAL ? 0
        : (wp.showCmd == SW_SHOWMAXIMIZED ? 1 : 2)) << " "
    << wp.rcNormalPosition.left << " "
    << wp.rcNormalPosition.top << " "
    << wp.rcNormalPosition.right - wp.rcNormalPosition.left << " "
    << wp.rcNormalPosition.bottom - wp.rcNormalPosition.top << "\n";
}

void TFrame::ReadLayout(fstream& f,int)
{
  int nShowCmd,x,y,w,h;
    f >> nShowCmd >> x >> y >> w >> h;
    SetWindowPos(0,x,y,w,h,SWP_NOZORDER);
    Show(nShowCmd ? (nShowCmd == 1 ? SW_SHOWMAXIMIZED : SW_SHOWMINIMIZED)
                                   : SW_SHOWNORMAL);
}

//////////////////////////////////////////////////////////////////////////
// TClient

class _USERCLASS TClient : public TMDIClient
{
  private:
    TMenu m_Menu;
    TPopupMenu m_PopupMenu;
  public:
    TClient();
  protected:
    void EvRButtonDown(UINT,TPoint& point);
#ifdef __OS2__
    void EvDropFiles(TDropInfo dropInfo);
#endif
  DECLARE_RESPONSE_TABLE(TClient);
};

DEFINE_RESPONSE_TABLE1(TClient,TMDIClient)
  EV_WM_RBUTTONDOWN,
#ifdef __OS2__
  EV_WM_DROPFILES,
#endif
END_RESPONSE_TABLE;

TClient::TClient()
: TMDIClient(),
  m_Menu(*GetModule(),IDP_APP),
#ifdef __OS2__
  m_PopupMenu(m_Menu)
#else
  m_PopupMenu(m_Menu.GetSubMenu(0))
#endif
{
  Attr.Style &= ~(WS_VSCROLL | WS_HSCROLL);
}

void TClient::EvRButtonDown(UINT,TPoint& point)
{
  TPoint lp = point;
  ClientToScreen(lp);
#ifndef __OS2__
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,0,*Parent);
#else
  (dynamic_cast<TFrame*>(Parent))->EvInitMenuPopup(m_PopupMenu,0,FALSE);
  (dynamic_cast<TFrame*>(Parent))->ScreenToClient(lp);
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,*Parent);
}

void TClient::EvDropFiles(TDropInfo dropInfo)
{
  (dynamic_cast<TApp*>(GetApplication()))->EvDropFiles(dropInfo);
#endif
}

//////////////////////////////////////////////////////////////////////////
// TMDIBaseChild

TMDIBaseChild::TMDIBaseChild(TMDIClient& parent,const char far* title,
                             TWindow* clientWnd,BOOL shrinkToClient,
			     TModule* module)
: TMDIChild(parent,title,clientWnd,shrinkToClient,module)
{
}

void TMDIBaseChild::WriteLayout(fstream& f)
{
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(&wp);
#ifdef __OS2__
  SWP swp;
  ::WinQueryWindowPos(*GetApplication()->GetMainWindow()->GetClientWindow(),
                      &swp);
  int h = swp.cy;
  if(wp.showCmd == SW_SHOWNORMAL)
  {
    ::WinQueryWindowPos(*this,&swp);
    wp.rcNormalPosition.left = swp.x;
    wp.rcNormalPosition.right = swp.x + swp.cx;
    wp.rcNormalPosition.top = h - swp.y - swp.cy;
    wp.rcNormalPosition.bottom = h - swp.y;
  }
  else
  {
    int top = wp.rcNormalPosition.top;
    wp.rcNormalPosition.top = h - wp.rcNormalPosition.bottom;
    wp.rcNormalPosition.bottom = h - top;
  }
#endif
  f << (wp.showCmd == SW_SHOWNORMAL ? 0
        : (wp.showCmd == SW_SHOWMAXIMIZED ? 1 : 2)) << " "
    << wp.rcNormalPosition.left << " "
    << wp.rcNormalPosition.top << " "
    << wp.rcNormalPosition.right - wp.rcNormalPosition.left << " "
    << wp.rcNormalPosition.bottom - wp.rcNormalPosition.top << "\n";
}

//////////////////////////////////////////////////////////////////////////
// TMDIEditorChild

class _USERCLASS TMDIEditorChild : public TMDIBaseChild
{
  private:
    BOOL m_bDelete;
  public:
    TMDIEditorChild(TMDIClient&     parent,
                    const char far* title = 0,
                    TWindow*        clientWnd = 0,
                    BOOL            shrinkToClient = FALSE,
                    TModule*        module = 0)
    : TMDIBaseChild(parent,title,clientWnd,shrinkToClient,module)
      {m_bDelete = FALSE;}
  protected:
    void EvParentNotify(UINT event,UINT childHandleOrX,UINT childIDOrY);
    void EvClose();
    void EvSetFocus(HWND);
  DECLARE_RESPONSE_TABLE(TMDIEditorChild);
};

DEFINE_RESPONSE_TABLE1(TMDIEditorChild,TMDIChild)
  EV_WM_PARENTNOTIFY,
  EV_WM_CLOSE,
END_RESPONSE_TABLE;

void TMDIEditorChild::EvParentNotify(UINT event,
                                     UINT childHandleOrX,UINT childIDOrY)
{
  if(event == WM_DESTROY && ClientWnd &&
     ClientWnd->HWindow == HWND(childHandleOrX))
    m_bDelete = TRUE;
  TMDIBaseChild::EvParentNotify(event,childHandleOrX,childIDOrY);
}

void TMDIEditorChild::EvClose()
{
  if(m_bDelete)
    TMDIChild::EvClose();
  else
    GetApplication()->GetMainWindow()->PostMessage(WM_COMMAND,CM_FILECLOSE);
}

//////////////////////////////////////////////////////////////////////////
// TMDIObjectChild

class _USERCLASS TMDIObjectChild : public TMDIBaseChild
{
  private:
    TObjectView* m_pObjectView;
  public:
    TMDIObjectChild(TMDIClient&     parent,
                    const char far* title = 0,
                    TWindow*        clientWnd = 0,
                    BOOL            shrinkToClient = FALSE,
                    TModule*        module = 0);
  protected:
    void EvHScroll(UINT nSBCode,UINT nPos,HWND);
    void EvVScroll(UINT nSBCode, UINT nPos,HWND);
    void OnScroll(UINT nBar, UINT nSBCode, UINT nPos);
  DECLARE_RESPONSE_TABLE(TMDIObjectChild);
};

DEFINE_RESPONSE_TABLE1(TMDIObjectChild,TMDIChild)
  EV_WM_HSCROLL,
  EV_WM_VSCROLL,
END_RESPONSE_TABLE;

TMDIObjectChild::TMDIObjectChild(TMDIClient& parent,const char far* title,
				 TWindow* clientWnd,BOOL shrinkToClient,
                                 TModule* module)
: TMDIBaseChild(parent,title,clientWnd,shrinkToClient,module)
{
  m_pObjectView = (TObjectView*) clientWnd;
}

void TMDIObjectChild::EvHScroll(UINT nSBCode,UINT nPos,HWND)
{
  OnScroll(SB_HORZ, nSBCode, nPos);
}

void TMDIObjectChild::EvVScroll(UINT nSBCode, UINT nPos,HWND)
{
  OnScroll(SB_VERT, nSBCode, nPos);
}

void TMDIObjectChild::OnScroll(UINT nBar, UINT nSBCode, UINT nPos)
{
  int zOrig, z;   // z = x or y depending on 'nBar'
  int zMin, zMax;
  zOrig = z = GetScrollPos(nBar);
  GetScrollRange(nBar,zMin,zMax);
  switch (nSBCode)
  {
    case SB_TOP:
      z = 0;
      break;
    case SB_BOTTOM:
      z = zMax;
      break;
    case SB_LINEUP:
      z -= 10;
      break;
    case SB_LINEDOWN:
      z += 10;
      break;
    case SB_PAGEUP:
      z -= 90;
      break;
    case SB_PAGEDOWN:
      z += 90;
      break;
    case SB_THUMBTRACK:
      if(!m_pObjectView->m_bTracking)
        return;
      z = nPos;
      break;
    case SB_THUMBPOSITION:
#ifdef __OS2__
      m_pObjectView->Invalidate();
#else
      z = nPos;
      break;
#endif
    default:        // ignore other notifications
      return;
  }
  if (z < 0)
    z = 0;
  else if (z > zMax)
    z = zMax;
  if (z != zOrig)
  {
    SetScrollPos(nBar,z);
    m_pObjectView->Invalidate();
  }
}

//////////////////////////////////////////////////////////////////////////
// TMDIActorChild

class _USERCLASS TMDIActorChild : public TMDIBaseChild
{
  public:
    TMDIActorChild(TMDIClient&     parent,
                   const char far* title = 0,
                   TWindow*        clientWnd = 0,
                   BOOL            shrinkToClient = FALSE,
                   TModule*        module = 0)
    : TMDIBaseChild(parent,title,clientWnd,shrinkToClient,module) {}
  protected:
    void EvSize(UINT sizeType,TSize& size);
  DECLARE_RESPONSE_TABLE(TMDIActorChild);
};

DEFINE_RESPONSE_TABLE1(TMDIActorChild,TMDIChild)
  EV_WM_SIZE,
END_RESPONSE_TABLE;

void TMDIActorChild::EvSize(UINT sizeType,TSize& size)
{
  TMDIBaseChild::EvSize(sizeType,size);
  ClientWnd->Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// TApp

DEFINE_RESPONSE_TABLE1(TApp, TApplication)
  EV_OWLVIEW(dnCreate,EvNewView),
  EV_WM_DROPFILES,
  EV_OWLVIEW(dnClose,EvCloseView),
  EV_COMMAND(CM_RESTORE,CmRestore),
  EV_COMMAND(CM_START,CmStart),
  EV_COMMAND_ENABLE(CM_START,CeStart),
  EV_COMMAND(CM_STEP,CmStep),
  EV_COMMAND_ENABLE(CM_STEP,CeStep),
  EV_COMMAND(CM_RESET,CmReset),
  EV_COMMAND_ENABLE(CM_RESET,CeReset),
  EV_COMMAND(CM_VIEWTREE,CmViewTree),
  EV_COMMAND_ENABLE(CM_VIEWTREE,CeViewTree),
  EV_COMMAND(CM_HELPCONTENTS,CmHelpContents),
  EV_COMMAND(CM_ABOUT,CmAbout),
END_RESPONSE_TABLE;

void TApp::InitInstance()
{
  SetDocManager(new TDocManager(dmSDI | dmMenu));
  TFrame* pFrame = new TFrame(*(Client = new TClient));
  TStatusBar* sb = new TStatusBar(pFrame, TGadget::Embossed,
                                  TStatusBar::CapsLock | TStatusBar::NumLock);
  m_pMessage = (TTextGadget*) (*sb)[0];
  sb->Insert(*new TStepsGadget);
  pFrame->Insert(*sb, TDecoratedFrame::Bottom);
  MainWindow = pFrame;
  pFrame->SetMenuDescr(TMenuDescr(IDM_APP,1,0,0,0,1,1));
  pFrame->SetIcon(GetMainWindow()->GetModule(),IDM_APP);
  MainWindow->SetFlag(wfMainWindow);
  MainWindow->Create();
  MainWindow->DragAcceptFiles(TRUE);
  MainWindow->Show(nCmdShow);
}

void TApp::EvDropFiles(TDropInfo dropInfo)
{
  int fileLength = dropInfo.DragQueryFileNameLen(0)+1;
  char* filePath = new char [fileLength];
  dropInfo.DragQueryFile(0, filePath, fileLength);
  TDocTemplate* tpl = GetDocManager()->MatchTemplate(filePath);
  if(tpl)
  {
    TDocument* doc;
    if((doc = GetDocManager()->DocList.Next(0)) != 0)
    {
      if(!doc->CanClose())
        return;
      if(!doc->Close())
      {
        GetDocManager()->PostDocError(*doc, IDS_UNABLECLOSE);
        return;
      }
      delete doc;
    }
    tpl->CreateDoc(filePath);
  }
  delete filePath;
  dropInfo.DragFinish();
}

BOOL TApp::IdleAction(long l)
{
  TDoc* pDoc = (TDoc*) GetDocManager()->GetCurrentDoc();
  BOOL bResult = !pDoc || pDoc->Idle();
  return TApplication::IdleAction(l) || bResult;
}

void TApp::EvNewView(TView& view)
{
  TBaseView* pBase = dynamic_cast<TBaseView*>(&view);
  TEditor* pEditor = dynamic_cast<TEditor*>(&view);
  TMDIChild* child;
  TRect rect = GetMainWindow()->GetClientWindow()->GetClientRect();
#ifdef __OS2__
  unsigned long nStyle = 0;
  int nCaption = GetMainWindow()->GetSystemMetrics(SM_CYCAPTION),
	         nFrame = GetMainWindow()->GetSystemMetrics(5);
#else
  int nCaption = GetSystemMetrics(SM_CYCAPTION),
                 nFrame = GetSystemMetrics(SM_CYFRAME);
#endif
  if(pEditor)
  {
    m_pDoc = pEditor->m_pDoc;
    m_pDoc->ReadLayout1();
    child = new TMDIEditorChild(*Client, 0, view.GetWindow());
  }
  else if(typeid(view) == typeid(TObjectView))
  {
    child = new TMDIObjectChild(*Client, 0, view.GetWindow());
    child->Attr.Style |= WS_VSCROLL | WS_HSCROLL;
  }
  else if(typeid(view) == typeid(TActorView))
    child = new TMDIActorChild(*Client, 0, view.GetWindow());
  else
  {
    child = new TMDIBaseChild(*Client, 0, view.GetWindow());
#ifdef __OS2__
    if(typeid(view) == typeid(TTreeView))
    {
      child->Attr.Style |= WS_VSCROLL | WS_HSCROLL;
      child->Scroller = new TScroller(child,1,1,1,1);
    }
#endif
  }
  child->SetMenuDescr(*view.GetViewMenu());
#if defined(__OS2__) || __BORLANDC__ < 0x460
  child->Attr.AccelTable = view.GetViewMenu()->Id;
#else
  child->Attr.AccelTable = view.GetViewMenu()->GetId();
#endif
  child->SetIcon(GetMainWindow()->GetModule(),child->Attr.AccelTable);
  if(m_pStream)
  {
    int nShowCmd;
    *m_pStream >> nShowCmd
               >> child->Attr.X
               >> child->Attr.Y
	       >> child->Attr.W
               >> child->Attr.H;
    if(nShowCmd == 2)
    {
      child->Attr.Style |= WS_MINIMIZE;
#ifdef __OS2__
      nStyle = child->Attr.Style;
      child->Attr.Style &= ~WS_VISIBLE;
      child->Attr.Style |= WS_DISABLED;
#endif
    }
    else if(nShowCmd == 1)
      child->Attr.Style |= WS_MAXIMIZE;
  }
  else if(typeid(view) == typeid(TTreeView))
  {
    child->Attr.X = 0;
    child->Attr.Y = 0;
    child->Attr.W = rect.right / 4;
    child->Attr.H = rect.bottom;
  }
  else if(typeid(view) == typeid(TObjectView) ||
          typeid(view) == typeid(TSensorView))
  {
    int nSize = nCaption + nFrame;
    m_pDoc->m_nDataCount++;
    child->Attr.X = rect.right < nSize ? 0
                    : (nSize * m_pDoc->m_nDataCount) % (rect.right - nSize);
    child->Attr.Y = rect.bottom < nSize ? 0
                    : (nSize * m_pDoc->m_nDataCount) % (rect.bottom - nSize);
    if(rect.right > rect.bottom)
    {
      child->Attr.H = rect.bottom * 4 / 5;
      child->Attr.W = child->Attr.H - nCaption;
    }
    else
    {
      child->Attr.W = rect.right * 4 / 5;
      child->Attr.H = child->Attr.W + nCaption;
    }
  }
  else if(typeid(view) == typeid(TActorView))
  {
    TClientDC dc(*GetMainWindow());
    int nHeight = 30 + dc.GetTextExtent("1",1).cy + nCaption + nFrame * 2;
    child->Attr.X = rect.right * 4 / 5;
    child->Attr.Y = rect.bottom < nHeight ? 0
                    : (nHeight * m_pDoc->m_nActorCount++) %
                      (rect.bottom - nHeight);
    child->Attr.W = rect.right / 5;
    child->Attr.H = nHeight;
  }
  child->Create();
#ifdef __OS2__
  if(nStyle & WS_MINIMIZE)
  {
    child->EnableWindow(TRUE);
    child->Attr.Style = nStyle;
    ::WinSetWindowPos(*child,HWND_TOP, 0, 0, 0, 0, SWP_MINIMIZE);
  }
#endif
  if(pBase && m_pStream)
    pBase->ReadLayout(*m_pStream,m_nVersion);
}

void TApp::EvCloseView(TView&)
{
}

void TApp::CmRestore()
{
  m_pDoc->ReadLayout2(*m_pStream,m_nVersion);
  delete m_pStream;
  m_pStream = 0;
}

void TApp::CmStart()
{
  m_pDoc->Start();
}

void TApp::CmStep()
{
  if(!m_pDoc->IsRunning())
    m_pDoc->Step();
}

void TApp::CmReset()
{
  if(!m_pDoc->IsRunning())
    m_pDoc->Reset();
}

void TApp::CeStart(TCommandEnabler& ce)
{
  ce.SetCheck(m_pDoc->IsRunning());
}

void TApp::CeStep(TCommandEnabler& ce)
{
  ce.Enable(!m_pDoc->IsRunning());
}

void TApp::CeReset(TCommandEnabler& ce)
{
  ce.Enable(!m_pDoc->IsRunning());
}

void TApp::CmViewTree()
{
  m_pDoc->ViewTree();
}

void TApp::CeViewTree(TCommandEnabler& ce)
{
  ce.Enable(m_pDoc->GetSimulation() != 0);
}

void TApp::CmHelpContents()
{
#ifdef __OS2__
  WinSendMsg((dynamic_cast<TFrame*>(GetMainWindow()))->m_hWndHelp,
             HM_HELP_CONTENTS,MPVOID,MPVOID);
#else
  GetMainWindow()->WinHelp(sHelp,HELP_INDEX,0);
#endif
}

void TApp::CmAbout()
{
  TDialog(GetMainWindow(), IDD_ABOUT).Execute();
}

static TApp* pApp;

int OwlMain(int argc, char* argv[])
{
  TApp App;
  if(argc == 1)
    sPath[0] = 0;
  else
    _fullpath(sPath,argv[1],_MAX_PATH);
  _fullpath(sHelp,argv[0],_MAX_PATH);
  for(int i = strlen(sHelp)-1; i >= 0 && sHelp[i] != '\\';i--);
  if(i >= 0 && sHelp[i] == '\\')
    sHelp[i] = 0;
  strcat(sHelp,"\\SimRobot.hlp");
  pApp = &App;
  return App.Run();
}

void WinExit(const char* pMessage)
{
  pApp->GetMainWindow()->MessageBox(pMessage,0,MB_OK | MB_ICONSTOP);
  exit(1);
}

