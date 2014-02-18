/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DSensors.cc                                       ****
 **** Inhalt: Implementation der Sensor-Objekte der Simulation      ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DSensors.h"
#include "sim3DWorld.h"
#include "sim3DErrors.h"
#include "string.h"
#include "stdlib.h"

//-- Fehlerfunktion --------------------------------------------------------

REAL SIM3DSensor::ErrorFunc[201] =
{
  -.9973737,
  -.8164767,-.7680072,-.7336854,-.7061362,-.6826966,
  -.6620622,-.6434861,-.6264956,-.6107701,-.5960813,
  -.5822603,-.5691776,-.5567324,-.5448437,-.5334456,
  -.5224838,-.5119129,-.5016946,-.4917958,-.4821883,
  -.4728475,-.4637517,-.4548823,-.4462221,-.4377564,
  -.4294717,-.421356,-.4133983,-.4055889,-.3979188,
  -.3903798,-.3829645,-.3756658,-.3684776,-.3613939,
  -.3544093,-.3475188,-.3407178,-.3340019,-.3273669,
  -.3208092,-.3143253,-.3079116,-.3015651,-.2952828,
  -.2890619,-.2828998,-.276794,-.2707421,-.2647419,
  -.2587912,-.252888,-.2470305,-.2412166,-.2354447,
  -.2297131,-.2240202,-.2183645,-.2127444,-.2071585,
  -.2016056,-.1960842,-.1905931,-.1851311,-.1796969,
  -.1742896,-.168908,-.1635509,-.1582175,-.1529066,
  -.1476173,-.1423487,-.1370999,-.1318699,-.126658,
  -.1214631,-.1162847,-.1111217,-.1059734,-.1008391,
  -9.571801E-02,-9.060937E-02,-8.551243E-02,-.0804265,-7.535088E-02,
  -7.028486E-02,-6.522776E-02,-6.017892E-02,-5.513766E-02,-5.010331E-02,
  -4.507525E-02,-.0400528,-3.503535E-02,-3.002224E-02,-2.501287E-02,
  -2.000658E-02,-1.500278E-02,-1.000082E-02,-5.000102E-03, 0,
  5.000102E-03, 1.000082E-02, 1.500278E-02, 2.000658E-02, 2.501287E-02,
  3.002224E-02, 3.503535E-02, .0400528, 4.507525E-02, 5.010331E-02,
  5.513766E-02, 6.017892E-02, 6.522776E-02, 7.028486E-02, 7.535088E-02,
  .0804265, 8.551243E-02, 9.060937E-02, 9.571801E-02, .1008391,
  .1059734, .1111217, .1162847, .1214631, .126658,
  .1318699, .1370999, .1423487, .1476173, .1529066,
  .1582175, .1635509, .168908, .1742896, .1796969,
  .1851311, .1905931, .1960842, .2016056, .2071585,
  .2127444, .2183645, .2240202, .2297131, .2354447,
  .2412166, .2470305, .252888, .2587912, .2647419,
  .2707421, .276794, .2828998, .2890619, .2952828,
  .3015651, .3079116, .3143253, .3208092, .3273669,
  .3340019, .3407178, .3475188, .3544093, .3613939,
  .3684776, .3756658, .3829645, .3903798, .3979188,
  .4055889, .4133983, .421356, .4294717, .4377564,
  .4462221, .4548823, .4637517, .4728475, .4821883,
  .4917958, .5016946, .5119129, .5224838, .5334456,
  .5448437, .5567324, .5691776, .5822603, .5960813,
  .6107701, .6264956, .6434861, .6620622, .6826966,
  .7061362, .7336854, .7680072, .8164767, .9973737,
};

//-- Portdefinitionen ------------------------------------------------------

// Sensorportdefinitionen:

INTEGER GetMultiDim (SIM3DElement* obj, INTEGER dim)
{
  if (dim >= 0 && dim < 3)
    return ((SIM3DSensor*) obj)->DimSize (dim);
  else
    return 1;
}

void GetMultiValue (SIM3DElement* obj, PSHORTREAL value)
{
  SIM3DSensor* sens = (SIM3DSensor*) obj;
  sens->CalcSensorIfNec ();
  LONGINT size = sens->Size;
  for (LONGINT i=0;i<size;i++)
    value [i] = sens->Value [i];
}

SPORTDEF S3DSensor0Port (NULL, "VALUE", 0, GetMultiDim, GetMultiValue);
SPORTDEF S3DSensor1Port (NULL, "VALUE", 1, GetMultiDim, GetMultiValue);
SPORTDEF S3DSensor2Port (NULL, "VALUE", 2, GetMultiDim, GetMultiValue);
SPORTDEF S3DSensor3Port (NULL, "VALUE", 3, GetMultiDim, GetMultiValue);

//-- SIM3DSensor -----------------------------------------------------------
//
// Siehe auch "sim3DSensors.h"

SIM3DSensor::SIM3DSensor (const STRING& Name) : SIM3DGlobal (Name)
{
  IsComputed  = FALSE;
  ClassesDefined = FALSE;
  KnowClasses = FALSE;
  KnowLength  = FALSE;
  Length      = 1.0;
  Size        = 1;
  minDepth    = 0;
  maxDepth    = 1;
  Noise       = 0.0;
  Value       = ALLOC_VALUE(Size);
  *Value      = 0.0F;
  Classes.AddElement (0);
}

SIM3DSensor::SIM3DSensor (const SIM3DSensor& Obj) : SIM3DGlobal (Obj),
  Classes (Obj.Classes)
{
  IsComputed  = FALSE;
  ClassesDefined = Obj.ClassesDefined;
  KnowClasses = Obj.KnowClasses;
  KnowLength  = Obj.KnowLength;
  Length      = Obj.Length;
  Size        = Obj.Size;
  minDepth    = Obj.minDepth;
  maxDepth    = Obj.maxDepth;
  Noise       = Obj.Noise;
  if (Size > 0)
    Value     = ALLOC_VALUE(Size);
  for (LONGINT i=0;i<Size;i++) Value [i] = Obj.Value [i];
}

SIM3DSensor::~SIM3DSensor ()
{
  if (Size > 0) FREE_VALUE(Value);
}

SPORTDEF* SIM3DSensor::GetSPortDef ()
{
  if (DimSize (2) > 1)
    return &S3DSensor3Port;
  else if (DimSize (1) > 1)
    return &S3DSensor2Port;
  else if (DimSize (0) > 1)
    return &S3DSensor1Port;
  else
    return &S3DSensor0Port;
}

INTEGER SIM3DSensor::DimSize (INTEGER i)
{
  if (i == 0) {
    if (KnowClasses)
      return Classes.HowMany ();
    else
      return 1;
  }
  else
    return 1;
}

void SIM3DSensor::ResizeBuffer ()
{
  if (Size > 0)
    FREE_VALUE(Value);
  Size = 1;
  for (INTEGER i=0;i<3;i++)
    Size = Size * DimSize (i);
  if (Size > 0)
  {
    Value = ALLOC_VALUE(Size);
    for (LONGINT i = 0; i < Size; i++) Value [i] = 0.0F;
  }
}

void SIM3DSensor::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "LENGTH" && KnowLength)
    (void) (GetCorrectReal (scan, Length));
  else if (attr == "CLASS" && KnowClasses) {
    INTEGER cl;
    if (GetCorrectInteger (scan, cl)) {
      if (ClassesDefined)
        Classes.AddElement (cl);
      else {
        ClassesDefined = TRUE;
        Classes [0] = cl;
      }
      ResizeBuffer ();
    }
  }
  else if(attr == "NOISE")
    GetCorrectReal(scan,Noise);
  else
    SIM3DGlobal::ReadAttr (attr, scan);
}

void SIM3DSensor::WriteAttrs (BACKUP& backup)
{
  SIM3DGlobal::WriteAttrs (backup);
  if (KnowLength) backup.WriteIdRealNext ("LENGTH",Length);
  if (ClassesDefined) {
    for (INTEGER i=0;i<Classes.HowMany();i++)
      if (Classes [i] == -1) {
        backup.WriteIdent  ("DEPTH");
        backup.WriteSymbol (sLParen);
        backup.WriteReal   (minDepth);
        backup.WriteSymbol (sComma);
        backup.WriteReal   (maxDepth);
        backup.WriteSymbol (sRParen);
        backup.ListNext    ();
      }
      else
        backup.WriteIdIntNext ("CLASS", Classes [i]);
  }
  if(Noise)
    backup.WriteIdRealNext("NOISE",Noise);
}

COLOR SIM3DSensor::GetColor ()
{
  if (Color < 0)
    return World->SensorColor;
  else
    return Color;
}

void SIM3DSensor::ComputeSensors ()
{
  if (Registered) {
    IsComputed = FALSE;
    if (World->CalcSensorsImmediate) CalcSensorIfNec ();
  }
}

void SIM3DSensor::CalcSensorIfNec ()
{
  if (! IsComputed) {
    CalcSensor ();
    IsComputed = TRUE;
    if(Noise)
      if(IsA(S3DCompass))
        Value[0] = (SHORTREAL) fmod(100 + Value[0] +
                        Noise * ErrorFunc[rand() % 201],1);
      else
        for(LONGINT i=0;i<Size;i++)
          Value[i] += (SHORTREAL) (Noise * ErrorFunc[rand() % 201]);
  }
}

//-- SIM3DSingleSensor -----------------------------------------------------
//
// Siehe auch "sim3DSensors.h"

SIM3DSingleSensor::SIM3DSingleSensor (const STRING& Name) : SIM3DSensor (Name)
{
}

//-- SIM3DMultiSensor -----------------------------------------------------
//
// Siehe auch "sim3DSensors.h"

SIM3DMultiSensor::SIM3DMultiSensor (const STRING& Name) : SIM3DSensor (Name)
{
  SizeX = 0;
  SizeY = 0;
  DeltaX = 1.0;
  DeltaY = 1.0;
}

SIM3DMultiSensor::SIM3DMultiSensor (const SIM3DMultiSensor& Obj) :
  SIM3DSensor (Obj)
{
  SizeX = Obj.SizeX;
  SizeY = Obj.SizeY;
  DeltaX = Obj.DeltaX;
  DeltaY = Obj.DeltaY;
}

INTEGER SIM3DMultiSensor::DimSize (INTEGER i)
{
  if (i == 2) {
    if (KnowClasses)
      return Classes.HowMany ();
    else
      return 1;
  }
  else if (i == 1)
    return SizeY;
  else if (i == 0)
    return SizeX;
  else
    return 1;
}

void SIM3DMultiSensor::ReadAttr (const STRING& attr, SCANNER& scan)
{
  INTEGER i1, i2;
  REAL r1, r2;

  if (attr == "RESOLUTION") {
    if (GetCorrectIntPair (scan, i1, i2)) {
      SizeX = i1;
      SizeY = i2;
      ResizeBuffer ();
    }
  }
  else if (attr == "SIZE") {
    if (GetCorrectRealPair (scan, r1, r2)) {
      DeltaX = r1;
      DeltaY = r2;
    }
  }
  else
    SIM3DSensor::ReadAttr (attr, scan);
}

void SIM3DMultiSensor::WriteAttrs (BACKUP& backup)
{
  SIM3DSensor::WriteAttrs (backup);

  backup.WriteIdent   ("RESOLUTION");
  backup.WriteSymbol  (sLParen);
  backup.WriteInteger (SizeX);
  backup.WriteSymbol  (sComma);
  backup.WriteInteger (SizeY);
  backup.WriteSymbol  (sRParen);
  backup.ListNext     ();

  backup.WriteIdent  ("SIZE");
  backup.WriteSymbol (sLParen);
  backup.WriteReal   (DeltaX);
  backup.WriteSymbol (sComma);
  backup.WriteReal   (DeltaY);
  backup.WriteSymbol (sRParen);
  backup.ListNext    ();
}

//-- SIM3DWhisker ----------------------------------------------------------

SIM3DWhisker::SIM3DWhisker (const STRING& Name) : SIM3DSingleSensor (Name)
{
  KnowLength = TRUE;
}

void SIM3DWhisker::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  Value [0] = (SHORTREAL) World->CalcWhiskerRay (
           RAY (GlobalPos.Offset, GlobalPos * VECTOR (0.0, 0.0, Length)));
}

COLOR SIM3DWhisker::GetColor ()
{
  if (Color < 0)
    return World->WhiskerColor;
  else
    return Color;
}

void SIM3DWhisker::Draw (GRAPHICPORT* G, const MOVEMATRIX& M,INTEGER,REAL)
{
  VECTOR v1 = M * VECTOR ();
  VECTOR v2 = M * VECTOR (0,0,Length);
  G->DrawLine (v1,v2, GetColor ());
}


//-- SIM3DTactile ----------------------------------------------------------

SIM3DTactile::SIM3DTactile (const STRING& Name) : SIM3DWhisker (Name)
{
  KnowClasses = TRUE;
}

void SIM3DTactile::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  SENSCLASSVECTOR scv = World->CalcTactileRay (
            RAY (GlobalPos.Offset, GlobalPos * VECTOR (0.0, 0.0, 1.0)));
  for (INTEGER i=0;i<Classes.HowMany();i++)
    Value [i] = scv.Index (Classes [i]);
}

//-- SIM3DReceptor ----------------------------------------------------------

SIM3DReceptor::SIM3DReceptor (const STRING& Name) : SIM3DSingleSensor (Name)
{
  KnowClasses = TRUE;
}

void SIM3DReceptor::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  SENSCLASSVECTOR scv = World->CalcReceptorRay (
            RAY (GlobalPos.Offset, GlobalPos * VECTOR (0.0, 0.0, 1.0)));
  for (INTEGER i=0;i<Classes.HowMany();i++)
    Value [i] = scv.Index (Classes [i]);
}

void SIM3DReceptor::Draw (GRAPHICPORT* G, const MOVEMATRIX& M, INTEGER,
                          REAL scale)
{
  VECTOR v1 = M * VECTOR ();
  VECTOR v2 = M * VECTOR (0,0,scale);
  G->DrawLine (v1,v2, GetColor ());
}


//-- SIM3DWhiskerField -----------------------------------------------------

SIM3DWhiskerField::SIM3DWhiskerField (const STRING& Name)
: SIM3DMultiSensor (Name)
{
  KnowLength = TRUE;
}

void SIM3DWhiskerField::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  for (INTEGER y=0;y<SizeY;y++) {
    for (INTEGER x=0;x<SizeX;x++) {
      REAL xx = (SizeX > 1 ? DeltaX/(SizeX-1)*x - DeltaX/2.0 : 0.0);
      REAL yy = (SizeY > 1 ? DeltaY/(SizeY-1)*y - DeltaY/2.0 : 0.0);
      Value [x+y*SizeX] = (SHORTREAL) World->CalcWhiskerRay (
              RAY (GlobalPos * VECTOR (xx, yy, 0.0),
                   GlobalPos * VECTOR (xx, yy, Length)));
    }
  }
}

COLOR SIM3DWhiskerField::GetColor ()
{
  if (Color < 0)
    return World->WhiskerColor;
  else
    return Color;
}

void SIM3DWhiskerField::Draw (GRAPHICPORT* G, const MOVEMATRIX& M, INTEGER,
                              REAL)
// Zeichnen des Whiskerfeldes als Rechteck, mit angedeuteten Tasthaaren an den
// vier Ecken.
{
  COLOR col = GetColor ();
  REAL xx = DeltaX/2.0;
  REAL yy = DeltaY/2.0;
  VECTOR v1 = M * VECTOR (xx,yy,0);
  VECTOR v2 = M * VECTOR (-xx,yy,0);
  VECTOR v3 = M * VECTOR (-xx,-yy,0);
  VECTOR v4 = M * VECTOR (xx,-yy,0);
  VECTOR v5 = M * VECTOR (xx,yy,Length);
  VECTOR v6 = M * VECTOR (-xx,yy,Length);
  VECTOR v7 = M * VECTOR (-xx,-yy,Length);
  VECTOR v8 = M * VECTOR (xx,-yy,Length);
  G->DrawLine (v1,v2,col);
  G->DrawLine (v2,v3,col);
  G->DrawLine (v3,v4,col);
  G->DrawLine (v4,v1,col);
  G->DrawLine (v1,v5,col);
  G->DrawLine (v2,v6,col);
  G->DrawLine (v3,v7,col);
  G->DrawLine (v4,v8,col);
}

//-- SIM3DCamera ----------------------------------------------------------

SIM3DCamera::SIM3DCamera (const STRING& Name) : SIM3DMultiSensor (Name)
{
  KnowClasses = TRUE;
  UseZBuffer = FALSE;
}

void SIM3DCamera::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "ZBUFFER")
    UseZBuffer = TRUE;
  else if (attr == "DEPTH") {
    if (GetCorrectRealPair (scan, minDepth, maxDepth)) {
      if (minDepth == maxDepth)
        sim3DOtherError (sim3DExpectDifferent,scan.GetLastPos ());
    }
    if (ClassesDefined)
      Classes.AddElement (-1);
    else {
      ClassesDefined = TRUE;
      Classes [0] = -1;
    }
    ResizeBuffer ();
  }
  else
    SIM3DMultiSensor::ReadAttr (attr, scan);
}

void SIM3DCamera::WriteAttrs (BACKUP& backup)
{
  SIM3DMultiSensor::WriteAttrs (backup);
  if (UseZBuffer) {
    backup.WriteIdent ("ZBUFFER");
    backup.ListNext ();
  }
}

void SIM3DCamera::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  if (UseZBuffer)
    World->CalcCameraView (GlobalPos, DeltaX, DeltaY,
                          Classes, SizeX, SizeY, minDepth, maxDepth, Value);
  else {
    VECTOR hedge = GlobalPos.Matrix * VECTOR (DeltaX/SizeX, 0, 0);
    VECTOR vedge = GlobalPos.Matrix * VECTOR (0, DeltaY/SizeY, 0);
    VECTOR p1    = GlobalPos        * VECTOR (-DeltaX/2, -DeltaY/2, 1);
    LONGINT size = (LONGINT) SizeX * (LONGINT) SizeY;
    for (INTEGER y = 0; y < SizeY; y++) {
      VECTOR p2 = p1;
      for (INTEGER x = 0; x < SizeX; x++) {
        REAL Depth;
        SENSCLASSVECTOR& scv =
          World->CalcCameraRay (RAY (GlobalPos.Offset, p2),Depth);
        for (INTEGER i = 0; i < Classes.HowMany(); i++)
          Value [i*size + y*(LONGINT)SizeX + x] =
            (SHORTREAL) (Classes[i] == -1
              ? (Depth == -1
                ? 1
                : (Depth - minDepth) / (maxDepth - minDepth))
              : scv.Index (Classes [i]));
        p2 = p2 + hedge;
      }
      p1 = p1 + vedge;
    }
  }
}

void SIM3DCamera::Draw (GRAPHICPORT* G, const MOVEMATRIX& M, INTEGER,
                        REAL scale)
// Zeichnen der Kamera als Sichtpyramide
{
  COLOR col = GetColor ();
  REAL xx = DeltaX/2.0 * scale;
  REAL yy = DeltaY/2.0 * scale;
  VECTOR v0 = M * VECTOR (0,0,0);
  VECTOR v1 = M * VECTOR (xx,yy,scale);
  VECTOR v2 = M * VECTOR (-xx,yy,scale);
  VECTOR v3 = M * VECTOR (-xx,-yy,scale);
  VECTOR v4 = M * VECTOR (xx,-yy,scale);
  G->DrawLine (v0,v1,col);
  G->DrawLine (v0,v2,col);
  G->DrawLine (v0,v3,col);
  G->DrawLine (v0,v4,col);
  G->DrawLine (v1,v2,col);
  G->DrawLine (v2,v3,col);
  G->DrawLine (v3,v4,col);
  G->DrawLine (v4,v1,col);
}

//- SIM3DFacette -----------------------------------------------------------


SIM3DFacette::SIM3DFacette (const STRING& Name) : SIM3DMultiSensor (Name)
{
  KnowClasses = TRUE;
}

void SIM3DFacette::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "DEPTH") {
    if (GetCorrectRealPair (scan, minDepth, maxDepth)) {
      if (minDepth == maxDepth)
        sim3DOtherError (sim3DExpectDifferent,scan.GetLastPos ());
    }
    if (ClassesDefined)
      Classes.AddElement (-1);
    else {
      ClassesDefined = TRUE;
      Classes [0] = -1;
    }
    ResizeBuffer ();
  }
  else
    SIM3DMultiSensor::ReadAttr (attr, scan);
}

void SIM3DFacette::CalcSensor ()
// Siehe auch "sim3DWorld.h"
{
  REAL dx = DeltaX / SizeX;
  REAL dy = DeltaY / SizeY;

  LONGINT size = (LONGINT) SizeX * (LONGINT) SizeY;

  REAL wy = (DeltaY-dy)/2;
  for (INTEGER y = 0; y < SizeY; y++) {
    REAL wx = (dx-DeltaX)/2;
    for (INTEGER x = 0; x < SizeX; x++) {
      REAL Depth;
      SENSCLASSVECTOR& scv = World->CalcCameraRay (RAY (GlobalPos.Offset, 
        GlobalPos.Matrix * TurnXZMatrix (wx) * TurnYZMatrix (wy) * 
          VECTOR (0,0,1) + GlobalPos.Offset),Depth);
      for (INTEGER i = 0; i < Classes.HowMany(); i++)
        Value [i*size + y*(LONGINT)SizeX + x] =
          (SHORTREAL) (Classes[i] == -1
            ? (Depth - minDepth) / (maxDepth - minDepth)
            : scv.Index (Classes [i]));
      wx = wx + dx;
    }
    wy = wy - dy;
  }
}

inline INTEGER absi (REAL f)
{
  return (INTEGER) (f < 0 ? -f : f);
}

void SIM3DFacette::Draw (GRAPHICPORT* G, const MOVEMATRIX& M, INTEGER,
                         REAL scale)
// Zeichnen der Kamera als Winkelangaben
{
  COLOR col = GetColor ();

  INTEGER sx = absi (DeltaX / 20) + 1;
  INTEGER sy = absi (DeltaY / 20) + 1;
  REAL dx = DeltaX / sx;
  REAL dy = DeltaY / sy;

  INTEGER i;

  VECTOR v = M.Offset;
  REAL w = -DeltaY/2;
  for (i = 0; i <= sy; i++) {
    VECTOR v2 = M.Matrix * TurnYZMatrix (w) *
        VECTOR (0,0,scale) + M.Offset;
    G->DrawLine (v, v2, col);
    v = v2;
    w = w + dy;
  }
  G->DrawLine (v, M.Offset, col);

  v = M.Offset;
  w = -DeltaX/2;
  for (i = 0; i <= sx; i++) {
    VECTOR v2 = M.Matrix * TurnXZMatrix (w) *
        VECTOR (0,0,scale) + M.Offset;
    G->DrawLine (v, v2, col);
    v = v2;
    w = w + dx;
  }
  G->DrawLine (v, M.Offset, col);
}

//-- SIM3DUltraSonic -------------------------------------------------------

SIM3DUltraSonic::SIM3DUltraSonic(const STRING& Name)
: SIM3DCamera(Name)
{
  KnowClasses = FALSE;
  Classes [0] = -1;
}

SIM3DUltraSonic::SIM3DUltraSonic(const SIM3DUltraSonic& Obj)
: SIM3DCamera(Obj)
{
  KnowClasses = FALSE;
}

void SIM3DUltraSonic::ReadAttr (const STRING& attr, SCANNER& scan)
{
  REAL r1,r2;
  if (attr == "SIZE") {
    if (GetCorrectRealPair (scan, r1, r2)) {
      if(r1 >= 0 && r1 <= 120 && r2 >= 0 && r2 <= 120)
      {
        DeltaX = 2 * tan(r1 / 360 * PI);
        DeltaY = 2 * tan(r2 / 360 * PI);
      }
      else
        sim3DOtherError (sim3DOpeningAngle,scan.GetLastPos ());
    }
  }
  else
    SIM3DCamera::ReadAttr (attr, scan);
}

void SIM3DUltraSonic::WriteAttrs (BACKUP& backup)
{
  REAL dx = DeltaX,
       dy = DeltaY;
  DeltaX = atan(DeltaX / 2) / PI *360;
  DeltaY = atan(DeltaY / 2) / PI *360;
  SIM3DCamera::WriteAttrs (backup);
  DeltaX = dx;
  DeltaY = dy;
}

// ein Ultraschallsensor ist eine Art Whiskerfield mit sehr langen Haaren,
// die alle in einem Punkt ihren Ursprung haben und der
// nur den Wert des am staerksten verbogenen Haars zurueckgibt.
// der Sensor beruecksichtigt nur Hindernisse im Entfernungsintervall
// [minDepth..maxDepth], welches auf [0..1] herunterskaliert wird.
// Alle Entfernungen kleiner minDepth werden als minDepth erkannt,
// alle groesser maxDepth als maxDepth.
void SIM3DUltraSonic::CalcSensor()
{
  REAL mindepth = 1.0e6;
  if(UseZBuffer)
    mindepth = World->CalcUltraSonic(GlobalPos,DeltaX,DeltaY,SizeX,SizeY);
  else
  {
    VECTOR hedge = GlobalPos.Matrix * VECTOR (DeltaX/SizeX, 0, 0);
    VECTOR vedge = GlobalPos.Matrix * VECTOR (0, DeltaY/SizeY, 0);
    VECTOR p1    = GlobalPos        * VECTOR (-DeltaX/2, -DeltaY/2, 1);
    for (INTEGER y = 0; y < SizeY; y++) {
      VECTOR p2 = p1;
      for (INTEGER x = 0; x < SizeX; x++) {
        REAL Depth = World->CalcUltraSonicRay (RAY (GlobalPos.Offset, p2));
        if(Depth < mindepth)
          mindepth = Depth;
        p2 = p2 + hedge;
      }
      p1 = p1 + vedge;
    }
  }
  // Tiefe auf [0..1] herunterskalieren
  Value[0] = (SHORTREAL) ((mindepth - minDepth) / (maxDepth - minDepth));
}

//-- SIM3DCompass ----------------------------------------------------------

SIM3DCompass::SIM3DCompass (const STRING& Name) : SIM3DSingleSensor (Name)
{
}

void SIM3DCompass::CalcSensor ()
{
  REAL Direction,
       Ignore;
  ExtractAngles(GlobalPos.Matrix,Ignore,Ignore,Direction);
  Value [0] = (SHORTREAL) ((Direction < 0 ? Direction + 360 
                                          : Direction) / 360);
}

void SIM3DCompass::Draw (GRAPHICPORT* G, const MOVEMATRIX& M, INTEGER,
                         REAL scale)
// Zeichnen der Kamera als Winkelangaben
{
  COLOR col = GetColor ();
  VECTOR v1 = M * VECTOR (scale,0,0);
  VECTOR v2 = M * VECTOR (-scale,scale/2,0);
  VECTOR v3 = M * VECTOR (-scale,-scale/2,0);
  G->DrawLine (v1,v2,col);
  G->DrawLine (v1,v3,col);
  G->DrawLine (v2,M.Offset,col);
  G->DrawLine (v3,M.Offset,col);
}

