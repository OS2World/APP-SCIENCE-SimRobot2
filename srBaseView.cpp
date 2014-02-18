#include "pch.h"
#include "srApp.h"
#include "srBaseView.h"

DEFINE_RESPONSE_TABLE1(TBaseView,TWindowView)
  EV_VN_ERROR,
  EV_WM_RBUTTONDOWN,
  EV_WM_DROPFILES,
END_RESPONSE_TABLE;

TBaseView::TBaseView(TDoc& doc,TResId Menu,TWindow* parent)
: TWindowView(doc,parent),
  m_Menu(*GetModule(),Menu),
#ifdef __OS2__
  m_PopupMenu(m_Menu)
#else
  m_PopupMenu(m_Menu.GetSubMenu(0))
#endif
{
  m_pDoc = &doc;
  m_pSim = m_pDoc->GetSimulation();
}

void TBaseView::WriteLayout(fstream& f)
{
  (dynamic_cast<TMDIBaseChild*>(Parent))->WriteLayout(f);
}

void TBaseView::ReadLayout(fstream&,int)
{
  // Layout is read in TApp::EvNewView
}

BOOL TBaseView::VnError(TPoint*)
{
  m_pSim = 0;
  Invalidate();
  return TRUE;
}

void TBaseView::EvRButtonDown(UINT,TPoint& point)
{
  Parent->SetFocus();
  SetFocus();
  TPoint lp = point;
  ClientToScreen(lp);
  TFrame* pFrame = dynamic_cast<TFrame*>(GetApplication()->GetMainWindow());
#ifndef __OS2__
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,0,*pFrame);
#else
  pFrame->EvInitMenuPopup(m_PopupMenu,0,FALSE);
  pFrame->ScreenToClient(lp);
  m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,lp,*pFrame);
}

void TBaseView::EvDropFiles(TDropInfo dropInfo)
{
  (dynamic_cast<TApp*>(GetApplication()))->EvDropFiles(dropInfo);
#endif
}


