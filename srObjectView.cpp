#include "pch.h"
#include "NonPortable.h"
#include "sim3DWorld.h"
#include "srObjectView.h"

DEFINE_DOC_TEMPLATE_CLASS(TDoc,TObjectView,ObjectTemplate);
ObjectTemplate tplObject("Scenes (*.scn)","*.scn",0,"scn",
                         dtAutoDelete |
                         dtFileMustExist |
                         dtHidden |
                         dtOverwritePrompt |
                         dtUpdateDir);

TObjectView* TObjectView::New(TDoc& doc)
{
  return (TObjectView*) tplObject.CreateView(doc);
}

DEFINE_RESPONSE_TABLE1(TObjectView,TDataView)
  EV_COMMAND_AND_ID(CM_ZOOM,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM0,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM1,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM2,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM3,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM4,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM5,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM6,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM7,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM8,CmZoom),
  EV_COMMAND_AND_ID(CM_ZOOM9,CmZoom),
  EV_COMMAND_ENABLE(CM_ZOOM0,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM1,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM2,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM3,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM4,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM5,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM6,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM7,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM8,CeZoom),
  EV_COMMAND_ENABLE(CM_ZOOM9,CeZoom),
  EV_COMMAND_AND_ID(CM_DIST,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST0,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST1,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST2,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST3,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST4,CmDistortion),
  EV_COMMAND_AND_ID(CM_DIST5,CmDistortion),
  EV_COMMAND_ENABLE(CM_DIST0,CeDistortion),
  EV_COMMAND_ENABLE(CM_DIST1,CeDistortion),
  EV_COMMAND_ENABLE(CM_DIST2,CeDistortion),
  EV_COMMAND_ENABLE(CM_DIST3,CeDistortion),
  EV_COMMAND_ENABLE(CM_DIST4,CeDistortion),
  EV_COMMAND_ENABLE(CM_DIST5,CeDistortion),
  EV_COMMAND_AND_ID(CM_DL,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL0,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL1,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL2,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL3,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL4,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL5,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL6,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL7,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL8,CmDetaillevel),
  EV_COMMAND_AND_ID(CM_DL9,CmDetaillevel),
  EV_COMMAND_ENABLE(CM_DL0,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL1,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL2,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL3,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL4,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL5,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL6,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL7,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL8,CeDetaillevel),
  EV_COMMAND_ENABLE(CM_DL9,CeDetaillevel),
  EV_COMMAND(CM_HIDDEN,CmHidden),
  EV_COMMAND_ENABLE(CM_HIDDEN,CeHidden),
  EV_COMMAND(CM_TURN,CmTurn),
  EV_COMMAND_ENABLE(CM_TURN,CeTurn),
  EV_WM_LBUTTONDOWN,
END_RESPONSE_TABLE;

TObjectView::TObjectView(TDoc& doc,TWindow* parent)
: TDataView(doc,IDP_OBJECT,parent)
{
  SetViewMenu(new TMenuDescr(IDM_OBJECT,0,3,0,0,0,0));
  m_nScale = 4;
  m_nDistortion = 4;
  m_bHidden = FALSE;
  m_bTracking = TRUE;
  m_nDetail = 9;
}

void TObjectView::WriteLayout(fstream& f)
{
  TDataView::WriteLayout(f);
  f << m_nScale << " "
    << m_bHidden << " "
    << m_bTracking << " "
    << m_nDetail << " "
    << Parent->GetScrollPos(SB_HORZ) << " "
    << Parent->GetScrollPos(SB_VERT) << " "
    << m_nDistortion << "\n";
}

void TObjectView::ReadLayout(fstream& f,int nVersion)
{
  TDataView::ReadLayout(f,nVersion);
  int x,y;
  f >> m_nScale >> m_bHidden >> m_bTracking >> m_nDetail >> x >> y;
  if(nVersion > 1)
    f >> m_nDistortion;
  else
    m_nDistortion = 4;
  Parent->SetScrollPos(SB_HORZ,x,FALSE);
  Parent->SetScrollPos(SB_VERT,y,FALSE);
}

void TObjectView::SetupWindow()
{
  Parent->SetScrollRange(SB_HORZ,0,360,FALSE);
  Parent->SetScrollRange(SB_VERT,0,360,FALSE);
  Parent->SetScrollPos(SB_HORZ,180,FALSE);
  Parent->SetScrollPos(SB_VERT,225,FALSE);
  TDataView::SetupWindow();
}

BOOL TObjectView::OnSuccess()
{
  m_pObject = m_pSim->GetObject(m_sObject);
  sim3DTakeLastError();
  return m_pObject != 0;
}

void TObjectView::OnPaint(TDC& dc)
{
  dc.SetWindowExt(TSize(16384,-16384));
#ifdef __OS2__
  dc.SetWindowOrg(TPoint(-8192,-8192));
#else
  dc.SetWindowOrg(TPoint(-8192,8192));
#endif
  OWLGRAPH gp(&dc);
  gp.RegisterColors(((SIM3DWorld*) m_pSim->GetObject(""))->ColorList);
  int x = Parent->GetScrollPos(SB_HORZ),
      y = Parent->GetScrollPos(SB_VERT);
  MOVEMATRIX M(TurnYZMatrix ((double) ((y+90) % 360))
               * TurnXYMatrix ((double) ((x+180) % 360))
               * MATRIX(VECTOR (1 << m_nScale,0,0),VECTOR (0,1 << m_nScale,0),
                        VECTOR (0,0,1 << m_nScale)),VECTOR (0,0,0));
  gp.SetViewPoint (M, 1, m_nDistortion ? (16384 >> m_nDistortion) << m_nScale : 1e12);
  gp.SetHidden(m_bHidden);
  gp.NewPicture();
  m_pSim->DrawObject(m_pObject,&gp,m_nDetail,20);
  gp.EndOfPicture();
}

void TObjectView::CmZoom(WPARAM nId)
{
  m_nScale = (nId - CM_ZOOM0);
  Invalidate();
}

void TObjectView::CeZoom(TCommandEnabler& ce)
{
  ce.SetCheck(ce.Id - CM_ZOOM0 == m_nScale);
}

void TObjectView::CmDistortion(WPARAM nId)
{
  m_nDistortion = (nId - CM_DIST0);
  Invalidate();
}

void TObjectView::CeDistortion(TCommandEnabler& ce)
{
  ce.SetCheck(ce.Id - CM_DIST0 == m_nDistortion);
}

void TObjectView::CmDetaillevel(WPARAM nId)
{
  m_nDetail = nId - CM_DL0;
  Invalidate();
}

void TObjectView::CeDetaillevel(TCommandEnabler& ce)
{
  ce.SetCheck(ce.Id - CM_DL0 == m_nDetail);
}

void TObjectView::CmHidden()
{
  m_bHidden = 1 - m_bHidden;
  Invalidate();
}

void TObjectView::CeHidden(TCommandEnabler& ce)
{
  ce.SetCheck(m_bHidden);
}

void TObjectView::CmTurn()
{
  m_bTracking = 1 - m_bTracking;
  Invalidate();
}

void TObjectView::CeTurn(TCommandEnabler& ce)
{
  ce.SetCheck(m_bTracking);
}

void TObjectView::EvLButtonDown(UINT,TPoint& point)
{
  if(m_pSim)
  {
    TRect rect = GetClientRect();
    TClientDC dc(*this);
    dc.SetMapMode(MM_ANISOTROPIC);
    dc.SetViewportExt(rect.Size());
    dc.SetViewportOrg(0,0);
    dc.SetWindowExt(TSize(16384,-16384));
#ifdef __OS2__
    dc.SetWindowOrg(TPoint(-8192,-8192));
#else
    dc.SetWindowOrg(TPoint(-8192,8192));
#endif
    dc.DPtoLP(&point);
    int x = Parent->GetScrollPos(SB_HORZ),
        y = Parent->GetScrollPos(SB_VERT);
    CLICKINFO ci(m_pObject,VECTOR(point.x,point.y,0),
                 MOVEMATRIX(TurnYZMatrix ((double) ((y+90) % 360))
                            * TurnXYMatrix ((double) ((x+180) % 360))
                            * MATRIX(VECTOR (1 << m_nScale,0,0),VECTOR (0,1 << m_nScale,0),
                                     VECTOR (0,0,1 << m_nScale)),VECTOR (0,0,0)),
                1 << m_nScale,m_nDistortion ? (16384 >> m_nDistortion) << m_nScale : 1e12);
    ((TDoc&) GetDocument()).OnMouseClick(ci);
  }
}
