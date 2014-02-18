#include "pch.h"
#include "srApp.h"
#include "srDataView.h"

DEFINE_RESPONSE_TABLE1(TDataView,TBaseView)
  EV_WM_SYSCOLORCHANGE,
  EV_VN_SUCCESS,
  EV_VN_REPAINT,
  EV_COMMAND(CM_EDITCOPY,CmEditCopy),
  EV_COMMAND_ENABLE(CM_EDITCOPY,CeEditCopy),
  EV_WM_SIZE,
END_RESPONSE_TABLE;

TDataView::TDataView(TDoc& doc,TResId Menu,TWindow* parent)
: TBaseView(doc,Menu,parent),
  m_Rect(0,0,0,0)
{
  m_pBitmap = 0;
  m_bRepaint = FALSE;
  m_sObject = m_pDoc->GetObject();
}

void TDataView::WriteLayout(fstream& f)
{
  TBaseView::WriteLayout(f);
  f << '.' << m_sObject << "\n";
}

void TDataView::ReadLayout(fstream& f,int nVersion)
{
  TBaseView::ReadLayout(f,nVersion);
  char buf[200];
  f >> ws;
  f.getline(buf,sizeof(buf));
  m_sObject = &buf[1];
  if(m_pSim && OnSuccess())
    m_bRepaint = TRUE;
  else
    m_pSim = 0;
}

void TDataView::SetupWindow()
{
  TBaseView::SetupWindow();
  if(m_pSim && !((TApp*) GetApplication())->m_pStream)
    VnSuccess(m_pSim);
}

void TDataView::EvSysColorChange()
{
  m_bRepaint = TRUE;
  TBaseView::EvSysColorChange();
}

BOOL TDataView::SetDocTitle(const char*, int index)
{
  if (index >= 0)
    Parent->SetCaption(STRING(GetViewName()) + STRING(" - ") +
		       STRING(m_sObject == ""
                       ? "WORLD"
                       : (const char*) m_sObject));
  return TRUE;
}

void TDataView::Paint(TDC& dc,BOOL,TRect&)
{
  TRect rect = GetClientRect();
  if(m_pSim && !Parent->IsIconic() && rect.right && rect.bottom)
  {
    if(!m_pBitmap || rect != m_Rect)
    {
      m_Rect = rect;
      delete m_pBitmap;
      m_pBitmap = new TBitmap(dc,rect.right,rect.bottom);
      m_bRepaint = TRUE;
    }
    TMemoryDC dcMem(dc);
    dcMem.SelectObject(*m_pBitmap);
    if(m_bRepaint)
    {
      m_bRepaint = FALSE;
      dcMem.SaveDC();
      TBrush br(SYSCOLOR(COLOR_WINDOW));
      dcMem.FillRect(rect,br);
      dcMem.SetMapMode(MM_ANISOTROPIC);
      dcMem.SetViewportExt(TSize(rect.right,rect.bottom));
      dcMem.SetViewportOrg(0,0);
      OnPaint(dcMem);
      dcMem.RestoreDC();
    }
    dc.BitBlt(0,0,rect.right,rect.bottom,dcMem,0,0);
  }
}

void TDataView::EvSize(UINT sizeType,TSize& size)
{
  TBaseView::EvSize(sizeType,size);
  TBaseView::Invalidate(FALSE);
}

BOOL TDataView::VnSuccess(SIMULATION* pSim)
{
  m_pSim = pSim;
  if(OnSuccess())
    m_bRepaint = TRUE;
  else
    m_pSim = 0;
  Invalidate();
  return TRUE;
}

void TDataView::Invalidate()
{
  m_bRepaint = TRUE;
#ifdef __OS2__
  TClientDC dc(*this);
  Paint(dc,TRUE,TRect());
#else
  TBaseView::Invalidate(FALSE);
#endif
}

BOOL TDataView::VnRepaint()
{
  if(m_pSim)
    Invalidate();
  return TRUE;
}

void TDataView::CmEditCopy()
{
  if(m_pSim)
  {
    TMetaFileDC dc;
    OnPaint(dc);
    TMetaFilePict mfp(dc.Close(),AutoDelete);
    TClipboard& clip = TClipboard::GetClipboard();
    clip.OpenClipboard(*this);
    clip.EmptyClipboard();
    mfp.ToClipboard(clip);
    HANDLE handle;
    PCHAR p = OnWrite(handle);
    if(p)
    {
#ifdef __OS2__
      clip.SetClipboardData(p);
      delete [] p;
#else
      ::GlobalUnlock(handle);
      ::SetClipboardData(CF_TEXT, handle);
#endif
    }
    clip.CloseClipboard();
  }
}

void TDataView::CeEditCopy(TCommandEnabler& ce)
{
  ce.Enable(m_pSim != 0);
}

