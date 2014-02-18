#ifndef __SRBASEVIEW_H__
#define __SRBASEVIEW_H__

#include "srDoc.h"

class _DOCVIEWCLASS TBaseView : public TWindowView
{
  private:
    TMenu m_Menu;
    TPopupMenu m_PopupMenu;
  protected:
    SIMULATION* m_pSim;
    BOOL VnError(TPoint*);
    void EvRButtonDown(UINT,TPoint&);
#ifdef __OS2__
    void EvDropFiles(TDropInfo dropInfo);
#endif
  public:
    TDoc* m_pDoc;
    TBaseView(TDoc& doc,TResId Menu,TWindow* parent = 0);
    BOOL CanClose() {return TWindow::CanClose();}
    virtual void WriteLayout(fstream& f);
    virtual void ReadLayout(fstream& f,int nVersion);
  DECLARE_RESPONSE_TABLE(TBaseView);
};

#endif
