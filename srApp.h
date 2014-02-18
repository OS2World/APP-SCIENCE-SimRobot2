#ifndef __SRAPP_H__
#define __SRAPP_H__

#include "srDoc.h"

class _USERCLASS TFrame : public TDecoratedMDIFrame {
  public:
    TFrame(TMDIClient& clientWnd);
#ifdef __OS2__
    ~TFrame();
    HWND m_hWndHelp;
#endif
    void ShowHelp(char* pTopic);
    void WriteLayout(fstream& f);
    void ReadLayout(fstream& f,int nVersion);
    void EvInitMenuPopup(HMENU hPopupMenu,UINT index,BOOL sysMenu);
  protected:
    void SetupWindow();
#ifdef __OS2__
  public:
    void ScreenToClient(TPoint& point) const;
  protected:
    void CmActivateChild(WPARAM id);
    void EvMenuSelect(UINT menuItemId, UINT flags, HMENU hMenu);
    void EvSize(UINT sizeType,TSize& size);
#endif
  DECLARE_RESPONSE_TABLE(TFrame);
};

class TApp : public TApplication {
  public:
    fstream* m_pStream;
    TDoc* m_pDoc;
    int m_nVersion;
    TTextGadget* m_pMessage;
    TApp() : TApplication() {m_pStream = 0; m_pDoc = 0;}
    void EvDropFiles(TDropInfo dropInfo);
  protected:
    void InitInstance();
    BOOL IdleAction(long);
    void EvNewView(TView& view);
    void EvCloseView(TView& view);
    void CmRestore();
    void CmStart();
    void CmStep();
    void CmReset();
    void CeStart(TCommandEnabler&);
    void CeStep(TCommandEnabler&);
    void CeReset(TCommandEnabler&);
    void CmViewTree();
    void CeViewTree(TCommandEnabler& ce);
    void CmHelpContents();
    void CmAbout();
    TMDIClient* Client;
  DECLARE_RESPONSE_TABLE(TApp);
};

class _USERCLASS TMDIBaseChild : public TMDIChild
{
  public:
    TMDIBaseChild(TMDIClient&     parent,
                  const char far* title = 0,
                  TWindow*        clientWnd = 0,
                  BOOL            shrinkToClient = FALSE,
                  TModule*        module = 0);
    void WriteLayout(fstream& f);
};

#endif
