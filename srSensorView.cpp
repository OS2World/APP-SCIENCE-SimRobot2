#include "pch.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "sim3DWorld.h"
#include "srSensorView.h"

DEFINE_DOC_TEMPLATE_CLASS(TDoc,TSensorView,SensorTemplate);
SensorTemplate tplSensor("Scenes (*.scn)","*.scn",0,"scn",
                         dtAutoDelete |
                         dtFileMustExist |
                         dtHidden |
                         dtOverwritePrompt |
                         dtUpdateDir);

TSensorView* TSensorView::New(TDoc& doc)
{
  return (TSensorView*) tplSensor.CreateView(doc);
}

DEFINE_RESPONSE_TABLE1(TSensorView,TDataView)
  EV_COMMAND_AND_ID(CM_LINE,CmViewer),
  EV_COMMAND_AND_ID(CM_COLUMN,CmViewer),
  EV_COMMAND_AND_ID(CM_GREY,CmViewer),
  EV_COMMAND_AND_ID(CM_COLOR,CmViewer),
  EV_COMMAND_AND_ID(CM_STEREO,CmViewer),
  EV_COMMAND_AND_ID(CM_DRAWING,CmViewer),
  EV_COMMAND_ENABLE(CM_LINE,CeLine),
  EV_COMMAND_ENABLE(CM_COLUMN,CeColumn),
  EV_COMMAND_ENABLE(CM_GREY,CeGrey),
  EV_COMMAND_ENABLE(CM_COLOR,CeColor),
  EV_COMMAND_ENABLE(CM_STEREO,CeStereo),
  EV_COMMAND_ENABLE(CM_DRAWING,CeDrawing),
  EV_COMMAND(CM_GRID,CmGrid),
  EV_COMMAND_ENABLE(CM_GRID,CeGrid),
  EV_COMMAND_AND_ID(CM_BRIGHT,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT0,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT1,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT2,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT3,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT4,CmBright),
  EV_COMMAND_AND_ID(CM_BRIGHT5,CmBright),
  EV_COMMAND_ENABLE(CM_BRIGHT0,CeBright),
  EV_COMMAND_ENABLE(CM_BRIGHT1,CeBright),
  EV_COMMAND_ENABLE(CM_BRIGHT2,CeBright),
  EV_COMMAND_ENABLE(CM_BRIGHT3,CeBright),
  EV_COMMAND_ENABLE(CM_BRIGHT4,CeBright),
  EV_COMMAND_ENABLE(CM_BRIGHT5,CeBright),
END_RESPONSE_TABLE;

TSensorView::TSensorView(TDoc& doc,TWindow* parent)
: TDataView(doc,IDP_SENSOR,parent)
{
  SetViewMenu(new TMenuDescr(IDM_SENSOR,0,3,0,0,0,0));
  m_nType = 3;
  m_bGrid = FALSE;
  m_dBright = 0.5;
}

void TSensorView::WriteLayout(fstream& f)
{
  TDataView::WriteLayout(f);
  f << m_nType << " "
    << m_bGrid << " "
    << m_dBright << "\n";
}

void TSensorView::ReadLayout(fstream& f,int nVersion)
{
  char buf[25];
  TDataView::ReadLayout(f,nVersion);
  f >> m_nType >> m_bGrid;
// f >> m_dBright; work around a bug with Win95
  f >> ws;
  f.getline(buf,sizeof(buf));
  m_dBright = atof(buf);
}

BOOL TSensorView::OnSuccess()
{
  m_spSensor = m_pSim->GetSensorPort(m_sObject);
  if(m_spSensor.PortObject != 0)
  {
    m_nDim = 0;
    m_nDimSize[0] = m_nDimSize[1] = m_nDimSize[2] = 1;
    for(int i = 0; i < 3; i++)
      if(m_pSim->GetSensorDimSize(m_spSensor,i) > 1)
      {
        m_nDimSize[m_nDim] = m_pSim->GetSensorDimSize(m_spSensor,i);
        m_nDim++;
      }
    if(m_nType == 3 && (!m_nDim || m_nDimSize[m_nDim-1] > 3))
      m_nType = 2;
    switch (m_nType)
    {
      case 0:
        if(!m_nDim)
          m_nType = 1;
      case 1:
      case 2:
      case 4:
        if(m_nDim > 2)
          m_nType = 3;
      case 3:
        if(m_nDimSize[2] > 3 && m_nDimSize[1] == 2)
          m_nType = 5;
    }
    return !(m_nType == 3 && (!m_nDim || m_nDimSize[m_nDim-1] > 3));
  }
  else
  {
    sim3DTakeLastError();
    return FALSE;
  }
}

void TSensorView::OnPaint(TDC& dc)
{
  PSHORTREAL pData = m_pSim->NewSensorBuffer(m_spSensor);
  m_pSim->GetSensorValue(m_spSensor,pData);
  switch(m_nType)
  {
    case 0:
      LineViewer(dc,pData);
      break;
    case 1:
      ColumnViewer(dc,pData);
      break;
    case 2:
      GreyViewer(dc,pData);
      break;
    case 3:
      ColorViewer(dc,pData);
      break;
    case 4:
      SirdsViewer(dc,pData);
      break;
    case 5:
      DrawingViewer(dc,pData);
  }
  m_pSim->DeleteSensorBuffer(m_spSensor,pData);
}

void TSensorView::CmViewer(WPARAM nId)
{
  if(!m_pSim)
    return;
  int nType = nId - CM_LINE;
  switch(nType)
  {
    case 0:
      if(m_nDimSize[0] == 1)
        return;
    case 1:
    case 2:
    case 4:
      if(m_nDim > 2)
        return;
      break;
    case 3:
      if(!m_nDim || m_nDimSize[m_nDim-1] > 3)
        return;
      break;
    case 5:
      if(m_nDimSize[0] == 1 || m_nDimSize[1] != 2)
        return;
  }
  m_nType = nType;
  OnSuccess();
  Invalidate();
}

void TSensorView::CeLine(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDim < 3 && m_nDimSize[0] > 1);
  ce.SetCheck(m_nType == 0);
}

void TSensorView::CeColumn(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDim < 3);
  ce.SetCheck(m_nType == 1);
}

void TSensorView::CeGrey(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDim < 3);
  ce.SetCheck(m_nType == 2);
}

void TSensorView::CeColor(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDim && m_nDimSize[m_nDim-1] <= 3);
  ce.SetCheck(m_nType == 3);
}

void TSensorView::CeStereo(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDim < 3);
  ce.SetCheck(m_nType == 4);
}

void TSensorView::CeDrawing(TCommandEnabler& ce)
{
  ce.Enable(m_pSim && m_nDimSize[0] > 1 && m_nDimSize[1] == 2);
  ce.SetCheck(m_nType == 5);
}

void TSensorView::CmGrid()
{
  if(m_nType != 4)
  {
    m_bGrid = 1 - m_bGrid;
    Invalidate();
  }
}

void TSensorView::CeGrid(TCommandEnabler& ce)
{
  ce.Enable(m_nType != 4);
  ce.SetCheck(m_bGrid);
}

void TSensorView::CmBright(WPARAM nId)
{
  m_dBright = 1.0 / ((nId - CM_BRIGHT0)*0.5 + 1.0);
  Invalidate();
}

void TSensorView::CeBright(TCommandEnabler& ce)
{
  double d = 1.0 / ((ce.Id - CM_BRIGHT0)*0.5 + 1.0) - m_dBright;
  ce.SetCheck(d*d < 0.0001);
}

void TSensorView::Grid2D(TDC& dc)
{
  if(m_bGrid)
  {
    TPen pen(TColor(0,0,0),0);
    dc.SelectObject(pen);
    for (int y = 0; y <= m_nDimSize[1] ; y++)
    {
      MOVE(dc,TPoint(0,y));
      LINE(dc,TPoint(m_nDimSize[0],y));
    }
    for (int x = 0; x <= m_nDimSize[0]; x++)
    {
      MOVE(dc,TPoint(x,0));
      LINE(dc,TPoint(x,m_nDimSize[1]));
    }
    dc.RestorePen();
  }
}

void TSensorView::Grid3D(TDC& dc)
{
  if(m_bGrid)
  {
    TPen pen(TColor(192,192,192),0);
    dc.SelectObject(pen);
    for (int y = 0; y <= m_ySize ; y++)
    {
      MOVE(dc,Project2D(0,y,1));
      LINE(dc,Project2D(0,y,0));
      LINE(dc,Project2D(m_xSize,y,0));
    }
    for (int x = 0; x <= m_xSize; x++)
    {
      MOVE(dc,Project2D(x,0,1));
      LINE(dc,Project2D(x,0,0));
      LINE(dc,Project2D(x,m_ySize,0));
    }
    for (SHORTREAL z = 0; z < 1.05; z += (SHORTREAL) 0.1)
    {
      MOVE(dc,Project2D(0,m_ySize,z));
      LINE(dc,Project2D(0,0,z));
      LINE(dc,Project2D(m_xSize,0,z));
    }
    dc.RestorePen();
  }
}

void TSensorView::GreyViewer(TDC& dc,PSHORTREAL pData)
{
  dc.SetWindowExt(TSize(m_nDimSize[0],m_nDimSize[1]));
  for (int y = 0; y < m_nDimSize[1]; y++)
  {
    int xStart = 0,
        x = 0;
    int clrLast,
        clrThis;
    while(x < m_nDimSize[0])
    {
      SHORTREAL r = pData[x + y * (LONGINT) m_nDimSize[0]];
      if (r > 1)
        r = 1;
      else if (r < 0)
        r = 0;
      clrThis = (int) (pow(r,m_dBright) * 255);
      if(!x)
        clrLast = clrThis;
      else if(clrThis != clrLast)
      {
        TBrush br(TColor(clrLast,clrLast,clrLast));
        TRect rect(xStart,y,x,y+1);
        dc.FillRect(rect,br);
        xStart = x;
        clrLast = clrThis;
      }
      x++;
    }
    TBrush br(TColor(clrLast,clrLast,clrLast));
    TRect rect(xStart,y,x,y+1);
    dc.FillRect(rect,br);
  }
  Grid2D(dc);
}

void TSensorView::ColorViewer(TDC& dc,PSHORTREAL pData)
{
  int zMax = m_nDimSize[m_nDim-1],
      xMax = m_nDim > 1 ? m_nDimSize[0] : 1,
      yMax = m_nDim > 2 ? m_nDimSize[1] : 1;
  dc.SetWindowExt(TSize(xMax,yMax));
  for (int y = 0; y < yMax; y++)
  {
    int xStart = 0,
        x = 0;
    TColor clrLast,
           clrThis;
    while(x < xMax)
    {
      int Color[3] = {0,0,0};
      for (int z = 0; z < zMax; z++)
      {
        SHORTREAL r = pData[x + y * (LONGINT) xMax
                              + z * (LONGINT) xMax
                                  * (LONGINT) yMax];
        if (r > 1)
          r = 1;
        else if (r < 0)
          r = 0;
        Color[z] = (int) (pow(r,m_dBright) * 255);
      }
      clrThis = TColor(Color[0],Color[1],Color[2]);
      if(!x)
        clrLast = clrThis;
      else if(clrThis != clrLast)
      {
        TBrush br(clrLast);
        TRect rect(xStart,y,x,y+1);
        dc.FillRect(rect,br);
        xStart = x;
        clrLast = clrThis;
      }
      x++;
    }
    TBrush br(clrLast);
    TRect rect(xStart,y,x,y+1);
    dc.FillRect(rect,br);
  }
  Grid2D(dc);
}

TPoint TSensorView::Project2D(int x,int y,SHORTREAL z)
{
  if (z > 1)
    z = 1;
  else if (z < 0)
    z = 0;
  if (m_ySize)
    return TPoint(x*2+m_ySize-y,
                  500 + (int) ((long) m_yAdd * y / m_ySize)
                        - (int) (z * 500 + 0.5));
  else
    return TPoint(x*2,500 - (int) (z * 500 + 0.5));
}

void TSensorView::ColumnViewer(TDC& dc,PSHORTREAL pData)
{
  m_xSize = m_nDimSize[0];
  m_ySize = m_nDimSize[1];
  m_yAdd = (int) (500L * m_ySize / m_xSize / 2);
  dc.SetWindowExt(TSize(m_xSize * 2 +  m_ySize,500 + m_yAdd));
  Grid3D(dc);
  TBrush brGray(SYSCOLOR(COLOR_BTNFACE)),
			brWhite(SYSCOLOR(COLOR_BTNHIGHLIGHT)),
			brDark(SYSCOLOR(COLOR_BTNSHADOW));
  for (int y = 0; y < m_nDimSize[1]; y++)
    for (int x = 0; x < m_nDimSize[0]; x++)
    {
      SHORTREAL r = pData[x + (LONGINT) m_nDimSize[0] * y];
      TPoint pt[4] =
      {
        Project2D(x,y+1,r),
        Project2D(x+1,y+1,r),
        Project2D(x+1,y,r),
        Project2D(x,y,r)
      };
      dc.SelectObject(brWhite);
      POLY(dc,pt,4);
      SHORTREAL r2 = y == m_nDimSize[1]-1
                     ? 0
                     : pData[x + (LONGINT) m_nDimSize[0] * (y+1)];
      if(r2 < r)
      {
        pt[2] = Project2D(x+1,y+1,r2);
        pt[3] = Project2D(x,y+1,r2);
        dc.SelectObject(brGray);
        POLY(dc,pt,4);
      }
      r2 = x == m_nDimSize[0]-1 ? 0
                  : pData[x+1 + (LONGINT) m_nDimSize[0] * y];
      if (r2 < r)
      {
        pt[0] = Project2D(x+1,y,r);
        pt[2] = Project2D(x+1,y+1,r2);
        pt[3] = Project2D(x+1,y,r2);
        dc.SelectObject(brDark);
        POLY(dc,pt,4);
      }
    }
  dc.RestoreBrush();
}

void TSensorView::LineViewer(TDC& dc,PSHORTREAL pData)
{
  m_xSize = m_nDimSize[0]-1;
  m_ySize = m_nDimSize[1]-1;
  m_yAdd = (int) (500L * m_ySize / m_xSize / 2);
  dc.SetWindowExt(TSize(m_xSize * 2 +  m_ySize,500 + m_yAdd));
  Grid3D(dc);
  if(m_nDimSize[1] == 1)
    for(int x = 0; x < m_nDimSize[0]; x++)
    {
      TPoint pt = Project2D(x,0,pData[x]);
      if (x)
        LINE(dc,pt);
      else
        MOVE(dc,pt);
    }
  else
  {
    TBrush br(SYSCOLOR(COLOR_BTNFACE));
    dc.SelectObject(br);
    for (int y = 0; y < m_nDimSize[1]-1; y++)
      for (int x = 0; x < m_nDimSize[0]-1; x++)
      {
        TPoint pt[4] =
        {
          Project2D(x,y,pData[x + (LONGINT) m_nDimSize[0] * y]),
          Project2D(x+1,y,pData[x+1 + (LONGINT) m_nDimSize[0] * y]),
          Project2D(x+1,y+1,pData[x+1 + (LONGINT) m_nDimSize[0] * (y+1)]),
          Project2D(x,y+1,pData[x + (LONGINT) m_nDimSize[0] * (y+1)])
        };
        POLY(dc,pt,4);
      }
  }
  dc.RestorePen();
}

void TSensorView::Constrains(PSHORTREAL pDepth,INTEGER* pConstrain,
                             BOOL* pVisible,REAL xDepthStep)
{
  REAL    E = 1.0/250,
          z,
          zMax = DBL_MIN;
  INTEGER nPos = m_xSize;

  for(INTEGER x = m_xSize-1; x >= 0; x--)
  {
    pConstrain[x] = x;
    z = pDepth[(INTEGER) (x * xDepthStep)];
    if(z < 0)
    {
      z = 0;
      pDepth[(INTEGER) (x * xDepthStep)] = 0;
    }
    else if(z > 1)
    {
      z = 1;
      pDepth[(INTEGER) (x * xDepthStep)] = 1;
    }
    if(z + (nPos-x) * E * (12 - 2 * z) > zMax)
    {
      nPos = x;
      zMax = z;
      pVisible[x] = TRUE;
    }
    else
      pVisible[x] = FALSE;
  }

  zMax = DBL_MIN;
  nPos = -1;
  for(x = 0; x < m_xSize; x++)
  {
    if(pVisible[x])
    {
      z = pDepth[(INTEGER) (x * xDepthStep)];
      if(z + (x-nPos) * E * (12 - 2 * z) > zMax)
      {
        nPos = x;
        zMax = z;
        INTEGER s = (INTEGER) ((3 - z) / (6 - z) / E + 0.5),
                left = x - (s >> 1),
                right = left + s;
        if(0 <= left && right < m_xSize)
        {
          INTEGER l = pConstrain[left];
          while(l != left && l != right)
          {
            if(l < right)
            {
              left = l;
              l = pConstrain[left];
            }
            else
            {
              pConstrain[left] = right;
              left = right;
              right = l;
              l = pConstrain[left];
            }
          }
          pConstrain[left] = right;
        }
      }
    }
  }
}

void TSensorView::SirdsViewer(TDC& dc,PSHORTREAL pData)
{
  TRect rect = GetClientRect();
  m_xSize = rect.right;
  m_ySize = rect.bottom;
  dc.SetWindowExt(TSize(m_xSize,m_ySize));
  INTEGER* pConstrain = new INTEGER[m_xSize];
  BOOL* pVisible = new BOOL[m_xSize];
  REAL xDepthStep = (REAL) m_nDimSize[0] / m_xSize,
       yDepthStep = (REAL) m_nDimSize[1] / m_ySize;
  INTEGER yd = -1,
          ydo;
  srand(0);
  for(INTEGER y = 0; y < m_ySize; y++)
  {
    ydo = yd;
    yd = (INTEGER) (y * yDepthStep);
    if(yd != ydo)
      Constrains(pData + (LONGINT) yd * m_nDimSize[0],pConstrain,
                 pVisible,xDepthStep);
    for(INTEGER x = m_xSize-1; x >= 0; x--)
    {
      BOOL bPixel = rand() & 1;
      if(pConstrain[x] == x)
        pVisible[x] = bPixel;
      else
        pVisible[x] = pVisible[pConstrain[x]];
    }
    for(x = 0; x < m_xSize; x++)
      if(pVisible[x])
        PIXEL(dc,TPoint(x,y));
  }
  delete [] pVisible;
  delete [] pConstrain;
}

PCHAR TSensorView::OnWrite(HANDLE& handle)
{
  PSHORTREAL pData = m_pSim->NewSensorBuffer(m_spSensor);
  m_pSim->GetSensorValue(m_spSensor,pData);
  char sBuffer[30];
  PSHORTREAL pShortReal = pData;
  LONGINT nCount = 0;
  for(LONGINT y = 0; y < (LONGINT) m_nDimSize[1] * m_nDimSize[2]; y++)
    for(INTEGER x = 0; x < m_nDimSize[0]; x++)
    {
      sprintf(sBuffer,"%g%s",*pShortReal,
              x + 1 == m_nDimSize[0] ? "\n" : "\t");
      nCount += strlen(sBuffer);
      pShortReal++;
    }
#ifdef __OS2__
  handle = 0;
  PCHAR pBuffer = new char[nCount+1];
#else
  handle = ::GlobalAlloc(GHND,nCount+1);
  if(!handle)
    return 0;
  PCHAR pBuffer = (PCHAR) ::GlobalLock(handle);
#endif // __WIN16__
  PCHAR pChar = pBuffer;
  pShortReal = pData;
  for(y = 0; y < (LONGINT) m_nDimSize[1] * m_nDimSize[2]; y++)
    for(INTEGER x = 0; x < m_nDimSize[0]; x++)
    {
      sprintf(sBuffer,"%g%s",*pShortReal,
              x + 1 == m_nDimSize[0] ? "\n" : "\t");
      char* p = sBuffer;
      while(*p)
        *(pChar++) = *(p++);
      pShortReal++;
    }
  *pChar = 0;
  m_pSim->DeleteSensorBuffer(m_spSensor,pData);
  return pBuffer;
}

void TSensorView::DrawingViewer(TDC& dc,PSHORTREAL pData)
{
  dc.SetWindowExt(TSize(10000,-10000));
#ifdef __OS2__
  dc.SetWindowOrg(TPoint(0,-10000));
#else
  dc.SetWindowOrg(TPoint(0,10000));
#endif
  if(m_bGrid)
  {
    TPen pen(TColor(192,192,192),0);
    dc.SelectObject(pen);
    for (int i = 0; i <= 10000; i += 1000)
    {
      MOVE(dc,TPoint(0,i));
      LINE(dc,TPoint(10000,i));
      MOVE(dc,TPoint(i,0));
      LINE(dc,TPoint(i,10000));
    }
    dc.RestorePen();
  }
  COLORLIST& cl = ((SIM3DWorld*) m_pSim->GetObject(""))->ColorList;
  for(int i = 0; i < m_nDimSize[2]; i++)
  {
    TColor col;
    cl.NthColor(i);
    if(cl.RGBDefined())
    {
      VECTOR v = cl.ActualRGB();
      col = TColor(v.x * 255,v.y*255,v.z*255);
    }
    else
    {
      PALETTEENTRY pe;
      dc.GetSystemPaletteEntries(i,1,(PALETTEENTRY*) &pe);
      col = TColor(pe);
    }
    TPen pen(col,0);
    dc.SelectObject(pen);
    for(int j = 0; j < m_nDimSize[0]; j++)
    {
      TPoint pt((int) (pData[2 * i * (long) m_nDimSize[0] + j] * 10000),
                (int) (pData[(2 * i + 1) * (long) m_nDimSize[0] + j] * 10000));
      if (j)
        LINE(dc,pt);
      else
        MOVE(dc,pt);
    }
    dc.RestorePen();
  }
}
