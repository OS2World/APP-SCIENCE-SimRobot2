/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  sim3DWorld.cc                                          ****
 **** Inhalt: Implementation der zentralen Dateninstanz             ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DErrors.h"
#include "sim3DWorld.h"
#include "sim3DReader.h"

SIM3DWorld::SIM3DWorld (const STRING& Name) : SIM3DGroup (Name),
  StandardRadiation (0, 1.0F),
  StandardSurface ("$$STANDARD"),
  Ambient (0, 0.0F),
  Background (0, 0.0F),
  scvTemp (0,0.0F)
{
  World = this;
  Registered    = TRUE;
  LastEmitter   = NULL;
  LastSurface   = NULL;
  LastTexture   = NULL;
  LastMacro     = NOELEMENT;
  CalcSensorsImmediate = FALSE;
  Shadows       = FALSE;
  QuickShading  = FALSE;
  QuickMoving   = FALSE;
  StandardColor = ColorList.AddColorDef ("Standard",0,0,0);
  BodyColor     = StandardColor;
  EmitterColor  = ColorList.AddColorDef ("Emitter",0,0.7,0.3);
  SensorColor   = ColorList.AddColorDef ("Sensor",1,0,0);
  WhiskerColor  = ColorList.AddColorDef ("Whisker",0.7,0.3,0);
  Steps = 0;
}

SIM3DWorld::SIM3DWorld (const SIM3DWorld& Obj) : SIM3DGroup (Obj),
  StandardRadiation (Obj.StandardRadiation),
  StandardSurface (Obj.StandardSurface),
  Ambient (Obj.Ambient),
  Background (Obj.Background),
  scvTemp (0,0.0F)
{
  World = this;
  Registered    = TRUE;
  LastEmitter   = NULL;
  LastSurface   = NULL;
  LastMacro     = NOELEMENT;
  CalcSensorsImmediate = Obj.CalcSensorsImmediate;
  Shadows       = Obj.Shadows;
  QuickShading  = Obj.QuickShading;
  QuickMoving   = Obj.QuickMoving;
  StandardColor = Obj.StandardColor;
  BodyColor     = Obj.BodyColor;
  EmitterColor  = Obj.EmitterColor;
  SensorColor   = Obj.SensorColor;
  WhiskerColor  = Obj.WhiskerColor;

  Steps = Obj.Steps;

  if (Obj.LastSurface != NULL) {                // Kopieren der Oberflaechen
    SURFACEDESCR* LoopSurface = LastSurface;
    do {
      LoopSurface = LoopSurface->Next;
      AddSurface (new SURFACEDESCR (*LoopSurface));
    } while (LoopSurface != LastSurface);
  }

  if (Obj.LastTexture != NULL) {                // Kopieren der Oberflaechen
    TEXTUREDESCR* LoopTexture = LastTexture;
    do {
      LoopTexture = LoopTexture->Next;
      AddTexture (new TEXTUREDESCR (*LoopTexture));
    } while (LoopTexture != LastTexture);
  }

  if (Obj.LastMacro != NOELEMENT) {             // Kopieren der Makros
    SIM3DElement* LoopMacro = LastMacro;
    do {
      LoopMacro = LoopMacro->Next;
      AddMacro (LoopMacro->Copy ());
    } while (LoopMacro != LastMacro);
  }
}

SIM3DWorld::~SIM3DWorld ()
{
  if (LastSurface != NULL) {                    // Loeschen der Oberflaechen
    while (LastSurface != LastSurface->Next) {
      SURFACEDESCR* surface = LastSurface->Next->Next;
      delete LastSurface->Next;
      LastSurface->Next = surface;
    }
    delete LastSurface;
    LastSurface = NULL;
  }

  if (LastTexture != NULL) {                    // Loeschen der Texturen
    while (LastTexture != LastTexture->Next) {
      TEXTUREDESCR* texture = LastTexture->Next->Next;
      delete LastTexture->Next;
      LastTexture->Next = texture;
    }
    delete LastTexture;
    LastTexture = NULL;
  }

  if (LastMacro != NOELEMENT) {                 // Entfernen der Makros
    while (LastMacro != LastMacro->Next) {
      SIM3DElement* obj = LastMacro->Next->Next;
      delete LastMacro->Next;
      LastMacro->Next = obj;
    }
    delete LastMacro;
    LastMacro = NOELEMENT;
  }
}

LONGINT SIM3DWorld::GetSteps ()
{
  return Steps;
}

void SIM3DWorld::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "AMBIENT")
    Ambient.ReadVector (scan);
  else if (attr == "BACKGROUND")
    Background.ReadVector (scan);
  else if (attr == "SHADOWS") {
    Shadows = TRUE;
    QuickShading = FALSE;
  }
  else if (attr == "QUICKSHADING") {
    Shadows = FALSE;
    QuickShading = TRUE;
  }
  else if (attr == "QUICKMOVING")
    QuickMoving = TRUE;
  else if (attr == "STANDARDCOLOR")
    StandardColor = ColorList.ReadColor (scan);
  else if (attr == "BODYCOLOR")
    BodyColor = ColorList.ReadColor (scan);
  else if (attr == "EMITTERCOLOR")
    EmitterColor = ColorList.ReadColor (scan);
  else if (attr == "SENSORCOLOR")
    SensorColor = ColorList.ReadColor (scan);
  else if (attr == "WHISKERCOLOR")
    WhiskerColor = ColorList.ReadColor (scan);
  else if (attr == "SURFACE") {
    if (scan.TestFor (sString, sSemicolon)) {
      SURFACEDESCR* surface = new SURFACEDESCR (scan.GetLastString ());
      scan.NextSymbol ();
      surface->ReadProperties (scan, this);
      AddSurface (surface);
    }
  }
  else if (attr == "TEXTURE") {
    TEXTUREDESCR* td = TextureLoader (scan);
    if (td != NULL) AddTexture (td);
  }
  else if (attr == "MACRO") {
    SIM3DElement* Obj = ReadObject (scan, this);
    if (Obj != NOELEMENT) AddMacro (Obj);
  }
  else
    SIM3DGroup::ReadAttr(attr,scan);
}

void SIM3DWorld::WriteAttrs (BACKUP& backup)
{
  SIM3DGroup::WriteAttrs(backup);
  backup.WriteIdent ("AMBIENT");
  Ambient.WriteVector (backup);
  backup.ListNext ();
  backup.WriteIdent ("BACKGROUND");
  Background.WriteVector (backup);
  backup.ListNext ();
  if (Shadows) {
    backup.WriteIdent ("SHADOWS");
    backup.ListNext ();
  }
  if (QuickShading) {
    backup.WriteIdent ("QUICKSHADING");
    backup.ListNext ();
  }
  if (QuickMoving) {
    backup.WriteIdent ("QUICKMOVING");
    backup.ListNext ();
  }
  ColorList.WriteColor (backup,"STANDARDCOLOR",StandardColor);
  ColorList.WriteColor (backup,"BODYCOLOR",    BodyColor);
  ColorList.WriteColor (backup,"EMITTERCOLOR", EmitterColor);
  ColorList.WriteColor (backup,"SENSORCOLOR",  SensorColor);
  ColorList.WriteColor (backup,"WHISKERCOLOR", WhiskerColor);

  TEXTUREDESCR* LoopTexture = LastTexture;
  if (LoopTexture != NULL) {
    do {
      LoopTexture = LoopTexture->Next;
      backup.WriteIdent ("TEXTURE");
      LoopTexture->WriteProperties (backup);
      backup.ListNext ();
    } while (LoopTexture != LastTexture);
  }
  SURFACEDESCR* LoopSurface = LastSurface;
  if (LoopSurface != NULL) {
    do {
      LoopSurface = LoopSurface->Next;
      backup.WriteIdent ("SURFACE");
      backup.WriteString (LoopSurface->Name);
      LoopSurface->WriteProperties (backup);
      backup.ListNext ();
    } while (LoopSurface != LastSurface);
  }
  SIM3DElement* LoopMacro = LastMacro;
  if (LoopMacro != NOELEMENT) {
    do {
      LoopMacro = LoopMacro->Next;
      backup.WriteIdent ("MACRO");
      LoopMacro->WriteObj (backup);
      backup.ListNext ();
    } while (LoopMacro != LastMacro);
  }
}

void SIM3DWorld::AddEmitter (SIM3DEmitter* Obj)
// Einfuegen eines Emitters in eine Ringliste
{
  if (Obj != NOELEMENT) {
    if (LastEmitter == NOELEMENT) {
      Obj->NextEmitter = Obj;
      LastEmitter = Obj;
    }
    else {
      Obj->NextEmitter = LastEmitter->NextEmitter;
      LastEmitter->NextEmitter = Obj;
      LastEmitter = Obj;
    }
  }
}

void SIM3DWorld::RemoveEmitter (SIM3DEmitter* Obj)
// Entfernen eines Emitters aus einer Ringliste
{
  if (LastEmitter != NOELEMENT) {
    SIM3DEmitter* LoopObj = LastEmitter;

    if (LoopObj == NOELEMENT)
      sim3DOtherError (sim3DInvalidObject);
    else {
      while ((LoopObj->NextEmitter != Obj) &&
             (LoopObj->NextEmitter != LastEmitter))
        LoopObj = LoopObj->NextEmitter;
      if (LoopObj->NextEmitter != Obj)
        sim3DOtherError (sim3DInvalidObject);
      else {
        if (Obj->NextEmitter == Obj)
          LastEmitter = NOELEMENT;
        else {
          LoopObj->NextEmitter = Obj->NextEmitter;
          if (LastEmitter == Obj)
            LastEmitter = LoopObj;
        }
        Obj->NextEmitter = NOELEMENT;
      }
    }
  }
}

void SIM3DWorld::AddSurface (SURFACEDESCR* surface)
// Einfuegen einer Oberflaeche in eine Ringliste
{
  if (surface != NULL) {
    if (LastSurface == NULL) {
      surface->Next = surface;
      LastSurface = surface;
    }
    else {
      surface->Next = LastSurface->Next;
      LastSurface->Next = surface;
      LastSurface = surface;
    }
  }
}

SURFACEDESCR* SIM3DWorld::FindSurface (const STRING& name)
// Finde eine Oberflaeche nach ihrem Namen
{
  if (LastSurface != NULL) {
    SURFACEDESCR* LoopSurface = LastSurface;
    do {
      LoopSurface = LoopSurface->Next;
      if (LoopSurface->Name == name)
        return LoopSurface;
    } while (LoopSurface != LastSurface);
  }
  return NULL;
}

void SIM3DWorld::AddTexture (TEXTUREDESCR* texture)
// Einfuegen einer Textur in eine Ringliste
{
  if (texture != NULL) {
    if (LastTexture == NULL) {
      texture->Next = texture;
      LastTexture = texture;
    }
    else {
      texture->Next = LastTexture->Next;
      LastTexture->Next = texture;
      LastTexture = texture;
    }
  }
}

TEXTUREDESCR* SIM3DWorld::FindTexture (const STRING& name)
// Finde eine Textur nach ihrem Namen
{
  if (LastTexture != NULL) {
    TEXTUREDESCR* LoopTexture = LastTexture;
    do {
      LoopTexture = LoopTexture->Next;
      if (LoopTexture->Name == name)
        return LoopTexture;
    } while (LoopTexture != LastTexture);
  }
  return NULL;
}

void SIM3DWorld::AddMacro (SIM3DElement* macro)
// Einfuegen eines Objekts in eine Ringliste
{
  if (macro != NOELEMENT) {
    if (LastMacro == NOELEMENT) {
      macro->Next = macro;
      LastMacro = macro;
    }
    else {
      macro->Next = LastMacro->Next;
      LastMacro->Next = macro;
      LastMacro = macro;
    }
  }
}

SIM3DElement* SIM3DWorld::FindMacro (const STRING& name)
// Finden eines Objekts nach seinem Namen
{
  if (LastMacro != NOELEMENT) {
    SIM3DElement* LoopMacro = LastMacro;
    do {
      LoopMacro = LoopMacro->Next;
      if (LoopMacro->ElementName == name)
        return LoopMacro;
    } while (LoopMacro != LastMacro);
  }
  return NOELEMENT;
}

INTEGER SIM3DWorld::HowManyMacros ()
// Objekte in Ringliste zaehlen
{
  INTEGER Count = 0;
  if (LastMacro != NOELEMENT) {
    SIM3DElement* LoopMacro = LastMacro;
    do {
      LoopMacro = LoopMacro->Next;
      Count++;
    } while (LoopMacro != LastMacro);
  }
  return Count;
}

SIM3DElement* SIM3DWorld::NthMacro (INTEGER n)
// Das n-te Objekt in einer Ringliste liefern
{
  if (LastMacro != NOELEMENT) {
    SIM3DElement* LoopMacro = LastMacro;
    do {
      LoopMacro = LoopMacro->Next;
      if (n == 0) return LoopMacro;
      n--;
    } while (LoopMacro != LastMacro);
  }
  sim3DOtherError (sim3DOutOfRange);
  return NOELEMENT;
}

void SIM3DWorld::DoTimeStep ()
// MakeTimeStep fuer die Welt ausfuehren, die Sensoren neu berechnen,
// Zahl der Zeitschritte erhoehen
{
  MOVEMATRIX M;
  MakeTimeStep (M);
  ComputeSensors ();
  Steps++;
}

void SIM3DWorld::RegisterBoundingBox () {}
// Das umgebende Rechteck fuer die Wurzel der Simulation ist nicht so
// interessant, deshalb wird es nicht beachtet.

BOOLEAN SIM3DWorld::ShadowCast (const VECTOR& p1, const VECTOR& p2)
// Berechne, ob sich zwischen den zwei Punkten ein Koerper befindet.
{
  if (! Shadows)
    return FALSE;
  else {
    REAL    where;
    SIM3DElement* obj;
    INTEGER facenr;
    return CalcRayIntersection (RAY (p1, p2), FALSE, where, obj, facenr);
  }
}

BOOLEAN SIM3DWorld::DoesIntersect (SIM3DElement* obj)
{
  if(QuickMoving)
    return FALSE;
  else
    return SIM3DGroup::DoesIntersect (obj);
}

SENSCLASSVECTOR& SIM3DWorld::LightAt (const VECTOR& p, const VECTOR& normal)
// Berechne die Lichtverhaeltnisse am Punkt 'p' durch Summation der 
// Lichteinwirkungen durch die vorhandenen Emitter. 'normal' ist der
// Normalenvektor der beschienen Flaeche und wirkt sich dahingehend aus,
// dass das Licht um so schwaecher ist, je schraeger es auftritt (diffuse
// Reflexion). Das ambiente Licht unterliegt nicht diesem Einfluss, weil
// es gleichmaessig von allen Richtungen kommt.
{
  scvTemp = Ambient;

  SIM3DEmitter* LoopObj = LastEmitter;

  if (LoopObj != NOELEMENT)
  do {
    LoopObj = LoopObj->NextEmitter;
    REAL cosangle = CosAngle (LoopObj->LightDirectionAt (p), normal);
    if (cosangle < 0 &&
        (!Shadows || !ShadowCast (p + 0.001*normal, LoopObj->GlobalPos.Offset)))
      scvTemp.MulAndAdd(*(LoopObj->GetRadiation()),
                    -cosangle * LoopObj->LightStrengthAt (p));
  } while (LoopObj != LastEmitter);
  return scvTemp;
}

SENSCLASSVECTOR& SIM3DWorld::CalcCameraRay (const RAY& viewray,REAL& Depth)
// Bestimme den von einem Kamerastrahl gesehenen Wert. Wenn der Hintergrund
// gesehen wird, muss der Wert von 'Background' benutzt werden.
{
  REAL    where;
  SIM3DElement* obj;
  INTEGER facenr;

  if (CalcRayIntersection (viewray, TRUE, where, obj, facenr)) {
    Depth = abs(viewray.Direction * where);
    return obj->GetColorOfFaceAt (facenr, viewray.Start + viewray.Direction * where,
                                  viewray.Direction);
  }
  else
  {
    Depth = 1.0e6;
    return Background;
  }
}

void SIM3DWorld::CalcCameraView (const MOVEMATRIX& pos, REAL deltax, REAL deltay,
         const INTEGERARRAY& classes, INTEGER width, INTEGER height,
         REAL minDepth, REAL maxDepth, PSHORTREAL buffer)
// Berechnung eines Kamerabildes mit dem z-Buffer-Verfahren
{
  ZBUFFER zbuffer (width, height);
  zbuffer.SetViewGeometry (pos * VECTOR (0,0,0),
                           pos * VECTOR (-deltax/2,-deltay/2,1),
                           pos.Matrix * VECTOR (deltax/width,0,0),
                           pos.Matrix * VECTOR (0,deltay/height,0),   1.0e6);
  DrawBodyOnZBuffer (zbuffer);
  zbuffer.CalcCamera (Background, classes, buffer, minDepth, maxDepth);
}

REAL SIM3DWorld::CalcWhiskerRay (const RAY& whisker)
// Ein Tasthaar ist sozusagen ein Radarstrahl mit begrenzter Reichweite
{
  REAL    where;
  SIM3DElement* obj;
  INTEGER facenr;

  if (CalcRayIntersection (whisker, FALSE, where, obj, facenr))
    return 1.0 - where;
  else
    return 0.0;
}

SENSCLASSVECTOR& SIM3DWorld::CalcTactileRay (const RAY& tactile)
// Ein taktiler Sensor kann sozusagen Farben 'ertasten'.
{
  static SENSCLASSVECTOR scvEmpty(0,0.0F);
  REAL    where;
  SIM3DElement* obj;
  INTEGER facenr;

  if (CalcRayIntersection (tactile, FALSE, where, obj, facenr))
    return obj->GetColorOfFaceAt (facenr, tactile.Start +
                   where * tactile.Direction);
  else
    return scvEmpty;
}

SENSCLASSVECTOR& SIM3DWorld::CalcReceptorRay (const RAY& receptor)
// Und ein Rezeptor ist eigentlich nur ein Photowiederstand oder richtungs-
// loser Helligkeitssensor
{
  return LightAt (receptor.Start, receptor.Direction);
}

REAL SIM3DWorld::CalcUltraSonicRay(const RAY& ray)
// Bestimme die Entfernung in einer Richtung.
// Wenn der Hintergrund gesehen wird, wird der Wert 1e6 zurueckgegeben.
{
  REAL        where;
  SIM3DElement  *obj;
  INTEGER      facenr;

  if(CalcRayIntersection(ray,TRUE,where,obj,facenr))
    return abs(ray.Direction*where);
  else
    return 1.0e6;
}

REAL SIM3DWorld::CalcUltraSonic(const MOVEMATRIX &pos,REAL deltax,REAL deltay,
                                INTEGER width,INTEGER height)
// Bestimme den von einem Ultraschallstrahl gesehenen Wert.
// Wenn der Hintergrund gesehen wird, wird der Wert 1e6 zurueckgegeben.
{
  ZBUFFER zbuffer(width,height);
  zbuffer.SetViewGeometry(pos * VECTOR(0,0,0),
                          pos * VECTOR(-deltax/2,-deltay/2,1),
                          pos.Matrix * VECTOR(deltax/width,0,0),
                          pos.Matrix * VECTOR(0,deltay/height,0),1.0e6);
  DrawBodyOnZBuffer(zbuffer);
  return zbuffer.GetMinDepth();
}

