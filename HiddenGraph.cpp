/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  HiddenGraph.cc                                        ****
 **** Inhalt: Implementation der Hiddenline-Graphikklasse           ****
 ****                                                               ****
 ***********************************************************************/

#include "HiddenGraph.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const REAL EPS = 1.0e-5;

// ************************************************
//
//   Dieses Modul ist vorlaeufig kaum kommentiert
//  (Hiddenline ist sowieso ein Kapitel fuer sich)
//
// ************************************************

enum PRELATION {PNEAR, PBEHIND, PBEFORE, PBOTH};

class BOUNDINGRECT
{
  COORD Min, Max;
  BOOLEAN Empty;
public:
  BOUNDINGRECT ();
  void    Clear ();
  void    AddPoint (COORD);
  BOOLEAN LiesIn (COORD);
  BOOLEAN DoesIntersect (BOUNDINGRECT);
};

class PLANENODE
{
public:
  PLANENODE*   Next;
  INTEGER      Count;
  COORD*       Vertices;
  BOUNDINGRECT Bounds;
  VECTOR       Normal;
  REAL         Dist;
  REAL         EyeDist;
  BOOLEAN      Ok;

  PLANENODE (INTEGER, VECTOR*, HIDDENGRAPH*);
  ~PLANENODE ();
  void      GetEdge (INTEGER, COORD&, COORD&);
  PRELATION GetRel (VECTOR);
  BOOLEAN   DoesPointLieInFace (COORD);
  void      WhereDoesLineCutPlane (VECTOR, VECTOR, BOOLEAN&, REAL&);
  BOOLEAN   EdgeCut (INTEGER, COORD, COORD, COORD, REAL&);
};

class LINENODE
{
public:
  LINENODE* Next;
  COLOR Color;
  VECTOR a, b;

  LINENODE (VECTOR, VECTOR, COLOR);
};

class VISIARR
{
  INTEGER SectionSize;
  INTEGER LocSectSize;
  REAL* Sections;
  REAL* LocSects;
  INTEGER SectionCount;
  INTEGER LocSectCount;
public:
  VECTOR p1, p2;
  COORD c1, c2, dir, normal;
  BOUNDINGRECT Bounds;

  VISIARR (INTEGER);
  ~VISIARR ();

  void         NewLine (VECTOR, VECTOR, HIDDENGRAPH*);
  BOUNDINGRECT GetBounds ();
  BOOLEAN      PartsLeft ()     { return (SectionCount > 0); };
  BOOLEAN      OddSects ()      { return ((LocSectCount % 2) == 1); };
  COORD        PointAt (REAL w) { return (c1 + dir * w); };
  void         AddSection (REAL);
  void         Merge ();
  void         Draw (HIDDENGRAPH*, COLOR);
};

//--------------------------------------------------------------------------

inline REAL min (REAL a, REAL b)
{ return (a < b ? a : b); }

inline REAL max (REAL a, REAL b)
{ return (a > b ? a : b); }

inline INTEGER min (INTEGER a, INTEGER b)
{ return (a < b ? a : b); }

inline INTEGER max (INTEGER a, INTEGER b)
{ return (a > b ? a : b); }

BOOLEAN LineCut (COORD p1, COORD p2, COORD p3, COORD p4, REAL& lambda)

// Berechnet den Schnittpunkt zweier Strecken g und h mit
// g: p1 + (p2 - p1) * lambda,   und
// h: p3 + (p4 - p3) * gamma,    0 <= gamma  <= 1.
// Resultat: TRUE wenn Schnittpunkt

{ REAL det, gamma;

  det = (p3.x - p4.x) * (p2.y - p1.y) - (p3.y - p4.y) * (p2.x - p1.x);
  if (abs (det) > EPS)    // Geraden sind parallel, wenn Determinante = 0
  {
    gamma  = (p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x);
    gamma  = gamma  / det;
    if ((gamma  >= 0.0) && (gamma  <= 1.0)) {
      lambda = (p3.y - p1.y) * (p3.x - p4.x) - (p3.x - p1.x) * (p3.y - p4.y);
      lambda = lambda / det;
      return TRUE;
    }
  }
  return FALSE;
}


//-- BOUNDINGRECT --------------------------------------------------------

BOUNDINGRECT::BOUNDINGRECT ()
{
  Empty = TRUE;
}

void BOUNDINGRECT::Clear ()
{
  Empty = TRUE;
}

void BOUNDINGRECT::AddPoint (COORD p)
{
  if (Empty) {
    Min = p;
    Max = p;
    Empty = FALSE;
  }
  else {
    Min.x = min (Min.x, p.x);
    Min.y = min (Min.y, p.y);
    Max.x = max (Max.x, p.x);
    Max.y = max (Max.y, p.y);
  }
}

BOOLEAN BOUNDINGRECT::LiesIn (COORD p)
{
  if (Empty)
    return FALSE;
  else
    return (p.x > Min.x) && (p.x < Max.x) && (p.y > Min.y) && (p.y < Max.y);
}

BOOLEAN BOUNDINGRECT::DoesIntersect (BOUNDINGRECT b)
{
  return (!Empty) && (!b.Empty) &&
         (Min.x < b.Max.x) && (Max.x > b.Min.x) &&
         (Min.y < b.Max.y) && (Max.y > b.Min.y);
}

//-- PLANENODE -----------------------------------------------------------

PLANENODE::PLANENODE (INTEGER c, VECTOR* d, HIDDENGRAPH* hg) :
  Normal ()
{
  Next      = NULL;
  Count     = c;
  Vertices  = new COORD [Count];
  VECTOR v1;
  VECTOR v2 = hg->Transform (d [Count-1]);
  VECTOR sum;
  Ok = TRUE;
  for (INTEGER i=0;i<Count;i++) {
    v1 = v2;
    if (v1.z >= hg->Eye-1) {
      Ok = FALSE;
      break;
    }
    v2 = hg->Transform (d [i]);
    Normal = Normal + VECTOR (
               (v2.y - v1.y) * (v1.z + v2.z),
               (v2.z - v1.z) * (v1.x + v2.x),
               (v2.x - v1.x) * (v1.y + v2.y));
    sum = sum + v1;
    Vertices [i] = hg->Projection (v1);
    Bounds.AddPoint (Vertices [i]);
  }
  Ok = Ok && Normal.Norm ();
  if (Ok) {
    Dist    = Normal * (sum / Count);
    EyeDist = Normal * VECTOR (0.0, 0.0, hg->Eye) - Dist;
  }
}

PLANENODE::~PLANENODE ()
{
  delete [] Vertices;
}

void PLANENODE::GetEdge (INTEGER nr, COORD& v1, COORD& v2)
{
  v1 = Vertices [nr];
  v2 = Vertices [(nr+1) % Count];
}

PRELATION PLANENODE::GetRel (VECTOR p)
{
  REAL diff = (Normal * p) - Dist;

  if (abs (diff) < EPS)
    return PNEAR;
  else if (diff * EyeDist < 0)
    return PBEHIND;
  else
    return PBEFORE;
}

BOOLEAN PLANENODE::DoesPointLieInFace (COORD p)
{
  REAL s;
  COORD c1, c2;
  INTEGER EvenOddCount = 0;

  for (INTEGER i=0;i<Count;i++) {
    GetEdge (i,c1,c2);
    if (((c1.y > p.y) && (c2.y <= p.y)) ||
        ((c1.y <= p.y) && (c2.y > p.y))) {
      s = (c1.x - c2.x) / (c1.y - c2.y) * (p.y - c2.y) + c2.x;
      if (s < p.x) EvenOddCount++;
    }
  }
  return ((EvenOddCount % 2) == 1);
}

void PLANENODE::WhereDoesLineCutPlane
  (VECTOR p1, VECTOR p2, BOOLEAN& does, REAL& where)
{
  REAL Distp1 = p1 * Normal;
  REAL Distp2 = p2 * Normal;
  REAL Distd  = Distp2 - Distp1;

  if (abs (Distd) > EPS) {
    does   = TRUE;
    where  = (Dist - Distp1) / Distd;
  }
  else
    does   = FALSE;
}

BOOLEAN PLANENODE::EdgeCut (INTEGER nr, COORD p1, COORD p2, COORD normal, REAL& where)
{
  COORD e_1 = Vertices [(nr+Count-1) % Count];
  COORD e0  = Vertices [nr];
  COORD e1  = Vertices [(nr+1) % Count];

  REAL abstand0 = (e0.x - p1.x) * normal.x + (e0.y - p1.y) * normal.y;

  if (abs (abstand0) < EPS) {   // Die Ecke beruehrt die Gerade

    REAL side0 = (e0.x - e_1.x) * normal.x + (e0.y - e_1.y) * normal.y;
    REAL side1 = (e1.x - e0.x)  * normal.x + (e1.y - e0.y)  * normal.y;

    if (((side0 > -EPS) && (side1 < EPS)) ||
        ((side0 < EPS) && (side1 > -EPS)))
      return FALSE;
    else {
      COORD dir = p2-p1;
      where = (e0-p1) * dir / (dir * dir);
      return TRUE;
    }
  }
  else {

    REAL abstand1 = (e1.x - p1.x) * normal.x + (e1.y - p1.y) * normal.y;

    if (abs (abstand1) >= EPS) {
      // Wir berechnen den ganz normalen Schnittpunkt mit der Kante.
      // (Die Ecken der Kante werden jedoch ausgespart...)

      REAL det = (e0.x - e1.x) * (p2.y - p1.y) - (e0.y - e1.y) * (p2.x - p1.x);

      if (abs (det) > EPS)    // Geraden sind parallel, wenn Determinante = 0
      {
        REAL gamma = ((e0.x - p1.x) * (p2.y - p1.y)
                    - (e0.y - p1.y) * (p2.x - p1.x)) / det;
        if ((gamma >= EPS) && (gamma  <= 1.0 - EPS)) {
          REAL lambda = ((e0.y - p1.y) * (e0.x - e1.x)
                       - (e0.x - p1.x) * (e0.y - e1.y)) / det;
          where = lambda;
          return TRUE;
        }
      }
    }
    return FALSE;
  }
}


//-- LINENODE ------------------------------------------------------------

LINENODE::LINENODE (VECTOR p1, VECTOR p2, COLOR c)
{
  Next = NULL;
  Color = c;
  a = p1;
  b = p2;
}

//-- VISIARR -------------------------------------------------------------

VISIARR::VISIARR (INTEGER count)
{
  SectionSize = count*4;
  LocSectSize = count;
  Sections = new REAL [SectionSize];
  LocSects = new REAL [LocSectSize];
  SectionCount = 0;
  LocSectCount = 0;
}

VISIARR::~VISIARR ()
{
  delete [] Sections;
  delete [] LocSects;
}

void VISIARR::NewLine (VECTOR pp1, VECTOR pp2, HIDDENGRAPH* hg)
{
  p1 = pp1;
  p2 = pp2;
  c1 = hg->Projection (p1);
  c2 = hg->Projection (p2);
  dir = (c2 - c1);
  REAL length = sqrt (dir * dir);
  if(length)
  {
    normal = COORD (-dir.y, dir.x) / length;
    Sections [0] = 0.0;
    Sections [1] = 1.0;
    SectionCount = 2;
    LocSectCount = 0;
    Bounds.Clear ();
    Bounds.AddPoint (c1);
    Bounds.AddPoint (c2);
  }
}

void VISIARR::AddSection (REAL w)
{
  if (LocSectCount < LocSectSize) {
    LocSects [LocSectCount] = w;
    LocSectCount++;
  }
}

void VISIARR::Merge ()
{
  if ((LocSectCount > 0) && (SectionCount > 0)) {

    // Erst die lokalen Schnittpunkte sortieren...

    for (INTEGER i=1;i<LocSectCount;i++) {
      INTEGER j=i;
      while ((j>0) && (LocSects [j] < LocSects [j-1])) {
        REAL h = LocSects [j];
        LocSects [j] = LocSects [j-1];
        LocSects [j-1] = h;
        j--;
      }
    }

    if (LocSectCount % 2 == 0)
    {
      // ...und dann in das globale Sichtbarkeitsfeld einsortieren

      REAL OldMin = Sections [0];
      REAL OldMax = Sections [SectionCount-1];

      REAL* Dest = new REAL [SectionSize];
      INTEGER gs = 0;
      INTEGER ls = 0;
      INTEGER ds = 0;

      while ((gs < SectionCount) && (ds < SectionSize)) {

        if (ls == LocSectCount) {
          Dest [ds++] = Sections [gs];
          gs++;
        }
        else {
          REAL g = Sections [gs];
          REAL l = LocSects [ls];
          if (abs (g-l) < EPS) {
            if ((gs % 2) != (ls % 2)) Dest [ds++] = g;
            gs++;
            ls++;
          }
          else if (g < l) {
            if ((ls % 2) == 0)  Dest [ds++] = g;
            gs++;
          }
          else {
            if ((gs % 2) == 1)  Dest [ds++] = l;
            ls++;
          }
        }
      }
      delete [] Sections;
      Sections = Dest;
      SectionCount = ds;

      if (SectionCount == 0)
        Bounds.Clear ();
      else {
        if ((OldMin != Sections [0]) ||
            (OldMax != Sections [SectionCount-1])) {
          Bounds.Clear ();
          Bounds.AddPoint (PointAt (Sections [0]));
          Bounds.AddPoint (PointAt (Sections [SectionCount-1]));
        }
      }
    }
  }
  LocSectCount = 0;
}

void VISIARR::Draw (HIDDENGRAPH* hg, COLOR color)
{
  COORD c1, c2;
  INTEGER i=0;
  while (i<SectionCount) {
    c1 = PointAt (Sections [i++]);
    c2 = PointAt (Sections [i++]);
    hg->SetLine (c1, c2, color);
  }
}

//-- HIDDENGRAPH ---------------------------------------------------------

HIDDENGRAPH::HIDDENGRAPH ()
{
  FirstLine = NULL;
  FirstPlane = NULL;
  Hidden = FALSE;
  HiddenInMind = FALSE;
}

HIDDENGRAPH::~HIDDENGRAPH ()
{
  while (FirstLine != NULL)
  {
    LINENODE* help = FirstLine;
    FirstLine = FirstLine->Next;
    delete help;
  }
  while (FirstPlane != NULL) {
    PLANENODE* help = FirstPlane;
    FirstPlane = FirstPlane->Next;
    delete help;
  }
}

VECTOR HIDDENGRAPH::Transform (const VECTOR& v)
{
  return MM * v;
}

COORD HIDDENGRAPH::Projection (const VECTOR& v)
{
  REAL f = (Eye - Plane) / (Eye - v.z);
  return COORD (v.x * f, v.y * f);
}

REAL HIDDENGRAPH::ProjectSection (const VECTOR& p1, const VECTOR& p2, REAL d)

// Wenn ein Punkt auf der Geraden durch p1_p2 an der Stelle d liegt
// (d=0 -> p1, d=1 -> p2); welche Stelle entspricht diesem Punkt
// nach der zentralperspektivischen Projektion?
// Nach Umformung ergibt sich folgende einfache Gleichung, in die
// nur noch die Tiefe der beiden Endpunkte eingeht:
{
  REAL zd = p1.z + (p2.z-p1.z) * d;
  return d * (Eye - p2.z) / (Eye - zd);
}

void HIDDENGRAPH::SetViewPoint (const MOVEMATRIX& M, REAL p, REAL e)
{
  MM = M;
  Plane = p;
  Eye = e;
}

void HIDDENGRAPH::SetHidden (BOOLEAN b)
{
  HiddenInMind = b;
}

void HIDDENGRAPH::NewPicture ()
{
  MaxVertices = 2;
  Hidden = HiddenInMind;
}

void HIDDENGRAPH::DrawLine (const VECTOR& p1, const VECTOR& p2, COLOR color)
{
  if (abs (p1-p2) > EPSILON) {
    VECTOR pt1 = Transform (p1),
           pt2 = Transform (p2);
    if ((pt1.z < Eye-1) && (pt2.z < Eye-1)) {
      if (Hidden) {
        LINENODE* line = new LINENODE (pt1, pt2, color);
        line->Next = FirstLine;
        FirstLine = line;
      }
      else {
        COORD c1 = Projection (pt1);
        COORD c2 = Projection (pt2);
        SetLine (c1, c2, color);
      }
    }
  }
}

void HIDDENGRAPH::DrawEdge (const VECTOR& p1, const VECTOR& p2, COLOR color)
{
  DrawLine (p1, p2, color);
}

void HIDDENGRAPH::DrawPolygon (INTEGER count, VECTOR* varr, COLOR)
{
  if (Hidden) {
    PLANENODE* plane = new PLANENODE (count, varr, this);
    if (plane->Ok) {
      plane->Next = FirstPlane;
      FirstPlane = plane;
      MaxVertices = max (MaxVertices, count);
    }
    else
      delete plane;
  }
}

void HIDDENGRAPH::EndOfPicture ()
{
  VECTOR p3, p4;

  if (Hidden) {
    VISIARR visiarr (MaxVertices);

    while (FirstLine != NULL) {
      visiarr.NewLine (FirstLine->a,FirstLine->b, this);

      PLANENODE* LoopPlane = FirstPlane;
      while (visiarr.PartsLeft () && (LoopPlane != NULL)) {
        PRELATION r1 = LoopPlane->GetRel (visiarr.p1);
        PRELATION r2 = LoopPlane->GetRel (visiarr.p2);
        if (((r1 == PBEHIND) || (r2 == PBEHIND)) &&
            visiarr.Bounds.DoesIntersect (LoopPlane->Bounds)) {
          REAL div, where;
          BOOLEAN does;
          INTEGER sel = 0;

          if ((r1 == PBEHIND) && (r2 == PBEFORE))       sel = 1;
          else if ((r1 == PBEFORE) && (r2 == PBEHIND))  sel = 2;

          if (sel != 0) {
            LoopPlane->WhereDoesLineCutPlane
              (visiarr.p1, visiarr.p2, does, div);
            if ((! does) || (div <= 0) || (div >= 1.0))
              SetError (GRAPHIC_COMP_ERROR);
            else
              div = ProjectSection (visiarr.p1, visiarr.p2, div);
          }

          for (INTEGER i=0;i<LoopPlane->Count;i++) {
            if (LoopPlane->EdgeCut (i, visiarr.c1, visiarr.c2,
                                       visiarr.normal, where))
              if ((sel == 0) ||
                 ((sel == 1) && (where < div)) ||
                 ((sel == 2) && (where > div)))
                visiarr.AddSection (where);
          }
          if (sel != 0 && visiarr.OddSects ())
            visiarr.AddSection (div);
          visiarr.Merge ();
        }
        LoopPlane = LoopPlane->Next;
      }
      visiarr.Draw (this, FirstLine->Color);
      LINENODE* help = FirstLine;
      FirstLine = FirstLine->Next;
      delete help;
    }
  }
  while (FirstPlane != NULL) {
    PLANENODE* help = FirstPlane;
    FirstPlane = FirstPlane->Next;
    delete help;
  }
}

