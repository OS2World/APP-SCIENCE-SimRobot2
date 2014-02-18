//
// Datei: NonPortable.cpp
// Zweck: Anpassung an MS Windows und OS/2 der auf Basis der
//        Object Windows Library
// Datum: 20.3.95
//

#include "pch.h"
#include <classlib\file.h>
#include <limits.h>
#include "sim3DErrors.h"
#include "NonPortable.h"

//////////////////////////////////////////////////////////////////////////
// class OWLGRAPH: Windows Implementation von Hiddengraph

OWLGRAPH::OWLGRAPH(TDC* pDC)
{
  m_pDC = pDC;
  m_lastColor = -1;
  m_pPen = new TPen(TColor(0,0,0),0);
  m_pColorList = 0;
}

OWLGRAPH::~OWLGRAPH()
{
  m_pDC->RestorePen();
  delete m_pPen;
}

void OWLGRAPH::RegisterColors(COLORLIST& ColorList)
{
  m_pColorList = &ColorList;
}

void OWLGRAPH::SetColor()
{
  m_pDC->RestorePen();
  delete m_pPen;
  if(m_pColorList)
  {
    m_pColorList->NthColor(m_lastColor);
    if(m_pColorList->RGBDefined())
    {
      VECTOR v = m_pColorList->ActualRGB();
      m_pPen = new TPen(TColor(v.x * 255,v.y*255,v.z*255),0);
      m_pDC->SelectObject(*m_pPen);
      return;
    }
  }
  PALETTEENTRY pe;
  m_pDC->GetSystemPaletteEntries(m_lastColor,1,
                                 (PALETTEENTRY*) &pe);
  m_pPen = new TPen(TColor(pe),0);
  m_pDC->SelectObject(*m_pPen);
}

void OWLGRAPH::SetLine(const COORD& c1,const COORD& c2,COLOR color)
{
  if (color != m_lastColor)
  {
    m_lastColor = color;
    SetColor ();
  }
  if(c1.x >= SHRT_MIN && c1.x <= SHRT_MAX &&
     c1.y >= SHRT_MIN && c1.y <= SHRT_MAX &&
     c2.x >= SHRT_MIN && c2.x <= SHRT_MAX &&
     c2.y >= SHRT_MIN && c2.y <= SHRT_MAX)
  {
    MOVE(*m_pDC,TPoint((int) (c1.x+0.5),(int) (c1.y+0.5)));
    LINE(*m_pDC,TPoint((int) (c2.x+0.5),(int) (c2.y+0.5)));
  }
}

#ifndef __OS2__
//////////////////////////////////////////////////////////////////////////
// Teile aus dem DIBAPI. Wird von BITMAPTEXTURE benoetigt.

DECLARE_HANDLE(HDIB);
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

WORD WINAPI DIBNumColors(LPSTR lpbi)
{
  WORD wBitCount;

  if (IS_WIN30_DIB(lpbi))
  {
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
    if (dwClrUsed != 0)
      return (WORD)dwClrUsed;
  }
  if (IS_WIN30_DIB(lpbi))
    wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
  else
    wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
  switch (wBitCount)
  {
    case 1:
      return 2;
    case 4:
      return 16;
    case 8:
      return 256;
    default:
      return 0;
  }
}

WORD WINAPI PaletteSize(LPSTR lpbi)
{
  if (IS_WIN30_DIB (lpbi))
   return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBQUAD));
  else
   return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}

LPSTR WINAPI FindDIBBits(LPSTR lpbi)
{
  return (lpbi + *(LPDWORD)lpbi + ::PaletteSize(lpbi));
}

DWORD WINAPI DIBWidth(LPSTR lpDIB)
{
  LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
  LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB
  lpbmi = (LPBITMAPINFOHEADER)lpDIB;
  lpbmc = (LPBITMAPCOREHEADER)lpDIB;
  if (IS_WIN30_DIB(lpDIB))
    return lpbmi->biWidth;
  else
    return (DWORD)lpbmc->bcWidth;
}

DWORD WINAPI DIBHeight(LPSTR lpDIB)
{
  LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
  LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB
  lpbmi = (LPBITMAPINFOHEADER)lpDIB;
  lpbmc = (LPBITMAPCOREHEADER)lpDIB;
  if (IS_WIN30_DIB(lpDIB))
    return lpbmi->biHeight;
  else
    return (DWORD)lpbmc->bcHeight;
}

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

HDIB WINAPI ReadDIBFile(TFile& file)
{
  BITMAPFILEHEADER bmfHeader;
  DWORD dwBitsSize;
  HDIB hDIB;
  LPSTR pDIB;
  dwBitsSize = file.Length();
  if ((file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) !=
    sizeof(bmfHeader)) || (bmfHeader.bfType != DIB_HEADER_MARKER))
  {
    return NULL;
  }
  hDIB = (HDIB) ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize);
  if (hDIB == 0)
  {
    return NULL;
  }
  pDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
  if (file.Read(
#ifdef __WIN16__
  (void __huge*)
#else
  (void*)
#endif
  pDIB, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
    dwBitsSize - sizeof(BITMAPFILEHEADER) ||
    ((LPBITMAPINFOHEADER) pDIB)->biCompression != BI_RGB)
  {
    ::GlobalUnlock((HGLOBAL) hDIB);
    ::GlobalFree((HGLOBAL) hDIB);
    return NULL;
  }
  ::GlobalUnlock((HGLOBAL) hDIB);
  return hDIB;
}

#ifdef __WIN16__
#define puc unsigned char __huge *
#else
#define puc unsigned char *
#endif

RGBQUAD GetDIBPixel(HDIB hDIB,int x,int y)
{
  LPBITMAPINFO pInfo = (LPBITMAPINFO) ::GlobalLock((HGLOBAL) hDIB);
  LPBITMAPINFOHEADER pHdr = (LPBITMAPINFOHEADER) &(pInfo->bmiHeader);
  puc p;
  puc pBits = (puc) ::FindDIBBits((LPSTR) pHdr);
  RGBQUAD rgb,
         *pColors = (RGBQUAD*) ((char*) pInfo + pHdr->biSize);
  long nSize = WIDTHBYTES(pHdr->biWidth*pHdr->biBitCount);
  int i;
  switch (pHdr->biBitCount)
  {
  case 1:
    i = (*(pBits + nSize*y+x/8) >> (7 - (x & 7))) & 1;
    break;
  case 4:
    i = (*(pBits + nSize*y+x/2) >> (1 - (x & 1))*4) & 15;
    break;
  case 8:
    i = *(pBits + nSize*y+x);
    break;
  case 24:
    p = pBits + nSize*y+x*3;
    rgb.rgbRed = *p;
    rgb.rgbGreen = *(p+1);
    rgb.rgbBlue = *(p+2);
    ::GlobalUnlock((HGLOBAL) hDIB);
    return rgb;
  }
  rgb = pColors[i];
  ::GlobalUnlock((HGLOBAL) hDIB);
  return rgb;
}

//////////////////////////////////////////////////////////////////////////
// class BITMAPTEXTURE: Windows Implementation von Bitmap-Texturen

void BITMAPTEXTURE::ReadDIB()
{
  m_hDIB = 0;
  TFile file;
  if (!file.Open((const char *) m_sFileName,O_RDONLY,SH_DENYWR))
    sim3DOtherError (sim3DInvalidTextureFile);
  else
  {
    m_hDIB = (unsigned) ::ReadDIBFile(file);
    file.Close();
    if (!m_hDIB)
      sim3DOtherError (sim3DInvalidTextureFile);
  }
}

BOOLEAN BITMAPTEXTURE::CalcSize()
{
  LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
  if (::DIBWidth(lpDIB) > INT_MAX ||::DIBHeight(lpDIB) > INT_MAX)
  {
    ::GlobalUnlock((HGLOBAL) m_hDIB);
    sim3DOtherError (sim3DInvalidTextureFile);
    return FALSE;
  }
  else
  {
    m_xSize = (int) ::DIBWidth(lpDIB);
    m_ySize = (int) ::DIBHeight(lpDIB);
    ::GlobalUnlock((HGLOBAL) m_hDIB);
    return TRUE;
  }
}

BITMAPTEXTURE::BITMAPTEXTURE (const STRING& name, const STRING& filename) :
  TEXTUREDESCR (name)
{
  m_sFileName = filename;
  ReadDIB();
  if (m_hDIB)
  {
    if (!CalcSize())
    {
      ::GlobalFree((HGLOBAL) m_hDIB);
      m_hDIB = 0;
    }
  }
}

BITMAPTEXTURE::~BITMAPTEXTURE ()
{
  if (m_hDIB)
    ::GlobalFree((HGLOBAL) m_hDIB);
}

void BITMAPTEXTURE::WriteProperties (BACKUP& backup)
{
  TEXTUREDESCR::WriteProperties (backup);
  backup.WriteIdent ("LOAD");
  backup.WriteString (m_sFileName);
}

void BITMAPTEXTURE::GetSize (INTEGER& planes, REAL& width, REAL& height)
{
  planes = 4;
  width = m_xSize;
  height = m_ySize;
}

SHORTREAL BITMAPTEXTURE::GetValueAt (INTEGER plane, REAL x, REAL y)
{
  RGBQUAD rgb = GetDIBPixel((HDIB) m_hDIB,(INTEGER) (x * (m_xSize-1)),
                                          (INTEGER) ((1-y) * (m_ySize-1)));
  switch (plane)
  {
    case 1: return (SHORTREAL) (rgb.rgbRed / 256.0);
    case 2: return (SHORTREAL) (rgb.rgbGreen / 256.0);
    case 3: return (SHORTREAL) (rgb.rgbBlue / 256.0);
    default: return (SHORTREAL) ((rgb.rgbRed * 77.0 +
                                  rgb.rgbGreen * 151.0 +
                                  rgb.rgbBlue * 28.0) / 65536.0);
  }
}

#else // !__OS2__

//////////////////////////////////////////////////////////////////////////
// class BITMAPTEXTURE: OWL Implementation von Bitmap-Texturen

BITMAPTEXTURE::BITMAPTEXTURE(const STRING& name,const STRING& filename)
: TEXTUREDESCR (name)
{
  m_sFileName = filename;
  try
  {
    m_pDib = new TDib(m_sFileName);
    m_pDC = new TDibDC(*m_pDib);
  }
  catch(TGdiBase::TXGdi& xGdi)
  {
    m_pDib = 0;
    m_pDC = 0;
    sim3DOtherError(sim3DInvalidTextureFile);
  }
}

BITMAPTEXTURE::~BITMAPTEXTURE()
{
  delete m_pDC;
  delete m_pDib;
}

void BITMAPTEXTURE::WriteProperties (BACKUP& backup)
{
  TEXTUREDESCR::WriteProperties (backup);
  backup.WriteIdent ("LOAD");
  backup.WriteString (m_sFileName);
}

void BITMAPTEXTURE::GetSize (INTEGER& planes, REAL& width, REAL& height)
{
  planes = 4;
  if(m_pDib)
  {
    width = m_pDib->Width();
    height = m_pDib->Height();
  }
  else
  {
    width = 1;
    height = 1;
  }
}

SHORTREAL BITMAPTEXTURE::GetValueAt (INTEGER plane, REAL x, REAL y)
{
  TColor rgb = m_pDC->GetPixel((INTEGER) (x * (m_pDib->Width()-1)),
			       (INTEGER) (y * (m_pDib->Height()-1)));
  switch (plane)
  {
    case 1: return (SHORTREAL) (rgb.Red() / 256.0);
    case 2: return (SHORTREAL) (rgb.Green() / 256.0);
    case 3: return (SHORTREAL) (rgb.Blue() / 256.0);
    default: return (SHORTREAL) ((rgb.Red() * 77.0 +
				  rgb.Green() * 151.0 +
				  rgb.Blue() * 28.0) / 65536.0);
  }
}
#endif // !__OS2__
