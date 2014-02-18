/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  ZBuffer.cc                                            ****
 **** Inhalt: Implementierung einer Klasse ZBUFFER zur alternativen ****
 ****         Kamerabild-Berechnung                                 ****
 ****                                                               ****
 ***********************************************************************/

#include "ZBuffer.h"
#include "Vector.h"
#include "sim3DBody.h"
#include "sim3DErrors.h"
#include <math.h>

class zBufElem            // Ein Pixel-Element des z-Buffer-Algorithmus
{
public:
  SHORTREAL     Depth,    // Tiefe
					 TextureX, // Texturkoordinaten
					 TextureY;
  SIM3DElement* Body;     // Dargestelltes Objekt
  INTEGER       FaceNr;   // Flaechennummer
#if defined(__WIN16__)
  char          _pad[26-3*sizeof(SHORTREAL)];   // fuer __huge
#endif // __WIN16__
};

ZBUFFER::ZBUFFER (INTEGER awidth, INTEGER aheight)
{
  VertexCount  = 0;
  PolygonStart = 0;
  Width        = awidth;
  Height       = aheight;
  Size         = ((LONGINT) Width) * ((LONGINT) Height);
  Pixel        = ALLOC_BLOCKS(Size,zBufElem);
  ViewPoint          = VECTOR (0,0,0);
  ViewPlaneCorner    = VECTOR (-1,-1,1);
  ViewPlaneHorizEdge = VECTOR (1,0,0);
  ViewPlaneVertEdge  = VECTOR (0,1,0);
  ViewPlaneNormal    = VECTOR (0,0,1);
  ViewPlaneDist      = 1;
}

ZBUFFER::~ZBUFFER ()
{
  FREE_BLOCKS(Pixel);
}

void ZBUFFER::SetViewGeometry (const VECTOR& vpoint, const VECTOR& vpcorner,
                  const VECTOR& vphorizedge, const VECTOR& vpvertedge,
                  REAL maxdepth)
// Festlegung der Sichtpyramide mit dem Sichtpunkt, der linken, oberen Ecke
// der Sichtflaeche, der horizontalen Kante, der vertikalen Kante und der
// maximalen Sichttiefe
{
  ViewPoint = vpoint;
  ViewPlaneCorner = vpcorner;
  ViewPlaneHorizEdge = vphorizedge;
  ViewPlaneVertEdge = vpvertedge;
  ViewPlaneNormal = CrossProduct (vphorizedge, vpvertedge);
  (void) ViewPlaneNormal.Norm ();
  ViewPlaneDist = vpcorner * ViewPlaneNormal;
  VertexCount = 0;
  PolygonStart = 0;

  // Initialisierung des Tiefenpuffers  
  LONGINT i;
  pzBufElem pix = Pixel;
  for (i=0;i<Size;i++) {
    pix->Depth    = (SHORTREAL) maxdepth;
    pix->TextureX = 0.0F;
    pix->TextureY = 0.0F;
    pix->Body     = NOELEMENT;
    pix->FaceNr   = 0;
    pix++;
  }
}

void ZBUFFER::NewFaceVertex (const VECTOR& v)
// Starten eines neuen Kantezuges durch Angabe des letzten Punktes
{
  if (PolygonStart < VertexCount) 
    NextVertex [VertexCount-1] = PolygonStart;  // Voriges Polygon schliessen
  PrevVertex = v;
  PolygonStart = VertexCount;
  PrevVisible = (PrevVertex*ViewPlaneNormal - ViewPlaneDist > 0);
}

void ZBUFFER::AddFaceVertex (const VECTOR& thisvertex)
// Weiterfuehren eines Kantenzuges durch Angabe des naechsten Punktes,
// automatisches Clipping an der Sichtebene
{
  REAL thisdist = thisvertex*ViewPlaneNormal - ViewPlaneDist;
  BOOLEAN thisvisible = (thisdist > 0);
  if (thisvisible != PrevVisible) {
    VECTOR diff = PrevVertex-thisvertex;
    REAL totallen = diff*ViewPlaneNormal;
    if (abs (totallen) > EPSILON) 
      AddVertex ((-thisdist/totallen)*diff + thisvertex);
  }
  if (thisvisible)
    AddVertex (thisvertex);
  PrevVisible = thisvisible;
  PrevVertex  = thisvertex;
}

void ZBUFFER::AddVertex (const VECTOR& v)
// Eintragen eines Punktes des geclippten Polygons in Sichtkoordinaten
{
  VECTOR diff1 = v - ViewPoint;
  VECTOR diff2 = ViewPoint - ViewPlaneCorner;
  REAL Det = det (ViewPlaneHorizEdge, ViewPlaneVertEdge, diff1);
  if (abs (Det) > EPSILON) {
    VXBuffer [VertexCount] = det (diff2, ViewPlaneVertEdge, diff1) / Det;
    VYBuffer [VertexCount] = det (ViewPlaneHorizEdge, diff2, diff1) / Det;
    NextVertex [VertexCount] = VertexCount+1;
    VertexCount++;
    if (VertexCount == MaxzBufVertices) sim3DFatalError (sim3DOverflow);
  }
}

static void GetMinMax(REAL* r,INTEGER Count,LONGINT& nMin,LONGINT& nMax)
{
  REAL rMin = r[0],
       rMax = rMin;
  for (INTEGER i = 1; i < Count; i++)
    if (r[i] < rMin)
      rMin = r[i];
    else if (r[i] > rMax)
      rMax = r[i];
  nMin = (LONGINT) ceil(rMin);
  nMax = (LONGINT) ceil(rMax);
}

static void Insert(LONGINT* n,INTEGER& Count,LONGINT nInsert)
{
  INTEGER i;
  for (i = Count-1; i >= 0 && n[i] > nInsert; i--)
    n[i+1] = n[i];
  n[i+1] = nInsert;
  Count++;
}

static void DrawLine(pzBufElem p,INTEGER xStart,INTEGER xEnd,
                     REAL ld1,REAL ldx,REAL md1,REAL mdx,REAL dd1,REAL ddx,
                     REAL nd,BOOLEAN bTexture,SIM3DElement* body,INTEGER facenr)
{
  REAL ld2,md2,
       dd2 = dd1 + xStart * ddx;
  if (bTexture)
  {
    ld2 = ld1 + xStart * ldx;
	 md2 = md1 + xStart * mdx;
  }
  if (xStart == xEnd)
    xEnd++;
  for (INTEGER x = xStart; x < xEnd; x++)
  {
    REAL depth = -nd/dd2;
    if (p->Depth > depth)
    {
      p->Depth = (SHORTREAL) depth;
      if (bTexture)
      {
        p->TextureX = (SHORTREAL) (ld2/dd2);
        p->TextureY = (SHORTREAL) (md2/dd2);
      }
      else
      {
        p->TextureX = 0.0F;
        p->TextureY = 0.0F;
      }
      p->Body = body;
      p->FaceNr = facenr;
    }
    if (bTexture)
    {
      ld2 += ldx;
      md2 += mdx;
    }
    dd2 += ddx;
    p++;
  }
}

void ZBUFFER::EnterThisFace (BOOLEAN texture, SIM3DElement* body,
    INTEGER facenr, const VECTOR& facefixpoint,
    const VECTOR& facehorizedge, const VECTOR& facevertedge)
// Eintragen des angegenen Polygons in den Tiefenpuffer mit den uebergebenen 
// Parametern
{
  if (PolygonStart < VertexCount) 
    NextVertex [VertexCount-1] = PolygonStart;  // Voriges Polygon schliessen

  if (VertexCount > 0) {
    LONGINT lxmin, lxmax, lymin, lymax;

    // Extrema bestimmen
    GetMinMax(VXBuffer,VertexCount,lxmin,lxmax);
    GetMinMax(VYBuffer,VertexCount,lymin,lymax);

    // Flaeche sichtbar?
    if (lxmin < (LONGINT) Width && lxmax >= 0 &&
        lymin < (LONGINT) Height && lymax >= 0) {
      INTEGER ymin = lymin < 0 ? 0 : (INTEGER) lymin,
              ymax = lymax > (LONGINT) Height ? Height : (INTEGER) lymax;

      // Parameter berechnen:
      VECTOR t00 = ViewPlaneCorner - ViewPoint;  // Richtung erster Sehstrahl
      VECTOR ba  = ViewPoint - facefixpoint;
      // (Differenz Sichtpunkt/Aufhaengepunkt der Flaeche)

      REAL ld1, ldx, ldy;
      REAL md1, mdx, mdy;
      REAL dd1, ddx, ddy;
      REAL nd;

      //Berechnung der Startwerte
      if (texture) {
  // Texturkoordinaten nur berechnen, wenn Texturoberflaeche
        ldx = det (ba, facevertedge, ViewPlaneHorizEdge);
        ldy = det (ba, facevertedge, ViewPlaneVertEdge);
        mdx = det (facehorizedge, ba, ViewPlaneHorizEdge);
        mdy = det (facehorizedge, ba, ViewPlaneVertEdge);
        ld1 = det (ba, facevertedge, t00) + ymin * ldy;
        md1 = det (facehorizedge, ba, t00) + ymin * mdy;
      }
      nd  = det (facehorizedge, facevertedge, ba);
      ddx = det (facehorizedge, facevertedge, ViewPlaneHorizEdge);
      ddy = det (facehorizedge, facevertedge, ViewPlaneVertEdge);
      dd1 = det (facehorizedge, facevertedge, t00) + ymin * ddy;
      LONGINT adr1 = (LONGINT) ymin * Width; // Startadresse im Tiefenpuffer

      static LONGINT CutPos [MaxzBufVertices];

      // Zeilen abklappern
      for (INTEGER y = ymin; y < ymax; y++) {
        INTEGER cutcount = 0;
        for (INTEGER e=0;e<VertexCount;e++) {
          // Schnittpunkte mit der Flaeche eintragen
          REAL y1 = VYBuffer [e];
          REAL y2 = VYBuffer [NextVertex [e]];
          if ((y1 <= y && y2 > y) || (y1 > y && y2 <= y)) {
            REAL x1 = VXBuffer [e];
            REAL x2 = VXBuffer [NextVertex [e]];
            Insert(CutPos,cutcount,
                   (LONGINT) ceil(x1 + (x2-x1) * (y-y1)/(y2-y1)));
          }
        }
        // Zeile in z-Buffer eintragen (nach Even-Odd-Methode)
        for (INTEGER i = 0; i < cutcount; i += 2)
        {
          LONGINT xStart = CutPos[i] < 0 ? 0 : CutPos[i],
                  xEnd = CutPos[i+1] > Width ? Width : CutPos [i+1];

          if (xStart < Width && xEnd >= 0 && xStart != xEnd)
            DrawLine(&Pixel[adr1 + xStart],
                     (INTEGER) xStart,(INTEGER) xEnd,
                     ld1,ldx,md1,mdx,dd1,ddx,nd,
                     texture,body,facenr);
        }
        dd1 += ddy;
        if (texture) {
          ld1 += ldy;
          md1 += mdy;
        }
        adr1 += (LONGINT) Width;
      }
    }
  }
  VertexCount = 0;
  PolygonStart = 0;
}

void ZBUFFER::CalcCamera (const SENSCLASSVECTOR& Background,
                          const INTEGERARRAY& classes,
                          PSHORTREAL data, REAL minDepth, REAL maxDepth)
// Die Farbwerte fuer das im Tiefenpuffer gebildete Bild in den Puffer 'data'
// gemaess den in 'classes' aufgefuehrten Indizes eintragen.
{
  pzBufElem pix = Pixel;
  PSHORTREAL p = data,
             p2;
  VECTOR d1, d2;
  INTEGER i;
  SENSCLASSVECTOR color (0,0.0F);

  d1 = ViewPlaneCorner - ViewPoint;
  for (INTEGER y=0;y<Height;y++) {
    d2 = d1;
    for (INTEGER x=0;x<Width;x++) {
      VECTOR d3 = ViewPoint + pix->Depth * d2;
      const SENSCLASSVECTOR& color = pix->Body == NOELEMENT ? Background
        : pix->Body->GetColorOfFaceAt(pix->FaceNr,d3,d2,
                                      pix->TextureX,pix->TextureY);
      // Die Farbwerte in den Puffer eintragen
      for (i = 0, p2 = p; i < classes.HowMany(); i++, p2 += Size)
      if(classes[i] == -1)
      {
        REAL depth = pix->Depth * sqrt(d2.x * d2.x + d2.y * d2.y + d2.z * d2.z);
        *p2 = (SHORTREAL) ((depth - minDepth) / (maxDepth - minDepth));
      }
      else
        *p2 = color.Index (classes [i]);
      pix++;
      p++;
      d2 = d2 + ViewPlaneHorizEdge;
    }
    d1 = d1 + ViewPlaneVertEdge;
  }
}

REAL ZBUFFER::GetMinDepth(void)
{
  pzBufElem pix = Pixel;
  VECTOR d1, d2;
  REAL mindepth = 1e6;
  d1 = ViewPlaneCorner - ViewPoint;
  for (INTEGER y=0;y<Height;y++) {
    d2 = d1;
    for (INTEGER x=0;x<Width;x++) {
      REAL depth = pix->Depth * sqrt(d2.x * d2.x + d2.y * d2.y + d2.z * d2.z);
      if(depth < mindepth)
        mindepth = depth;
      pix++;
      d2 = d2 + ViewPlaneHorizEdge;
    }
    d1 = d1 + ViewPlaneVertEdge;
  }
  return mindepth;
}
