/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  SensorClasses.cc                                      ****
 **** Inhalt: Implementation der Datentypen zu den Sensorklassen    ****
 ****                                                               ****
 ***********************************************************************/

#include "SensorClasses.h"
#include "sim3DErrors.h"
#include "sim3DWorld.h"
#include "Vector.h"
#include <math.h>
#include <stdio.h>

inline REAL max (REAL a, REAL b)
{
  return (a < b ? b : a);
}

inline INTEGER max (INTEGER a, INTEGER b)
{
  return (a < b ? b : a);
}

// SENSCLASSVECTOR: Siehe auch "SensorClasses.h"

SENSCLASSVECTOR::SENSCLASSVECTOR (INTEGER size, SHORTREAL deflt)
{
  Size = size;
  Default = deflt;
  if (Size > 0)
  {
    Data = new SHORTREAL [Size];
    for (INTEGER i=0;i<Size;i++)  Data [i] = Default;
  }
  else
    Data = 0;
}

SENSCLASSVECTOR::SENSCLASSVECTOR (const SENSCLASSVECTOR& scv)
{
  Size = scv.Size;
  Default = scv.Default;
  if (Size > 0)
  {
    Data = new SHORTREAL [Size];
    for (INTEGER i=0;i<Size;i++)  Data [i] = scv.Data [i];
  }
  else
    Data = 0;
}

SENSCLASSVECTOR::~SENSCLASSVECTOR ()
{
  if (Size > 0) delete [] Data;
}

SHORTREAL& SENSCLASSVECTOR::operator [] (INTEGER i)
{
  if (i < 0 || i >= Size)
    sim3DFatalError (sim3DOutOfRange);
  return Data [i];
}

SHORTREAL SENSCLASSVECTOR::Index (INTEGER i) const
{
  if ((i >= 0) && (i < Size))
    return Data [i];
  else
    return Default;
}

void SENSCLASSVECTOR::Set (INTEGER i,SHORTREAL s)
{
  if (i < 0)
    sim3DFatalError (sim3DOutOfRange);
  else if (i >= Size) {
    INTEGER j;
    SHORTREAL* temp = new SHORTREAL [i+1];
    if (Size > 0) {
      for (j = 0; j < Size; j++) temp[j] = Data[j];
      delete [] Data;
    }
    Data = temp;
    for (j = Size; j < i; j++) Data[j] = Default;
    Size = i+1;
  }
  Data [i] = s;
}

SENSCLASSVECTOR SENSCLASSVECTOR::operator * (const REAL x) const
{
  SENSCLASSVECTOR result (*this);
  result.Default *= (SHORTREAL) x;
  for (INTEGER i=0;i<Size;i++)  result [i] *= (SHORTREAL) x;
  return result;
}

SENSCLASSVECTOR SENSCLASSVECTOR::operator * (const SENSCLASSVECTOR& scv) const
{
  INTEGER mSize = max (Size, scv.Size);
  SENSCLASSVECTOR result (mSize, Default * scv.Default);
  for (INTEGER i=0;i<mSize;i++)
    result [i] = Index (i) * scv.Index (i);
  return result;
}

SENSCLASSVECTOR SENSCLASSVECTOR::operator + (const SENSCLASSVECTOR& scv) const
{
  INTEGER mSize = max (Size, scv.Size);
  SENSCLASSVECTOR result (mSize, Default + scv.Default);
  for (INTEGER i=0;i<mSize;i++)
    result [i] = Index (i) + scv.Index (i);
  return result;
}

SENSCLASSVECTOR& SENSCLASSVECTOR::operator *= (const SENSCLASSVECTOR& scv)
{
  INTEGER mSize = max (Size, scv.Size);
  for (INTEGER i = mSize - 1; i >= 0; i--)
    Set(i,SHORTREAL(Index (i) * scv.Index (i)));
  Default *= scv.Default;
  return *this;
}

void SENSCLASSVECTOR::MulAndAdd(const SENSCLASSVECTOR& scv,REAL factor)
{
  INTEGER mSize = max (Size, scv.Size);
  for (INTEGER i = mSize - 1;i >= 0;i--)
    Set(i,SHORTREAL (Index(i) + scv.Index (i) * factor));
  Default *= SHORTREAL (scv.Default * factor);
}

void SENSCLASSVECTOR::operator = (const SENSCLASSVECTOR& scv)
{
  if (&scv != this) {
    if (Size != scv.Size) {
      if (Size > 0) delete [] Data;
      Size = scv.Size;
      if(Size > 0)
        Data = new SHORTREAL [Size];
      else
        Data = 0;
    }
    Default = scv.Default;
    for (INTEGER i=0;i<Size;i++)  Data [i] = scv.Data [i];
  }
}

void SENSCLASSVECTOR::ReadVector (SCANNER& scan)
{
  if (scan.TestFor (sLParen, sReal)) {
    if (Size > 0) {
      delete [] Data;
      Size = 0;
    }
    do {
      scan.NextSymbol ();
      if ((scan.Symbol == sReal) || (scan.Symbol == sInteger)) {
        Set (Size,(SHORTREAL) scan.GetLastReal ());
        scan.NextSymbol ();
      }
    } while (scan.Symbol == sComma);
    scan.Expect (sRParen, sSemicolon);
  }
}

void SENSCLASSVECTOR::WriteVector (BACKUP& backup)
{
  backup.StartList (sLParen,sComma,sRParen,TRUE);
  for (INTEGER i=0;i<Size;i++) {
    backup.WriteReal (Data [i]);
    backup.ListNext ();
  }
  backup.EndList ();
}

//-- TEXTUREDESCR ---------------------------------------------------------

TEXTUREDESCR::TEXTUREDESCR (const STRING& name) :
  Name (name)
{
  Next = NULL;
}

TEXTUREDESCR::~TEXTUREDESCR ()
{
}

void TEXTUREDESCR::WriteProperties (BACKUP& backup)
{
  backup.WriteString (Name);
}

void TEXTUREDESCR::GetSize (INTEGER& planes, REAL& width, REAL& height)
{
  planes = 1;
  width = 1.0;
  height = 1.0;
}

SHORTREAL TEXTUREDESCR::GetValueAt (INTEGER, REAL, REAL)
{
  return 1.0F;
}

//-- SPECIALTEXTURE -------------------------------------------------------

class SPECIALTEXTURE : public TEXTUREDESCR
{
public:
  INTEGER Nr;

  SPECIALTEXTURE (STRING,INTEGER);
  virtual void      WriteProperties (BACKUP& backup);
  virtual SHORTREAL GetValueAt (INTEGER plane, REAL x, REAL y);
};

SPECIALTEXTURE::SPECIALTEXTURE (STRING name, INTEGER nr) :
  TEXTUREDESCR (name)
{
  Nr = nr;
}

void SPECIALTEXTURE::WriteProperties (BACKUP& backup)
{
  TEXTUREDESCR::WriteProperties (backup);
  backup.WriteIdent ("SPECIAL");
  backup.WriteInteger (Nr);
}

SHORTREAL SPECIALTEXTURE::GetValueAt (INTEGER, REAL x, REAL y)
{
  switch (Nr) {
    case 0:
      if ((x >= 0.4 && x < 0.6) || (y >= 0.4 && y < 0.6))
	return 1.0F;
      else
	return 0.0F;
    case 1:
      return (SHORTREAL) ((1+sin(x*2*PI)) * 0.5);
    case 2:
      x -= .5;
      y -= .5;
      return (SHORTREAL) (1-cos(x*y*2*PI));
    default:
      return 1.0F;
  }
}

#if !defined(__WIN16__) && !defined(__WIN32__) && !defined(__OS2__)
//-- BITMAPTEXTURE ---------------------------------------------------------

typedef unsigned char BYTE;

const MAGIC = 0x4f5a5101;

#define TRUE_RED(PIXVAL)   (((unsigned long)((PIXVAL) & 0xff0000)) >> 16)
#define TRUE_GREEN(PIXVAL) (((unsigned long)((PIXVAL) & 0xff00)) >> 8)
#define TRUE_BLUE(PIXVAL)   ((unsigned long)((PIXVAL) & 0xff))

#define memToVal(PTR,LEN) (                                   \
(LEN) == 1 ? (unsigned long)(                 *( (BYTE *)(PTR))         ) :    \
(LEN) == 2 ? (unsigned long)(((unsigned long)(*( (BYTE *)(PTR))   ))<< 8)      \
                          + (                 *(((BYTE *)(PTR))+1)      ) :    \
(LEN) == 3 ? (unsigned long)(((unsigned long)(*( (BYTE *)(PTR))   ))<<16)      \
                          + (((unsigned long)(*(((BYTE *)(PTR))+1)))<< 8)      \
                                                  + (                 *(((BYTE *)(PTR))+2)      ) :    \
             (unsigned long)(((unsigned long)(*( (BYTE *)(PTR))   ))<<24)      \
                                                  + (((unsigned long)(*(((BYTE *)(PTR))+1)))<<16)      \
                                                  + (((unsigned long)(*(((BYTE *)(PTR))+2)))<< 8)      \
                                                  + (                 *(((BYTE *)(PTR))+3)      ) )

class RGBValue 
{
public:
  unsigned char Red, Green, Blue;

  RGBValue (INTEGER r, INTEGER g, INTEGER b)
    { Red = r; Green = g; Blue = b; };
};

class BITMAPTEXTURE : public TEXTUREDESCR
{
public:
  STRING   FileName;
  LONGINT  Width, Height;
  LONGINT  ColorsUsed, BytePerPixel, Size;
  LONGINT* ColorData;
  BYTE*    Data;

  BITMAPTEXTURE (STRING,STRING);
  virtual           ~BITMAPTEXTURE();
  virtual void      WriteProperties (BACKUP& backup);
  virtual void      GetSize (INTEGER& planes, REAL& width, REAL& height);
          RGBValue  GetRGBValue (INTEGER x, INTEGER y);
  virtual SHORTREAL GetValueAt (INTEGER plane, REAL x, REAL y);
};

BOOLEAN FReadLong (FILE* f, LONGINT& l)
{
  unsigned char buf[4];
  BOOLEAN result = fread (buf, 4, 1, f) == 1;
  l = (LONGINT) buf[0] << 24 |
      (LONGINT) buf[1] << 16 |
      (LONGINT) buf[2] << 8 |
      (LONGINT) buf[3];
  return result;
}

BITMAPTEXTURE::BITMAPTEXTURE (STRING name, STRING filename) :
  TEXTUREDESCR (name),
  FileName (filename)
{
  Width = 0;
  Height = 0;
  ColorsUsed = 0;
  Size = 0;
  BytePerPixel = 0;
  ColorData = NULL;
  Data = NULL;

  BOOLEAN popenUsed = FALSE;
  BOOLEAN ok = FALSE;

  FILE* f = fopen ((const char*) filename, "r");
  LONGINT magic;
  if (f != NULL && FReadLong (f, magic)) {
    if (magic != MAGIC) {
      fclose (f);
      STRING h = (STRING) "SRTextureConvert " + filename;
      f = popen ((const char*) h, "r");
      popenUsed = TRUE;
      ok = (f != NULL && FReadLong (f, magic) && magic == MAGIC);
    }
    else
      ok = TRUE;
  }
  else 
    ok = FALSE;

  if (ok) {
    if (FReadLong (f, BytePerPixel) &&
        FReadLong (f, Width) && FReadLong (f, Height) &&
        FReadLong (f, ColorsUsed) && FReadLong (f, Size)) {
      ColorData = new LONGINT [ColorsUsed];
      Data = new BYTE [Size];
      if (fread (ColorData, sizeof (*ColorData), ColorsUsed, f) == ColorsUsed &&
          fread (Data, sizeof (*Data), Size, f) == Size)
        return;
    }
  }
  else
    sim3DOtherError (sim3DInvalidTextureFile);
  if (popenUsed)
    pclose (f);
  else
    fclose (f);
}

BITMAPTEXTURE::~BITMAPTEXTURE ()
{
  if (ColorsUsed > 0) delete [] ColorData;
  if (Size > 0) delete [] Data;
}

void BITMAPTEXTURE::WriteProperties (BACKUP& backup)
{
  TEXTUREDESCR::WriteProperties (backup);
  backup.WriteIdent ("LOAD");
  backup.WriteString (FileName);
}

void BITMAPTEXTURE::GetSize (INTEGER& planes, REAL& width, REAL& height)
{
  planes = 4;
  width = Width;
  height = Height;
}

RGBValue BITMAPTEXTURE::GetRGBValue (INTEGER x, INTEGER y)
{
  if (BytePerPixel == 0) {
    if (Data [((Width+7)/8)*y + x/8] & (0x80 >> (x % 8)))
      return RGBValue (255,255,255);
    else
      return RGBValue (0,0,0);
  }
  else {
    LONGINT val = memToVal (&(Data [(Width*y+x)*BytePerPixel]), BytePerPixel);
    if (BytePerPixel < 3) val = ColorData [val];
    return RGBValue (TRUE_RED(val), TRUE_GREEN(val), TRUE_BLUE(val));
  }
}

SHORTREAL BITMAPTEXTURE::GetValueAt (INTEGER plane, REAL x, REAL y)
{
  RGBValue rgb = GetRGBValue ((INTEGER) (x * Width), (INTEGER) (y * Height));
  switch (plane) {
    case 1: return (SHORTREAL) rgb.Red   / 256.0;
    case 2: return (SHORTREAL) rgb.Green / 256.0;
    case 3: return (SHORTREAL) rgb.Blue  / 256.0;
    default: return (SHORTREAL) (rgb.Red*77 + rgb.Green*151 + rgb.Blue*28) 
                                 / 65536.0;
  }
}

#else
#include "NonPortable.h"
#endif // !__WIN16__ && !__WIN32__ && !__OS2__

//-- TextureLoader --------------------------------------------------------

TEXTUREDESCR* TextureLoader (SCANNER& scan)
{
  STRING name;
  STRING filename;
  INTEGER i;

  if (GetCorrectString (scan, name))
    if (scan.Symbol == sIdent)
      if (scan.GetLastString() == "SPECIAL") {
        scan.NextSymbol ();
        if (GetCorrectInteger (scan, i)) 
          return new SPECIALTEXTURE (name, i);
      }
      else if (scan.GetLastString() == "LOAD") {
        scan.NextSymbol ();
        if (GetCorrectString (scan, filename))
          return new BITMAPTEXTURE (name, filename);
      }
      else 
        scan.SyntaxError (sim3DUnknownTextureType1, scan.GetLastString(),
                          sSemicolon);
    else
      scan.SyntaxError (sim3DExpectTextureType, sSemicolon);
  return NULL;
}

//-- SURFACEDESCR ---------------------------------------------------------

SURFACEDESCR::SURFACEDESCR (const STRING& name) :
  Name (name),
  Color          (0,0.5F),
  TextureBgColor (0,0.0F),
  TextureFgColor (0,1.0F),
  scvTemp        (0,0.0F)
{
  Next       = NULL;
  Texture    = NULL;
  Width      = 0;
  Height     = 0;
  AutoSize   = TRUE;
  WrapAround = FALSE;
  NotShading = FALSE;
}

BOOLEAN GetCorrectTextureVal (SCANNER& scan, INTEGER& plane, REAL& a, REAL& b)
{
  if (! scan.TestFor (sLParen, sComma)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sInteger, sRParen)) return FALSE;
  plane = (INTEGER) scan.GetLastInt ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  a = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  b = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sRParen, sSemicolon)) return FALSE;
  scan.NextSymbol ();
  return TRUE;
}

void SURFACEDESCR::ReadProperties (SCANNER& scan, SIM3DWorld* World)
{
  Color.ReadVector (scan);
  if (scan.Symbol == sLBracket) {
    do {
      scan.NextSymbol ();
      if (scan.Symbol == sRBracket) break;
      if (scan.TestFor (sIdent, sSemicolon)) {
        STRING attr = scan.GetLastString ();
        scan.NextSymbol ();
        ReadAttr (attr, scan, World);
      }
    } while (scan.Symbol == sSemicolon);
    scan.Expect (sRBracket, sSemicolon);
  }
}

void SURFACEDESCR::ReadAttr (STRING& attr, SCANNER& scan, SIM3DWorld* World)
{
  REAL r1, r2;
  STRING n;
  INTEGER p;

  if (attr == "NOT_SHADING")
    NotShading = TRUE;

  else if (attr == "WRAPAROUND")
     WrapAround = TRUE;

  else if (attr == "SIZE" || attr == "RATIO") {
    if (GetCorrectRealPair (scan, r1, r2)) {
      AutoSize = (attr == "RATIO");
      Width    = r1;
      Height   = r2;
    }
  }
  else if (attr == "TEXTURE") {
    if (GetCorrectString (scan, n)) {
      Texture = World->FindTexture (n);
      if (Texture == NULL) 
        scan.SyntaxError (sim3DUnknownTexture1, n);
      else
        Texture->GetSize (p, r1, r2);
    }
    INTEGER i = 0;
    if (scan.TestFor (sLParen, sSemicolon)) 
    do {
      scan.NextSymbol ();
      INTEGER planenr;
      if (scan.Symbol == sInteger) {
        planenr = (INTEGER) scan.GetLastInt ();
        if (planenr >= p) scan.SyntaxError (sim3DHighTextureNumber);
        TexturePlaneNr.AddElement (planenr);
        scan.NextSymbol ();
      }
      else if (scan.Symbol == sLParen) {
        if (GetCorrectTextureVal (scan, planenr, r1, r2)) {
          if (planenr >= p) scan.SyntaxError (sim3DHighTextureNumber);
          TexturePlaneNr.AddElement (planenr);
          TextureBgColor.Set(i,(SHORTREAL) r1);
          TextureFgColor.Set(i,(SHORTREAL) r2);
        }
      }
      else
        TexturePlaneNr.AddElement (-1);
      i++;
    } while (scan.Symbol == sComma);
    scan.Expect (sRParen, sSemicolon);
  }        
}

void SURFACEDESCR::WriteProperties (BACKUP& backup)
{
  Color.WriteVector (backup);
  backup.StartListI (sLBracket, sSemicolon, sRBracket, FALSE);

    if (Texture != NULL) {
      backup.WriteIdent ("TEXTURE");
      backup.WriteString (Texture->Name);
      backup.StartList (sLParen, sComma, sRParen, FALSE);
      for (INTEGER i=0;i<TexturePlaneNr.HowMany();i++) { 
        if (TexturePlaneNr [i] >= 0) {
          backup.WriteSymbol (sLParen);
          backup.WriteInteger (TexturePlaneNr [i]);
          backup.WriteSymbol (sComma);
          backup.WriteReal (TextureBgColor.Index(i));
          backup.WriteSymbol (sComma);
          backup.WriteReal (TextureFgColor.Index(i));
          backup.WriteSymbol (sRParen);
        }
        else
          backup.WriteSymbol (sNull);
        backup.ListNext ();
      }
      backup.EndList ();
      backup.ListNext ();
    }
    
    if (Width != 0 || Height != 0) {
      if (AutoSize)
        backup.WriteIdent ("RATIO");
      else
        backup.WriteIdent ("SIZE");
      backup.WriteSymbol (sLParen);
      backup.WriteReal (Width);
      backup.WriteSymbol (sComma);
      backup.WriteReal (Height);
      backup.WriteSymbol (sRParen);
      backup.ListNext ();
    }
  
    if (WrapAround) {
      backup.WriteIdent ("WRAPAROUND");
      backup.ListNext ();
    }

    if (NotShading) {
      backup.WriteIdent ("NOT_SHADING");
      backup.ListNext ();
    }

  backup.EndList ();
}

BOOLEAN SURFACEDESCR::HasTexture ()
{
  return (Texture != NULL);
}

BOOLEAN SURFACEDESCR::HasTexture (INTEGER cl)
{
  return (Texture != NULL && cl >= 0 && cl < TexturePlaneNr.HowMany() &&
          TexturePlaneNr [cl] >= 0);
}

SENSCLASSVECTOR& SURFACEDESCR::GetColorAt (REAL x, REAL y)
{
  SENSCLASSVECTOR result = Color;
  if (Texture != NULL && 
      ((x >= -0.5 && y >= -0.5 && x < 0.5 && y < 0.5) || WrapAround)) {
    scvTemp = Color;
    x += 0.5;
    y += 0.5;
    if (WrapAround) {
      x = x - floor (x);
      y = y - floor (y);
    }
    for (INTEGER i=0;i<TexturePlaneNr.HowMany();i++) 
      if (TexturePlaneNr [i] >= 0) {
        REAL r = TextureBgColor.Index(i) + (TextureFgColor.Index(i) - 
                 TextureBgColor.Index(i))
                 * Texture->GetValueAt (TexturePlaneNr [i], x, y);
        scvTemp.Set(i,(SHORTREAL) r);
      }
    return scvTemp;
  }
  else
    return Color;
}

void SURFACEDESCR::GetSizeOrRatio (REAL& x, REAL& y, BOOLEAN& autosize)
{
  x = Width;
  y = Height;
  autosize = AutoSize;
}
