//
// Datei: NonPortable
// Zweck: Anpassung an MS Windows und OS/2 unter Verwendung der
//        Object Windows Library
// Datum: 20.3.95
//

#ifndef _NONPORTABLE_H_
#define _NONPORTABLE_H_

#include "HiddenGraph.h"
#include "SensorClasses.h"
#define COLOR COLOR2
#define BOOLEAN BOOLEAN2
#define COORD COORD2
#include <OWL/dc.h>
#undef COLOR
#undef BOOLEAN
#undef COORD

class OWLGRAPH : public HIDDENGRAPH
{
  TDC*  m_pDC;
  TPen* m_pPen;
  COLOR m_lastColor;
  COLORLIST* m_pColorList;
  void SetColor ();
public:
  OWLGRAPH (TDC* pDC);
  ~OWLGRAPH ();
  virtual void RegisterColors (COLORLIST&);
  virtual void SetLine  (const COORD&, const COORD&, COLOR);
};

#ifndef __OS2__
class BITMAPTEXTURE : public TEXTUREDESCR
{
private:
  STRING  m_sFileName;
  int     m_xSize,
          m_ySize,
          m_hDIB;
  void    ReadDIB();
  BOOLEAN CalcSize();
public:
  BITMAPTEXTURE     (const STRING&,const STRING&);
  virtual           ~BITMAPTEXTURE();
  virtual void      WriteProperties (BACKUP& backup);
  virtual void      GetSize (INTEGER& planes, REAL& width, REAL& height);
  virtual SHORTREAL GetValueAt (INTEGER plane, REAL x, REAL y);
};
#else //!__OS2__
class BITMAPTEXTURE : public TEXTUREDESCR
{
private:
  TDib*   m_pDib;
  TDibDC* m_pDC;
  STRING  m_sFileName;
public:
  BITMAPTEXTURE (const STRING&,const STRING&);
  virtual ~BITMAPTEXTURE();
  virtual void WriteProperties (BACKUP& backup);
  virtual void GetSize (INTEGER& planes, REAL& width, REAL& height);
  virtual SHORTREAL GetValueAt (INTEGER plane, REAL x, REAL y);
};
#endif // !__OS2__
#endif
