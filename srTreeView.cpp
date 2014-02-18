#include "pch.h"
#include "srApp.h"
#include "srObjectView.h"
#include "srSensorView.h"
#include "srActorView.h"
#include "srTreeView.h"

#ifdef __OS2__
#define SCROLLER Parent->Scroller
#else
#define SCROLLER Scroller
#endif

DEFINE_DOC_TEMPLATE_CLASS(TDoc,TTreeView,TreeTemplate);
TreeTemplate tplTree("Scenes (*.scn)","*.scn",0,"scn",
                     dtAutoDelete |
                     dtFileMustExist |
                     dtHidden |
                     dtOverwritePrompt |
                     dtSingleView|
                     dtUpdateDir);

TTreeView* TTreeView::New(TDoc& doc)
{
  return (TTreeView*) tplTree.CreateView(doc);
}

DEFINE_RESPONSE_TABLE1(TTreeView,TBaseView)
  EV_WM_SYSCOLORCHANGE,
  EV_WM_ERASEBKGND,
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONDBLCLK,
  EV_COMMAND(CM_OPENVIEW,CmOpenView),
  EV_COMMAND_ENABLE(CM_OPENVIEW,CeOpenView),
  EV_VN_SUCCESS,
  EV_COMMAND(CM_HELPONOBJECT,CmHelpOnObject),
END_RESPONSE_TABLE;

TTreeView::TTreeView(TDoc& doc,TWindow* parent)
: TBaseView(doc,IDP_TREE,parent)
{
  m_pDoc->m_pTreeView = this;
  SetViewMenu(new TMenuDescr(IDM_TREE,0,1,0,0,0,1));
  m_nTemplate = 0;
#ifndef __OS2__
  Attr.Style |= WS_VSCROLL | WS_HSCROLL;
  Scroller = new TScroller(this, 1, 1, 1, 1);
#endif
}

void TTreeView::BitmapBackground()
{
  TBitmap bmp(GetModule()->GetInstance(),IDM_TREE);
  TClientDC dcClient(*this);
  TMemoryDC dcSource(dcClient),
            dcDest(dcClient);
  m_pBmpIcons = new TBitmap(dcClient,bmp.Width(),bmp.Height());
  dcSource.SelectObject(bmp);
  dcDest.SelectObject(*m_pBmpIcons);
  dcDest.FillRect(0,0,bmp.Width(),bmp.Height(),TBrush(SYSCOLOR(COLOR_WINDOW)));
  for(int y = 0; y < bmp.Height(); y++)
    for(int x = 0; x < bmp.Width(); x++)
    {
      TColor cl = dcSource.GetPixel(x,y);
      if(!(cl.Red() > 0xf7 && cl.Green() > 0xf7 && cl.Blue() > 0xf7))
        dcDest.SetPixel(x,y,cl);
    }
}

void TTreeView::SetupWindow()
{
  TBaseView::SetupWindow();
  BitmapBackground();
}

void TTreeView::EvSysColorChange()
{
  delete m_pBmpIcons;
  BitmapBackground();
  TBaseView::EvSysColorChange();
}

BOOL TTreeView::EvEraseBkgnd(HDC hDC)
{
  TRect rect = GetClientRect();
  TDC dc(hDC);
  dc.FillRect(rect,TBrush(SYSCOLOR(COLOR_WINDOW)));
  return TRUE;
}

void TTreeView::WriteLayout(fstream& f)
{
  TBaseView::WriteLayout(f);
  f << m_nTemplate << " "
    << SCROLLER->XPos << " "
    << SCROLLER->YPos << " "
    << SCROLLER->XLine << " "
    << SCROLLER->YLine << " "
    << SCROLLER->XRange << " "
    << SCROLLER->YRange << "\n";
}

void TTreeView::ReadLayout(fstream& f,int nVersion)
{
  TBaseView::ReadLayout(f,nVersion);
  long xPos,yPos,xLine,yLine,xRange,yRange;
  f >> m_nTemplate >> xPos >> yPos >> xLine >> yLine >> xRange >> yRange;
  SCROLLER->XLine = xLine;
  SCROLLER->YLine = yLine;
  SCROLLER->SetPageSize();
  SCROLLER->SetRange(xRange,yRange);
  SCROLLER->ScrollTo(xPos,yPos);
}

TTreeView::~TTreeView()
{
  delete m_pBmpIcons;
  m_pDoc->m_pTreeView = 0;
}

BOOL TTreeView::SetDocTitle(const char*,int index)
{
  if(index >= 0)
    Parent->SetCaption(GetViewName());
  return TRUE;
}

BOOL TTreeView::VnSuccess(SIMULATION* pSim)
{
  m_pSim = pSim;
  Invalidate();
  return TRUE;
}

void TTreeView::Paint(TDC& dc,BOOL,TRect&)
{
  if (m_pSim)
  {
    m_pDC = &dc;
#ifdef __OS2__
    TFont font("Helv",10);
    m_pDC->SelectObject(font);
#endif
    m_pDC->SetBkMode(OPAQUE);
    m_pDC->SetBkColor(SYSCOLOR(COLOR_WINDOW));
    m_pBmpDC = new TMemoryDC(dc);
    m_pBmpDC->SelectObject(*m_pBmpIcons);
    m_xMax = 0;
    int y = 0,y2 = 0;
    EnumObjects(m_pSim->GetObject(""),-1,y,y2);
    delete m_pBmpDC;
    TSize sz = m_pDC->GetTextExtent("M",1);
    if(sz.cy < 16) sz.cy = 16;
    TRect rect = GetClientRect();
    m_xMax = m_xMax > rect.right ? m_xMax - rect.right : 0;
    y = y > rect.bottom ? y - rect.bottom : 0;
    SCROLLER->XLine = sz.cx;
    SCROLLER->YLine = sz.cy;
    SCROLLER->SetPageSize();
    SCROLLER->SetRange(m_xMax,y);
#ifdef __OS2__
    m_pDC->RestoreFont();
#endif
  }
}

STRING TTreeView::Add(STRING s1,STRING s2)
{
  if(s1 == "")
    return s2;
  else
    return s1 + STRING(".") + s2;
}

#ifdef __OS2__
#undef COLOR_HIGHLIGHT
#undef COLOR_HIGHLIGHTTEXT
#define COLOR_HIGHLIGHT (-32L)
#define COLOR_HIGHLIGHTTEXT (-33L)
#endif // __OS2__

void TTreeView::TextOut(int x,int& y,int& y2,STRING s,int nIcon,
                        STRING Name)
{
  TSize sz = m_pDC->GetTextExtent((const char*) s,s.Length());
  if(sz.cy < 16) sz.cy = 16;
  MOVE(*m_pDC,TPoint(x-8,y2));
  y2 = y + sz.cy / 2;
  LINE(*m_pDC,TPoint(x-8,y2));
  LINE(*m_pDC,TPoint(x,y2));
  m_pDC->BitBlt(x+2,y + sz.cy / 2 - 8,16,16,*m_pBmpDC,16*nIcon,0);
  if (Name == m_pDoc->GetObject())
  {
    m_pDC->SaveDC();
    m_pDC->SetBkColor(SYSCOLOR(COLOR_HIGHLIGHT)),
    m_pDC->SetTextColor(SYSCOLOR(COLOR_HIGHLIGHTTEXT));
    m_pDC->TextOut(x+22,y,(const char*) s,s.Length());
    m_pDC->RestoreDC();
  }
  else
    m_pDC->TextOut(x+22,y,(const char*) s,s.Length());
  y += sz.cy;
  if (m_xMax < x+22+sz.cx)
    m_xMax = x+22+sz.cx;
}

void TTreeView::EnumObjects(SIM3DOBJECT s,
                            int x,int& y,int& y2,STRING Name)
{
  TextOut(x,y,y2,m_pSim->ObjectClass(s) + STRING(" ") + m_pSim->ObjectName(s),
          0,Name);
  x += 16;
  int i,
      y3 = y;
  for(i = 0; i < m_pSim->ActorPortCount(s); i++)
    TextOut(x,y,y3,m_pSim->ActorPortName(s,i),2,
            Add(Name,m_pSim->ActorPortName(s,i)));
  for(i = 0; i < m_pSim->SensorPortCount(s); i++)
    TextOut(x,y,y3,m_pSim->SensorPortName(s,i),1,
            Add(Name,m_pSim->SensorPortName(s,i)));
  for(i = 0; i < m_pSim->SubObjectCount(s); i++)
    EnumObjects(m_pSim->SubObject(s,i),x,y,y3,
                Add(Name,m_pSim->ObjectName(m_pSim->SubObject(s,i))));
}

STRING TTreeView::GetName(SIM3DOBJECT s,int ySearch,int& y,STRING Name)
{
  if (ySearch == y)
    return STRING("o")+Name;
  else
  {
    y++;
    if (ySearch >= y && ySearch < y + m_pSim->ActorPortCount(s))
      return STRING("a")+Add(Name,m_pSim->ActorPortName(s,ySearch-y));
    else
    {
      y += m_pSim->ActorPortCount(s);
      if(ySearch >= y && ySearch < y + m_pSim->SensorPortCount(s))
        return STRING("s")+Add(Name,m_pSim->SensorPortName(s,ySearch-y));
      else
      {
        y += m_pSim->SensorPortCount(s);
        for (int i = 0; i < m_pSim->SubObjectCount(s); i++)
        {
      	  STRING sResult = GetName(m_pSim->SubObject(s,i),ySearch,y,
             Add(Name,m_pSim->ObjectName(m_pSim->SubObject(s,i))));
           if(!(sResult == ""))
	          return sResult;
        }
        return "";
      }
    }
  }
}

static TRect rect2; // fixes an optimizer bug

void TTreeView::EvLButtonDown(UINT,TPoint& point)
{
  m_nTemplate = 0;
  if (m_pSim)
  {
    TClientDC dc(*this);
#ifdef __OS2__
    TFont font("Helv",10);
    dc.SelectObject(font);
#endif
    TRect rect(point,point);
    rect2 = rect;
    SCROLLER->BeginView(dc,rect2);
    TSize sz = dc.GetTextExtent("M",1);
    if(sz.cy < 16) sz.cy = 16;
    int y = 0;
    STRING s = GetName(m_pSim->GetObject(""),rect2.bottom / sz.cy,y);
    if(!(s == ""))
    {
      if(s.First(1) == "o")
        m_nTemplate = 1;
      else if(s.First(1) == "s")
        m_nTemplate = 2;
      else
        m_nTemplate = 3;
      s = s.ButFirst(1);
      if(!(s == m_pDoc->GetObject()))
      {
        m_pDoc->GetObject() = s;
        Paint(dc,FALSE,TRect());
      }
    }
    else
      BEEP1;
    SCROLLER->EndView();
#ifdef __OS2__
    dc.RestoreFont();
#endif
  }
  else
    BEEP1;
}

void TTreeView::EvLButtonDblClk(UINT,TPoint& point)
{
  EvLButtonDown(0,point);
  CmOpenView();
}

void TTreeView::CmOpenView()
{
  if(m_pSim)
    switch(m_nTemplate)
    {
    case 1:
      TObjectView::New(*m_pDoc);
      break;
    case 2:
      TSensorView::New(*m_pDoc);
      break;
    case 3:
      TActorView::New(*m_pDoc);
    }
}

void TTreeView::CeOpenView(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nTemplate);
}

void TTreeView::CmHelpOnObject()
{
  if(m_pSim)
  {
    STRING s;
    if(m_nTemplate == 1)
      s = m_pSim->ObjectClass(m_pSim->GetObject(m_pDoc->GetObject()));
    else
    {
      const char* p = (const char *) m_pDoc->GetObject();
      for(int i = strlen(p)-1; i >= 0 && p[i] != '.'; i--);
      if(i >= 0)
        s = m_pDoc->GetObject().ButFirst(i+1);
    }
    (dynamic_cast<TFrame*>(GetApplication()->GetMainWindow()))
    ->ShowHelp(const_cast<char*>((const char*) s));
  }
}
