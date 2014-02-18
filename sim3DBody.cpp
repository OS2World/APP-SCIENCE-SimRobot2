/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DBody.cc                                          ****
 **** Inhalt: Die Klassen SIM3DBody (feste Koerper) und SIM3DPolyeder **
 ****         (polygonale Koerper) werden hier implementiert.       ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DBody.h"
#include "sim3DWorld.h"
#include "sim3DErrors.h"
#include <stdlib.h>
#include <math.h>

inline REAL min (REAL a, REAL b)
  { return (a < b ? a : b); }

inline REAL max (REAL a, REAL b)
  { return (a > b ? a : b); }

inline REAL MakeRad (REAL x) 
  { return x / 180 * PI; }

SURFACEDESCR* ReadSurface (SCANNER& scan, SIM3DWorld* World)
{
  STRING s;

  if (GetCorrectString (scan, s)) {
    SURFACEDESCR* surface = World->FindSurface (s);
    if (surface == NULL)
      scan.SyntaxError (sim3DUnknownMaterial1,s);
    return surface;
  }
  else
    return NULL;
}

// --- SIM3DBody --------------------------------------------------------

SIM3DBody::SIM3DBody (const STRING& name) : SIM3DElement (name)
{
  Material = NULL;
}

SIM3DBody::SIM3DBody (const SIM3DBody& Obj) : SIM3DElement (Obj)
{
  Material = Obj.Material;
}

void SIM3DBody::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "SURFACE")
    Material = ReadSurface (scan, World);
  else
    SIM3DElement::ReadAttr (attr, scan);
}

void SIM3DBody::WriteAttrs (BACKUP& backup)
{
  SIM3DElement::WriteAttrs (backup);
  if (Material != NULL)
    backup.WriteIdStrNext ("SURFACE", Material->Name);
}

COLOR SIM3DBody::GetColor ()
{
  if (Color < 0)
    return World->BodyColor;
  else
    return Color;
}

SURFACEDESCR* SIM3DBody::GetSurface (INTEGER)
{
  if (Material != NULL)
    return Material;
  else 
    return &(World->StandardSurface);
}

//- FACESTRUCT -------------------------------------------------------------
//
// Ein FACESTRUCT enthaelt Indices auf die Kanten einer Flaeche in einem
// Polyeder, den Normalenvektor, Orts- und Richtungsvektoren, einen
// Zeiger auf einen Oberflaechendescriptor, die Groesse der Flaeche, usw.

class FACESTRUCT 
{
public:
  INTEGER FirstEdge,
          NextEdge,
          NormalDir,
          HorizDir,
          VertDir,
          FixPoint;
  BOOLEAN IsPrecalculated;
  SENSCLASSVECTOR Color;
  SURFACEDESCR* Material;
  REAL SizeX,
       SizeY;
  REAL Angle;

  FACESTRUCT ();
  ~FACESTRUCT () {}
  INTEGER EdgeCount ();
};

FACESTRUCT::FACESTRUCT ()
: Color(0,0.0F)
{
  FirstEdge = 0;
  NextEdge  = 0;
  NormalDir = 0;
  HorizDir  = 0;
  VertDir   = 0;
  FixPoint  = 0;
  Material  = NULL;
  SizeX     = 0.0;
  SizeY     = 0.0;
  Angle     = 0;
  IsPrecalculated = FALSE;
}

INTEGER FACESTRUCT::EdgeCount ()
{
  return NextEdge - FirstEdge;
}

//- FACEARRAY -------------------------------------------------------------

FACEARRAY::FACEARRAY () : DYNARRAY (2)
{
  Data = NULL;
}

FACEARRAY::FACEARRAY (const FACEARRAY& v) : DYNARRAY (v.Delta)
{
  Count = v.Count;
  Delta = v.Delta;
  Size  = v.Count;
  if(Size)
    Data  = new FACESTRUCT [Size];
  INTEGER i;
  for (i=0;i<Count;i++) {Data [i] = v.Data [i];}
}

FACEARRAY::~FACEARRAY ()
{
  if (Size != 0) delete [] Data;
}

void FACEARRAY::Resize (INTEGER newsize)
{
  FACESTRUCT* newdata = 0;
  if (newsize > 0)
  {
    newdata = new FACESTRUCT [newsize];
    INTEGER copysize = ((newsize > Size) ? Size : newsize);
    INTEGER i;
    for (i=0;i<copysize;i++) {newdata [i] = Data [i];}
  }
  if (Size > 0) delete [] Data;
  Data = newdata;
  Size = newsize;
}

void FACEARRAY::AddElement (const FACESTRUCT& f)
{
  NewElementCount (Count+1);
  Data [Count] = f;
  Count++;
}

FACESTRUCT& FACEARRAY::operator [] (INTEGER i)
{
  if (i < 0 || i >= Count)
    sim3DFatalError (sim3DOutOfRange);
  return Data [i];
}


// --- SIM3DPolyeder ------------------------------------------------
//
// Siehe auch "sim3DBody.h" und "sim3DBasics.h"

SIM3DPolyeder::SIM3DPolyeder (const STRING& name)
: SIM3DBody (name), scvTemp(0,0.0F)
{
  Closed     = FALSE;
  PointCount = 0;
  IsTransparent = FALSE;
}

SIM3DPolyeder::SIM3DPolyeder (const SIM3DPolyeder& Obj) : SIM3DBody (Obj),
  BasicPoints (Obj.BasicPoints),
  NewPoints   (Obj.NewPoints),
  OldPoints   (Obj.OldPoints),
  BasicFaceDir(Obj.BasicFaceDir),
  NewFaceDir  (Obj.NewFaceDir),
  OldFaceDir  (Obj.OldFaceDir),
  Edges       (Obj.Edges),
  Vertices    (Obj.Vertices),
  Faces       (Obj.Faces),
  scvTemp(0,0.0F)
{
  Closed     = Obj.Closed;
  PointCount = Obj.PointCount;
  IsTransparent = Obj.IsTransparent;
}

BOOLEAN ReadCorrectSurface (SCANNER& scan, SIM3DWorld* World, 
                        SURFACEDESCR*& material, REAL& angle)
{
  angle = 0;
  material = ReadSurface (scan,World);
  if (scan.Symbol == sComma) {
    scan.NextSymbol ();
    REAL r;
    if (GetCorrectReal (scan, r)) 
      angle = r;
    else
      return FALSE;
  }
  return (material != NULL);
}

void SIM3DPolyeder::ReadAttr (const STRING& attr, SCANNER& scan)
{
  VECTOR v;
  INTEGER i;

  if (attr == "TRANSPARENT") {
    IsTransparent = TRUE;
  }
  else if (attr == "SURFACE") {
    Material = ReadSurface (scan, World);
    if (Material != NULL && Material->HasTexture ())
      for (INTEGER f=0;f<Faces.HowMany();f++)
        if (Faces [f].Material == NULL)
          SetFaceOrientation (f, Faces [f].Angle, 90);
  }
  else if (attr == "BASEPOINTS") {
    if (scan.TestFor (sLParen, sSemicolon)) {
      do {
        scan.NextSymbol ();
        if (GetCorrectVector (scan, v)) {
          BasicPoints.AddElement (v);
          PointCount++;
        }
      } while (scan.Symbol == sComma);
      (void) scan.Expect (sRParen, sSemicolon);
    }
  }
  else if (attr == "SWEEP") {
    FILEPOS pos = scan.GetLastPos ();
    if (GetCorrectVector (scan, v))
      Sweep (v,pos);
  }
  else if (attr == "ROTATE") {
    FILEPOS pos = scan.GetLastPos ();
    if (GetCorrectInteger (scan, i))
      Rotate (i, FALSE,pos);
  }
  else if (attr == "FACES") {
    FILEPOS pos = scan.GetLastPos ();
    if (scan.TestFor (sLParen, sSemicolon)) {
      do {
        scan.NextSymbol ();
        if (scan.TestFor (sLParen, sSemicolon)) {
          BOOLEAN Ok = TRUE;
          do {
            scan.NextSymbol ();
            FILEPOS pos2 = scan.GetLastPos ();
            if (GetCorrectInteger (scan, i))
              if(i >= 0 && i < PointCount) {
                if (!Closed)
                  AddFaceEdge (i);
                else if (Ok) {
                  sim3DOtherError(sim3DAlreadyDefined,pos);
                  Ok = FALSE;
                }
              }
              else {
                sim3DOtherError(sim3DPointUndef,pos2);
                Ok = FALSE;
              }
          } while (scan.Symbol == sComma);
          if (Ok)
          {
            NewFace (pos);
            if (scan.Symbol == sSemicolon) {
              scan.NextSymbol ();
              SURFACEDESCR* material = NULL;
              REAL angle = 0;
              if (scan.Symbol == sString && 
                  ReadCorrectSurface (scan, World, material, angle)) {
                INTEGER f = Faces.HowMany()-1;
                Faces [f].Material = material;
                Faces [f].Angle    = angle;
                if (material->HasTexture()) 
                  SetFaceOrientation (f, angle, 90);
              }  
            }
          }
          scan.Expect (sRParen, sSemicolon);
        }
      } while (scan.Symbol == sComma);
      LastFace (pos);
      scan.Expect (sRParen, sSemicolon);
    }
  }
  else
    SIM3DBody::ReadAttr (attr, scan);
}

void SIM3DPolyeder::WriteAttrs (BACKUP& backup)
{
  SIM3DBody::WriteAttrs (backup);
  INTEGER i, f;

  if (IsTransparent) {
    backup.WriteIdent ("TRANSPARENT");
    backup.ListNext ();
  }

  backup.WriteIdent ("BASEPOINTS");
  backup.StartListI (sLParen,sComma,sRParen, TRUE);
  for (i=0;i<PointCount;i++) {
    backup.WriteVector (BasicPoints [i]);
    backup.ListNext ();
  }
  backup.EndList ();
  backup.ListNext ();

  backup.WriteIdent ("FACES");
  backup.StartListI (sLParen,sComma,sRParen, TRUE);
  for (f=0;f<Faces.HowMany();f++) {
    backup.StartList (sLParen,sComma,sRParen, FALSE);
    for (i=Faces[f].FirstEdge;i<Faces[f].NextEdge-1;i++) {
      backup.WriteInteger (abs (Vertices [i])-1);
      backup.ListNext ();
    }
    backup.WriteInteger (abs (Vertices [i])-1);
    if (Faces [f].Material != NULL) {
      backup.WriteSymbol (sSemicolon);
      backup.WriteString (Faces [f].Material->Name);
      if (Faces [f].Angle != 0.0) {
        backup.WriteSymbol (sComma);
        backup.WriteReal (Faces [f].Angle);
      }
    }
    backup.EndList ();
    backup.ListNext ();
  }
  backup.EndList ();
  backup.ListNext ();
}

void SIM3DPolyeder::NewPosition (const MOVEMATRIX& M)
// Angeben einer neuen Transformation und Translation, die auf den Grundkoerper
// angewendet werden soll.
// Retten der alten Werte.
{
  INTEGER i, count;

  count = NewPoints.HowMany ();
  for (i=0;i<count;i++) {
    OldPoints [i] = NewPoints [i];
    NewPoints [i] = M * BasicPoints [i];
  }

  count = NewFaceDir.HowMany ();
  for (i=0;i<count;i++) {
    OldFaceDir [i] = NewFaceDir [i];
    NewFaceDir [i] = M.Matrix * BasicFaceDir [i];
    NewFaceDir [i].Norm ();
  }
}

void SIM3DPolyeder::RegisterAdd ()
// Beim Einfuegen Position des Polyeders an die Lage in der Szene anpassen
{
  SIM3DElement::RegisterAdd ();
  NewPosition (GetPosRelTo (NOELEMENT));
  MakeNewBoundingBox ();
}

void SIM3DPolyeder::MakeStep (const MOVEMATRIX& M)
// Bewegung durchfuehren
{
  NewPosition (M * GetLocalTransform ());
}

void SIM3DPolyeder::StepBack ()
// Bewegung rueckgaengig machen
{
  INTEGER i;
  INTEGER count = NewPoints.HowMany ();

  for (i=0;i<count;i++)  NewPoints [i] = OldPoints [i];

  count = NewFaceDir.HowMany ();

  for (i=0;i<count;i++)  NewFaceDir [i] = OldFaceDir [i];

}

void SIM3DPolyeder::MakeNewBoundingBox ()
// Erstellen eines neuen umgebenden Rechtecks
{
  INTEGER i;
  BoundingBox.Clear ();
  INTEGER pointCount = NewPoints.HowMany();
  for (i=0;i<pointCount;i++)  BoundingBox.AddPoint (NewPoints [i]);
}

void SIM3DPolyeder::AddPoint (const VECTOR& p)
// Anfuegen eines neuen Eckpunktes
{
  if (Closed)
    sim3DOtherError (sim3DAlreadyDefined);
  else
    BasicPoints.AddElement (p);
}

VECTOR SIM3DPolyeder::GetFaceVertex (INTEGER facenr, INTEGER vertex)
{
  if (facenr < 0 || facenr >= Faces.HowMany() || vertex < 0 ||
      vertex >= Faces [facenr].NextEdge) {
    sim3DOtherError (sim3DOutOfRange);
    return VECTOR (0.0, 0.0, 0.0);
  }
  else
    return BasicPoints [abs (Vertices [Faces [facenr].FirstEdge + vertex])-1];
}

void SIM3DPolyeder::AddFaceEdge (INTEGER e)
// Anfuegen einer Ecke einer Flaeche (Index in Point-Array)
{
  if (Closed)
    sim3DOtherError (sim3DAlreadyDefined);
  else if (e < 0)
    sim3DOtherError (sim3DOutOfRange);
  else
    Vertices.AddElement (e+1);
}

void SIM3DPolyeder::AddEdgeOnce (INTEGER index1, INTEGER index2)
// Eintragen einer Kante (zwei Indizes in Point-Array). Doppelte werden nicht
// eingetragen
{
  INTEGER EdgeCount = Edges.HowMany ();
  INTEGER edgei=0;
  while ((edgei < EdgeCount) &&
         ((Edges [edgei] != index1) || (Edges [edgei+1] != index2)))
    edgei = edgei + 2;
  if (edgei == EdgeCount) {
    Edges.AddElement (index1);
    Edges.AddElement (index2);
  }
}

void SIM3DPolyeder::NewFace (const FILEPOS& pos)
// Starten einer neuen Flaeche. Die letzte Ecke, die mit AddFaceEdge fuer die
// Flaeche ingetragen wurde, wird mit ihrem negativen Absolutwert als Endpunkt
// eines geschlossenen Kantenzuges markiert. Der Kantenzug begrenzt die Flaeche.
// Die einzelnen Kanten werden mit AddEdgeOnce eingetragen.
// Ausserdem wird der Normalenvektor der Flaeche berechnet. Wenn dieser Null
// ist, wird die Flaeche nicht eingetragen.
{
  FACESTRUCT face;

  if (Closed)
    sim3DOtherError (sim3DAlreadyDefined,pos);
  else if (Vertices.HowMany () > 0) {
    INTEGER FaceCount = Faces.HowMany ();
    face.FirstEdge = (FaceCount == 0 ? 0 : Faces [FaceCount-1].NextEdge);
    face.NextEdge  = Vertices.HowMany ();
    Vertices [face.NextEdge-1] = -Vertices [face.NextEdge-1];

    if (face.EdgeCount() < 3) {
      if (face.FirstEdge != face.NextEdge-1)
        AddEdgeOnce (abs (Vertices [face.FirstEdge]), 
                     abs (Vertices [face.NextEdge-1]));
      sim3DOtherError (sim3DTooFewEdges,pos);
    }
    else {
      INTEGER startindex, index1, index2, i;
      VECTOR normal (0,0,0);
      VECTOR mid (0,0,0);
      VECTOR v1, v2;

      startindex = Vertices [face.FirstEdge];
      for (i=face.FirstEdge;i<face.NextEdge;i++)
      {
        index1 = Vertices [i];
        if (index1 < 0) {
          index1 = -index1;
          index2 = startindex;
          if (i+1 < face.NextEdge)
            startindex = abs (Vertices [i+1]);
        }
        else
          index2 = abs (Vertices [i+1]);

        AddEdgeOnce (index1, index2);           // Eintragen der Kanten

        v1 = BasicPoints [index1-1];
        v2 = BasicPoints [index2-1];

        normal = normal + VECTOR (              // Bestimmung eines Normalen-
          (v2.y - v1.y) * (v1.z + v2.z),        // vektors
          (v2.z - v1.z) * (v1.x + v2.x),
          (v2.x - v1.x) * (v1.y + v2.y));
        mid = mid + v1;
      }
      mid = mid / face.EdgeCount();

      // Bestimme die erste Kante der Flaeche:
      v1 = BasicPoints [abs (Vertices [face.FirstEdge+1])-1]
         - BasicPoints [abs (Vertices [face.FirstEdge])-1];
      // Benutze den Richtungsvektor, um zwei senkrecht aufeinander stehende
      // Richtungsvektoren der Flaeche zu konstruieren.
      v2 = CrossProduct (normal, v1);
      v1 = -CrossProduct (normal, v2);

      // Wenn sich die Richtungsvektoren normieren lassen (und damit also
      // vom Null-Vektor verschieden sind), Flaeche abspeichern
      if (normal.Norm() && v1.Norm() && v2.Norm()) {
        face.NormalDir = BasicFaceDir.HowMany ();
        face.HorizDir  = face.NormalDir+1;
        face.VertDir   = face.NormalDir+2;
        face.FixPoint  = BasicPoints.HowMany();
        BasicFaceDir.AddElement (normal);
        BasicFaceDir.AddElement (v1);
        BasicFaceDir.AddElement (v2);
        BasicPoints.AddElement (mid);
        Faces.AddElement (face);
        RecalcFaceGeometry (FaceCount);
      }
      else                                      // Wenn Flaecheninhalt = 0
        Vertices.Remove (face.EdgeCount());
    }
  }
}

void SIM3DPolyeder::LastFace (const FILEPOS& pos)
// Polyeder-Definition abschliessen, Aufhaengepunkte der Flaechen berechnen,
// restliche Arrays initialisieren
{
  if (Closed) {
    sim3DOtherError (sim3DAlreadyDefined,pos);
    return;
  }

  Closed     = TRUE;
  NewPoints  = BasicPoints;
  OldPoints  = NewPoints;
  NewFaceDir = BasicFaceDir;
  OldFaceDir = NewFaceDir;
}

void SIM3DPolyeder::SetFaceOrientation (INTEGER facenr, 
                                        REAL winkela, REAL winkelb)
{
  // Standardvektoren neu bestimmen:
  VECTOR nv = BasicFaceDir [Faces [facenr].NormalDir];
  // Bestimme die erste Kante der Flaeche:
  VECTOR hdv = GetFaceVertex (facenr, 1) - GetFaceVertex (facenr, 0);
  // Benutze den Richtungsvektor, um zwei senkrecht aufeinander stehende
  // Richtungsvektoren der Flaeche zu konstruieren.
  VECTOR vdv = CrossProduct (nv, hdv);
         hdv = -CrossProduct (nv, vdv);
  (void) hdv.Norm();
  (void) vdv.Norm();
  
  winkelb = winkelb + winkela;
  winkela = MakeRad (winkela);
  winkelb = MakeRad (winkelb);

  BasicFaceDir [Faces [facenr].HorizDir] =
    hdv * cos (winkela) + vdv * sin (winkela);
  BasicFaceDir [Faces [facenr].VertDir] = 
    hdv * cos (winkelb) + vdv * sin (winkelb);

  RecalcFaceGeometry (facenr);
}  

void SIM3DPolyeder::RecalcFaceGeometry (INTEGER facenr)
// Bestimmung der Flaechenausdehnung und des Mittelpunktes
{
    FACESTRUCT* face = &Faces [facenr];
    REAL        hmin, hmax, vmin, vmax;
    VECTOR      v1   = BasicPoints [face->FixPoint];
    VECTOR      hdv  = BasicFaceDir [face->HorizDir];
    VECTOR      vdv  = BasicFaceDir [face->VertDir];
    REAL        hd   = hdv * v1;
    REAL        vd   = vdv * v1;
    BOOLEAN    Start = TRUE;
    for (INTEGER i=0;i<face->EdgeCount();i++) {
      VECTOR    v    = GetFaceVertex (facenr, i);
      REAL      hd2  = hdv * v - hd;
      REAL      vd2  = vdv * v - vd;
      if (Start || hd2 < hmin) hmin = hd2;
      if (Start || hd2 > hmax) hmax = hd2;
      if (Start || vd2 < vmin) vmin = vd2;
      if (Start || vd2 > vmax) vmax = vd2;
      Start = FALSE;
    }

    face->SizeX = hmax-hmin;
    face->SizeY = vmax-vmin;
    BasicPoints [face->FixPoint] = v1 + hdv * (hmin+hmax)/2 
                                      + vdv * (vmin+vmax)/2;

    if (face->Material != NULL || Material != NULL || Registered) {
      SURFACEDESCR* material = GetSurface (facenr);
      if (material->HasTexture()) {
        REAL ratiox, ratioy;
        BOOLEAN autosize;
        material->GetSizeOrRatio (ratiox, ratioy, autosize);
        REAL r;
        if (ratiox != 0 && ratioy != 0 && autosize) 
          r = max (face->SizeX/ratiox, face->SizeY/ratioy);  
        else
          r = 1;
        if (ratiox != 0) 
          face->SizeX = r * ratiox;
        if (ratioy != 0)
          face->SizeY = r * ratioy;
      }
    }
}

void SIM3DPolyeder::Sweep (const VECTOR& delta,const FILEPOS& pos)
// Sweep-Koerper erstellen. Die in BasicPoints eingetragenen Punkte werden als
// die Ecken *eines* geschlossenen Kantenzuges begriffen. Dieser wird um 'delta'
// verschoben und das entstehende Prisma gebildet. Die Punkte sollten deshalb
// moeglichst in einer Ebene liegen.
{
  INTEGER count = BasicPoints.HowMany();
  INTEGER i;

  if (Edges.HowMany() != 0)
    sim3DOtherError (sim3DAlreadyDefined,pos);
  else if (count < 2)
    sim3DOtherError (sim3DTooFewEdges,pos);
  else {
    for (i=0;i<count;i++)
      BasicPoints.AddElement (BasicPoints [i] + delta);
    if (count > 2) {
      for (i=0;i<count;i++) AddFaceEdge (i);
      NewFace (pos);
      for (i=0;i<count;i++) AddFaceEdge (i+count);
      NewFace (pos);
      for (i=0;i<count-1;i++) {
        AddFaceEdge (i);
        AddFaceEdge (i+1);
        AddFaceEdge (i+1 + count);
        AddFaceEdge (i + count);
        NewFace (pos);
      }
    }
    AddFaceEdge (count-1);
    AddFaceEdge (0);
    AddFaceEdge (count);
    AddFaceEdge (count-1 + count);
    NewFace (pos);
    LastFace (pos);
  }
}

BOOLEAN IsAtZAxis (VECTOR v)
// Test: Liegt der angegebene Punkt auf der z-Achse
{
  return ((v.x + v.y) < EPSILON);
}

void SIM3DPolyeder::Rotate (INTEGER turns, BOOLEAN torus,const FILEPOS& pos)
// Die Punkte in 'BasicPoints' werden als Kantenzug begriffen, der 'turns' Mal
// um die z-Achse gedreht wird. 'torus' gibt an, ob die Flaechen erzeugt werden,
// die den Koerper oben und unten schliessen.
{
  INTEGER count = BasicPoints.HowMany();
  INTEGER i, j;

  if (Edges.HowMany() != 0)
    sim3DOtherError (sim3DAlreadyDefined,pos);
  else if ((turns < 3) || (count < 1))
    sim3DOtherError (sim3DTooFewEdges,pos);
  else {
    for (i=1;i<turns;i++) {
      MATRIX M = TurnXYMatrix ((360.0 / turns) * i);
      for (j=0;j<count;j++) {
        AddPoint (M * BasicPoints [j]);
      }
    }
    if (count > 1) {
      for (j=0;j<count-1;j++) {
        for (i=0;i<turns;i++) {
          AddFaceEdge (j + i*count);
          AddFaceEdge (j + ((i+1) % turns)*count);
          AddFaceEdge (j+1 + ((i+1) % turns)*count);
          AddFaceEdge (j+1 + i*count);
          NewFace (pos);
        }
      }
    }
    if (!torus) {
      for (i=0;i<turns;i++) AddFaceEdge (i*count);
      NewFace (pos);
      if (count > 1) {
        for (i=0;i<turns;i++) AddFaceEdge (i*count + count-1);
        NewFace (pos);
      }
    }
    else if (count > 2) {
      for (i=0;i<turns;i++) {
        AddFaceEdge (i*count);
        AddFaceEdge (((i+1) % turns)*count);
        AddFaceEdge (count-1 + ((i+1) % turns)*count);
        AddFaceEdge (count-1 + i*count);
        NewFace (pos);
      }
    }
    LastFace (pos);
  }
}

void SIM3DPolyeder::CheckFaceNr (INTEGER facenr)
{
  if (facenr < 0 || facenr >= Faces.HowMany ()) 
    sim3DFatalError (sim3DOutOfRange);
}

SURFACEDESCR* SIM3DPolyeder::GetSurface (INTEGER n)
{
  CheckFaceNr (n);
  if (Faces [n].Material == NULL)
    return SIM3DBody::GetSurface (n);
  else
    return Faces [n].Material;
}

VECTOR SIM3DPolyeder::NormalVectorOfFace (INTEGER nr)
// Berechne den Normalenvektor der angegebenen Flaeche.
{
  INTEGER first, last, actual;
  VECTOR sum (0.0, 0.0, 0.0);
  VECTOR v1, v2;

  if (nr >= Faces.HowMany ()) sim3DFatalError (sim3DOutOfRange);

  first = Faces [nr].FirstEdge;
  last  = Faces [nr].NextEdge-1;

  for (actual=first;actual<=last;actual++) {
    GetEdge (actual,v1,v2);
    sum = sum + VECTOR (
      (v2.y - v1.y) * (v1.z + v2.z),
      (v2.z - v1.z) * (v1.x + v2.x),
      (v2.x - v1.x) * (v1.y + v2.y));
  }
  return sum;
}

void SIM3DPolyeder::GetEdge (INTEGER nr, VECTOR& v1, VECTOR& v2)
// Hole die Kante einer Flaeche in Koordinaten. Wenn der erste Index, der aus
// 'Vertices' entnommen wurde, negativ ist, so ist dies die letzte Ecke eines
// Kantenzuges und die erste Ecke des Kantenzuges muss bestimmt werden.
{
  INTEGER index1, index2;

  index1 = Vertices [nr];
  if (index1 < 0) {
    index1 = -index1;
    while ((nr > 0) && (Vertices [nr-1] > 0)) nr--;
    index2 = Vertices [nr];
  }
  else
    index2 = abs (Vertices [nr+1]);
  v1 = NewPoints [index1-1];
  v2 = NewPoints [index2-1];
}

void SIM3DPolyeder::GetEdgeOnce (INTEGER nr, VECTOR& v1, VECTOR& v2)
// Hole eine Kante des Polyeders in Koordinaten
{
  nr = nr*2;
  v1 = NewPoints [Edges [nr]-1];
  v2 = NewPoints [Edges [nr+1]-1];
}

SENSCLASSVECTOR& SIM3DPolyeder::GetColorOfFaceAt (INTEGER n, const VECTOR& p)
{
  REAL x = 0,
       y = 0;
  if(GetSurface (n)->HasTexture())
    GetPlaneCoord (n, p, x, y);
  return GetColorOfFaceAt(n,x,y);
}
  
SENSCLASSVECTOR& SIM3DPolyeder::GetColorOfFaceAt (INTEGER n,
                                                  REAL TextureX, REAL TextureY)
{
  return GetSurface (n)->GetColorAt (TextureX,TextureY);
}

SENSCLASSVECTOR& SIM3DPolyeder::GetColorOfFaceAt2 (INTEGER n, const VECTOR& p,
                                                   const VECTOR& v,
                                                   REAL TextureX,REAL TextureY)
{
  CheckFaceNr (n);
  VECTOR normal = NewFaceDir [Faces [n].NormalDir];
  if (normal * v > 0) normal = -normal;
  return (World->LightAt (p, normal) *= GetColorOfFaceAt(n,TextureX,TextureY));
}

SENSCLASSVECTOR& SIM3DPolyeder::GetColorOfFaceAt (INTEGER n, const VECTOR& p,
                                                  const VECTOR& v)
{
  REAL x = 0,
       y = 0;
  if(GetSurface (n)->HasTexture())
    GetPlaneCoord (n, p, x, y);
  return GetColorOfFaceAt(n,p,v,x,y);
}
  
SENSCLASSVECTOR& SIM3DPolyeder::GetColorOfFaceAt (INTEGER n, const VECTOR& p,
                                                  const VECTOR& v,
                                                  REAL TextureX, REAL TextureY)
{
  if(World->QuickShading) {
    if(!Faces[n].IsPrecalculated)
    {
      if(GetSurface(n)->HasTexture())
      {
        VECTOR normal = NewFaceDir [Faces [n].NormalDir];
        if (normal * v > 0) normal = -normal;
        Faces[n].Color = World->LightAt (NewPoints [Faces [n].FixPoint], normal);
      }
      else
        Faces[n].Color = GetColorOfFaceAt2(n,NewPoints [Faces [n].FixPoint],v,
                                           TextureX,TextureY);
      Faces[n].IsPrecalculated = TRUE;
    }
    if(GetSurface(n)->HasTexture())
    {
      scvTemp = Faces[n].Color * GetColorOfFaceAt (n,TextureX,TextureY); 
      return scvTemp;
    }
    else
      return Faces[n].Color;
  }
  else
    return GetColorOfFaceAt2(n,p,v,TextureX,TextureY);
}

BOOLEAN SIM3DPolyeder::GetPlaneCoord (INTEGER nr, const VECTOR& p,
                                      REAL& x, REAL& y)
{
  CheckFaceNr (nr);
  VECTOR r = p - NewPoints [Faces [nr].FixPoint];
  VECTOR v = NewFaceDir [Faces [nr].HorizDir] * Faces [nr].SizeX;
  VECTOR u = NewFaceDir [Faces [nr].VertDir]  * Faces [nr].SizeY;
  VECTOR n = NewFaceDir [Faces [nr].NormalDir];

  REAL d = det (v, u, n);
  if (abs (d) > EPSILON) {
    x = det (r, u, n) / d;
    y = det (v, r, n) / d;
    return TRUE;
  }
  else
    return FALSE;
}

BOOLEAN SIM3DPolyeder::DoesPointLieInFaceNr (const VECTOR& p,INTEGER nr)
// Liegt der Punkt 'p' in der angegebenen Flaeche? Die Flaeche wird auf eine
// der drei von den Achse aufgespannten Ebenen projeziert, dann wird ein
// einfacher Even-Odd-Test durchgefuehrt.
{
  INTEGER EvenOddCount = 0;
  INTEGER first, last, actual;
  REAL s;
  VECTOR v1, v2;

  if (nr < 0 || nr >= Faces.HowMany ()) sim3DFatalError (sim3DOutOfRange);

  VECTOR Normal = NewFaceDir [Faces [nr].NormalDir];

  first = Faces [nr].FirstEdge;
  last  = Faces [nr].NextEdge-1;

  if ((abs (Normal.x) > abs (Normal.y)) &&
      (abs (Normal.x) > abs (Normal.z))) {

    for (actual=first;actual<=last;actual++) {
      GetEdge (actual,v1,v2);
      if (((v1.y > p.y) && (v2.y <= p.y)) ||
          ((v1.y <= p.y) && (v2.y > p.y))) {
        s = (v1.z - v2.z) / (v1.y - v2.y) * (p.y - v2.y) + v2.z;
        if (s < p.z) EvenOddCount++;
      }
    }

  }
  else if (abs (Normal.y) > abs (Normal.z)) {

    for (actual=first;actual<=last;actual++) {
      GetEdge (actual,v1,v2);
      if (((v1.z > p.z) && (v2.z <= p.z)) ||
          ((v1.z <= p.z) && (v2.z > p.z))) {
        s = (v1.x - v2.x) / (v1.z - v2.z) * (p.z - v2.z) + v2.x;
        if (s < p.x) EvenOddCount++;
      }
    }

  }
  else {

    for (actual=first;actual<=last;actual++) {
      GetEdge (actual,v1,v2);
      if (((v1.y > p.y) && (v2.y <= p.y)) ||
          ((v1.y <= p.y) && (v2.y > p.y))) {
        s = (v1.x - v2.x) / (v1.y - v2.y) * (p.y - v2.y) + v2.x;
        if (s < p.x) EvenOddCount++;
      }
    }

  }
  return ((EvenOddCount % 2) == 1);
}

void SIM3DPolyeder::WhereDoesRayCutPlaneNr
  (const RAY& ray,INTEGER nr,BOOLEAN& does,REAL& where)
// Ein bisschen Vektorarithmetik, um den Schnittpunkt zu bestimmen.
{
  VECTOR planep   = NewPoints  [Faces [nr].FixPoint];
  VECTOR planedir = NewFaceDir [Faces [nr].NormalDir];

  REAL   prod1    = (planep - ray.Start) * planedir;
  REAL   prod2    = ray.Direction * planedir;

  if (abs (prod2) > EPSILON) {
    does   = TRUE;
    where  = prod1 / prod2;
  }
  else
    does   = FALSE;
}

void SIM3DPolyeder::FindFirstCutWith
  (const RAY& ray, BOOLEAN& exist, REAL& where, INTEGER& facenr)
// Hier muessen alle Flaechen durchgegangen werden, um den raeumlich naechsten
// Schnittpunkt mit dem Polyeder zu bestimmen.
{
  BOOLEAN exist2;
  REAL    where2;

  exist = FALSE;

  if (!Closed) {
    sim3DOtherError (sim3DNotInitialized);
    return;
  }

  INTEGER count = Faces.HowMany ();
  INTEGER i;
  for (i=0;i<count;i++) {
    WhereDoesRayCutPlaneNr (ray, i, exist2, where2);
    if (exist2 && (where2 > EPSILON) && (!exist || (where2 < where)))
      if (DoesPointLieInFaceNr (ray.Start + ray.Direction * where2, i)) {
        exist = TRUE;
        where = where2;
        facenr = i;
      }
  }
}

BOOLEAN SIM3DPolyeder::IntersectionFound (SIM3DPolyeder* Obj)
// Hier werden alle Kanten von 'this' mit allen Flaechen von 'Obj' geschnitten
// und umgekehrt. Ein Schnitt genuegt, um TRUE zu liefern.
{
  if (Obj == this) return FALSE;

  if (!Closed) {
    sim3DOtherError (sim3DNotInitialized);
    return FALSE;
  }

  INTEGER edgeNr, edgeCount;
  INTEGER planeNr, planeCount;
  BOOLEAN does;
  REAL where;
  VECTOR v1,v2;

  edgeCount = Edges.HowMany () / 2;
  planeCount = Obj->Faces.HowMany ();

  for (edgeNr=0;edgeNr<edgeCount;edgeNr++) {
    GetEdgeOnce (edgeNr, v1, v2);
    RAY ray (v1, v2);
    for (planeNr=0;planeNr<planeCount;planeNr++) {
      Obj->WhereDoesRayCutPlaneNr (ray, planeNr, does, where);
      if (does && (where > 0.0) && (where < 1.0) &&
          Obj->DoesPointLieInFaceNr
            (ray.Start + ray.Direction * where, planeNr) )
        return TRUE;
    }
  }

  edgeCount = Obj->Edges.HowMany () / 2;
  planeCount = Faces.HowMany ();

  for (edgeNr=0;edgeNr<edgeCount;edgeNr++) {
    Obj->GetEdgeOnce (edgeNr, v1, v2);
    RAY ray (v1, v2);
    for (planeNr=0;planeNr<planeCount;planeNr++) {
      WhereDoesRayCutPlaneNr (ray, planeNr, does, where);
      if (does && (where > 0.0) && (where < 1.0) &&
          DoesPointLieInFaceNr
            (ray.Start + ray.Direction * where, planeNr) )
        return TRUE;
    }
  }
  return FALSE;
}


BOOLEAN SIM3DPolyeder::DoesIntersect (SIM3DElement* Obj)
// Test auf Schnitt von 'this' mit einem beliebigen Objekt. Wenn 'Obj' ein
// SIM3DBody ist, so muss es ein SIM3DPolyeder sein, um den Schnitt durchfuehren
// zu koennen. Ansonsten wird angenommen, dass es ein SIM3DGroup-Objekt ist,
// und 'this' und 'Obj' werden vertauscht, um einen Schnitt mit den Unter-
// objekten von 'Obj' zu erreichen.
{
  if (Obj->IsA (S3DBody)) {
    if (Obj->IsA (S3DPolyeder))
      return IntersectionFound ((SIM3DPolyeder*) Obj);
    else {
      sim3DFatalError (sim3DNotImplemented);
      return FALSE;
    }
  }
  else
    return Obj->DoesIntersect (this);
}

BOUNDINGBOX SIM3DPolyeder::GetBoundingBoxOfFace (INTEGER nr)
// Berechne das umgebende Rechteck aus 'NewPoints'
{
  BOUNDINGBOX result;
  INTEGER first, last, index;

  first = Faces [nr].FirstEdge;
  last  = Faces [nr].NextEdge-1;

  for (index=first;index<=last;index++)
    result.AddPoint (NewPoints [abs (Vertices [index])-1]);

  return result;
}

void SIM3DPolyeder::Draw (GRAPHICPORT* graphic, const MOVEMATRIX& M,
                          INTEGER, REAL)
// Zeichne den Polyeder. Die mit DrawEdge gezeichneten Kanten sollten evtl. 
// nicht ausgegeben werden, wenn die mit DrawPolygon angegebenen Flaechen 
// gezeichnet werden (Da die Kanten die Kanten der Flaechen sind), aber dies
// ist die Aufgabe des GRAPHICPORTs
{
  INTEGER edgei, edgeCount = Edges.HowMany () / 2;
  VECTOR v1, v2;
  COLOR col = GetColor ();

  for (edgei=0;edgei<edgeCount;edgei++)
  {
    v1 = BasicPoints [Edges [edgei*2]-1];
    v2 = BasicPoints [Edges [edgei*2+1]-1];
    graphic->DrawEdge (M*v1, M*v2, col);
  }

  INTEGER planei, planeCount = Faces.HowMany ();
  INTEGER startindex;

  if (!IsTransparent) {
    for (planei=0;planei<planeCount;planei++)
    {
      startindex = Faces [planei].FirstEdge;

      INTEGER size = Faces [planei].EdgeCount();
      VECTOR* data = new VECTOR [size];

      for (INTEGER i=0;i<size;i++)
        data [i] = M * BasicPoints [abs (Vertices [i+startindex])-1];
      graphic->DrawPolygon (size, data, col);

      delete [] data;
    }
  }
}

BOOLEAN SIM3DPolyeder::CalcRayIntersection (const RAY& ray, BOOLEAN infinite,
            REAL& where, SIM3DElement*& obj, INTEGER& facenr)
// Berechne den ersten Schnitt des Strahls mit dem Polygon und beruecksichtige
// dabei, ob der Strahl begrenzt ist.
{
  BOOLEAN exist;
  FindFirstCutWith (ray, exist, where, facenr);
  if (exist && (where > 1) && (! infinite)) exist = FALSE;
  if (exist) obj = this;
  return exist;
}

void SIM3DPolyeder::DrawBodyOnZBuffer (ZBUFFER& zbuf)
{
  for (INTEGER f=0;f<Faces.HowMany();f++) {
    FACESTRUCT* face = &(Faces [f]);
    face->IsPrecalculated = FALSE;
    BOOLEAN NewStart = TRUE;
    VECTOR FirstPoint, v;
    for (INTEGER e=face->FirstEdge;e<face->NextEdge;e++) {
      v = NewPoints [abs (Vertices [e])-1];
      if (NewStart) {
        FirstPoint = v;
        zbuf.NewFaceVertex (v);
        NewStart = FALSE;
      }
      else {
        zbuf.AddFaceVertex (v);
        if (Vertices [e] < 0) {
          zbuf.AddFaceVertex (FirstPoint);
          NewStart = TRUE;
        }
      }
    }
    zbuf.EnterThisFace (GetSurface(f)->HasTexture(), this, f, 
      NewPoints  [face->FixPoint],
      NewFaceDir [face->HorizDir] * face->SizeX, 
      NewFaceDir [face->VertDir]  * face->SizeY);
  }
} 
