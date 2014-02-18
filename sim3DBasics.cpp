/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DBasics.cc                                        ****
 **** Inhalt: Implementation der Grundklassen der Simulations-      ****
 ****         hierarchie                                            ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DErrors.h"
#include "sim3DBasics.h"
#include "sim3DWorld.h"

static SENSCLASSVECTOR scvEmpty(0,0.0F);

// RaytrSortObj wird benutzt fuer SIM3DGroup::CalcRayIntersection
// (um die Rechnung wenn moeglich abzukuerzen) und dient dazu,
// eine Liste mit den Objekten aufzustellen, deren umgebendes Rechteck
// von dem Sehstrahl getroffen wird. Die Liste wird nach der unteren
// Grenze des Intervalls sortiert, so dass die Berechnung abgebrochen
// werden kann, wenn ein Schnittpunkt gefunden wurde, der noch vor
// der unteren Grenze des Intervalls des aktuellen Objekts liegt.

class RaytrSortObj 
{
public:
  RaytrSortObj* Next;
  INTERVALL Range;
  SIM3DElement* Object;

  RaytrSortObj (INTERVALL iv, SIM3DElement* obj)
  {
    Range  = iv;
    Object = obj;
    Next   = NULL;
  }

  ~RaytrSortObj ()
  {
    if (Next != NULL) delete Next;
  }
};

//--- SIM3DElement ---------------------------------------------------------
//
// Siehe auch "sim3DBasics.h"

SIM3DElement::SIM3DElement (const STRING& name) :
  ElementName (name)
{
  Next  = NOELEMENT;
  Owner = NOELEMENT;
  World = NOELEMENT;
  Registered = FALSE;
  Color = -1;
  DetailLevel = 0;
  CustomData = NULL;
}

SIM3DElement::SIM3DElement (const SIM3DElement& Obj) :
  ElementName (Obj.ElementName),
  Remark (Obj.Remark)
{
  Next        = NOELEMENT;
  Owner       = NOELEMENT;
  Registered  = FALSE;
  World       = Obj.World;
  DetailLevel = Obj.DetailLevel;
  Color       = Obj.Color;
  BoundingBox = Obj.BoundingBox;
  Transform   = Obj.Transform;
  CustomData  = NULL;
}

SIM3DElement::~SIM3DElement ()
{
  delete CustomData;
}

STRING SIM3DElement::GetNameRelTo (SIM3DElement* obj)
{
  if (obj == this)                      // Pfad relativ zu sich selbst
    return "";
  else if (Owner == obj)                // Pfad relativ zu Eigner ist trivial
    return GetName ();
  else if (Owner == NOELEMENT) {        // Wenn wir in der Wurzel sind
    sim3DOtherError (sim3DInvalidObject); // und da obj != NOELEMENT
    return "***ERROR***";               // liegt ein Fehler vor
  }
  else                                  // Ansonsten liefere zusammengesetzten
    return Owner->GetNameRelTo (obj) + STRING(".") + GetName ();        // Namen
}

MOVEMATRIX SIM3DElement::GetPosRelTo (SIM3DElement* obj)
{
  if (obj == this)                      // Nach dem gleichen Schema wie oben
    return MOVEMATRIX ();               
  else if (Owner == NOELEMENT) {        
    if (obj == NOELEMENT)
      return GetLocalTransform ();
    else {
      sim3DOtherError (sim3DInvalidObject);
      return MOVEMATRIX ();
    }
  }
  else
    return Owner->GetPosRelTo (obj) * GetLocalTransform ();
}

INTEGER SIM3DElement::SubObjectCount ()
{
  return 0;     // Defaultmaessig hat ein SIM3DElement keine Unterobjekte
}

SIM3DElement* SIM3DElement::SubObject (INTEGER)
{
  sim3DOtherError (sim3DOutOfRange);  // Deshalb gibt's hier immer einen Fehler
  return NOELEMENT;
}

SIM3DElement* SIM3DElement::FirstSubObject ()
{
  return NOELEMENT;     // Dito hier
}

SIM3DElement* SIM3DElement::NextSubObject (SIM3DElement*)
{
  return NOELEMENT;     // Dito hier
}

void SIM3DElement::ReadAttr (const STRING& attr, SCANNER& scan)
{
  REAL   r;

  // Fuer die Lesefunktionen siehe auch "sim3DFile.h"

       if (attr == "TURNZ") {
         if (GetCorrectReal (scan, r))
           Transform.Matrix = TurnXYMatrix (r) * Transform.Matrix;
       }
  else if (attr == "TURNY") {
         if (GetCorrectReal (scan, r))
           Transform.Matrix = TurnXZMatrix (r) * Transform.Matrix;
       }
  else if (attr == "TURNX") {
         if (GetCorrectReal (scan, r))
           Transform.Matrix = TurnYZMatrix (r) * Transform.Matrix;
       }
  else if (attr == "_MATRIX") {
         if (scan.TestFor (sLParen, sSemicolon)) {
           scan.NextSymbol ();
           MATRIX M;
           if (GetCorrectVector (scan, M.e1) &&
               scan.TestFor (sComma,sSemicolon)) {
             scan.NextSymbol ();
             if (GetCorrectVector (scan, M.e2) &&
                 scan.TestFor (sComma,sSemicolon)) {
               scan.NextSymbol ();
               if (GetCorrectVector (scan, M.e3) &&
                   scan.TestFor (sRParen,sSemicolon)) {
                 scan.NextSymbol ();
                 Transform.Matrix = M;
               }
             }
           }
         }
       }
  else if (attr == "MOVETO") {
         VECTOR v;
         if (GetCorrectVector (scan, v))
           Transform.Offset = Transform.Offset + v;
       }
  else if (attr == "SCALE") {
         REAL r;
         if (GetCorrectReal (scan,r))
           Transform.Matrix = Transform.Matrix * 
                              MATRIX(VECTOR (r,0,0),
                                     VECTOR (0,r,0),
                                     VECTOR (0,0,r));
       }
  else if (attr == "REMARK")
         (void) GetCorrectString (scan, Remark);
  else if (attr == "COLOR")
         Color = World->ColorList.ReadColor (scan);
  else if ((attr == "DETAILLEVEL") || (attr == "DL"))
         (void) GetCorrectInteger (scan, DetailLevel);
  else {
    scan.SyntaxError(sim3DUnknownAttribute1, attr, sSemicolon);
    return;
  }
}

void SIM3DElement::WriteAttrs (BACKUP& backup)
{
  if(DetailLevel)
    backup.WriteIdIntNext ("DL", DetailLevel);

  if (! (Remark == ""))
    backup.WriteIdStrNext ("REMARK", Remark);

  if (Color >= 0) {
    World->ColorList.WriteColor (backup, "COLOR", Color);
  }

  REAL a,b,c,
       s = abs(Transform.Matrix.e1);
  
  // ExtractAngles ist definiert in "Vector.h" und versucht, die Lagewinkel aus
  // der MOVEMATRIX Transform zu berechnen
  if (ExtractAngles (Transform.Matrix,a,b,c)) {
    if (abs (a) > EPSILON)
      backup.WriteIdRealNext ("TURNX", a);
    if (abs (b) > EPSILON)
      backup.WriteIdRealNext ("TURNY", b);
    if (abs (c) > EPSILON)
      backup.WriteIdRealNext ("TURNZ", c);
    if (abs(s - 1) > EPSILON)
      backup.WriteIdRealNext ("SCALE", s);
  }
  // Funktioniert das nicht, so speichere die Matrix ab
  else {
    backup.WriteIdent ("_MATRIX");
    backup.WriteSymbol (sLParen);
    backup.WriteVector (Transform.Matrix.e1);
    backup.WriteSymbol (sComma);
    backup.WriteVector (Transform.Matrix.e2);
    backup.WriteSymbol (sComma);
    backup.WriteVector (Transform.Matrix.e3);
    backup.WriteSymbol (sRParen);
    backup.ListNext ();
  }

  if (abs (Transform.Offset) > EPSILON) {
    backup.WriteIdent ("MOVETO");
    backup.WriteVector (Transform.Offset);
    backup.ListNext ();
  }
}

void SIM3DElement::WriteObj (BACKUP& backup)
{
  backup.WriteIdent (GetClass ());
  backup.WriteString (GetName ());
  backup.StartListI (sLParen, sSemicolon, sRParen, TRUE);
  WriteAttrs (backup);
  backup.EndList ();
}

// Alle zu den Ports gehoerigen Informationen werden aus den oder ueber die 
// APORTDEF- oder SPORTDEF-Strukturen gelesen

ACTORPORT SIM3DElement::GetActorPort (const STRING& s)
{
  APORTDEF* apdef = GetAPortDef ();
  INTEGER portnr = 0;
  while (apdef != NULL) {
    if (s == apdef->Name) return PORT (this, portnr);
    apdef = apdef->Prev;
    portnr++;
  }
  sim3DOtherError (sim3DPortNotFound);
  return PORT (NOELEMENT, 0);
}

SENSORPORT SIM3DElement::GetSensorPort (const STRING& s)
{
  SPORTDEF* spdef = GetSPortDef ();
  INTEGER portnr = 0;
  while (spdef != NULL) {
    if (s == spdef->Name) return PORT (this, portnr);
    spdef = spdef->Prev;
    portnr++;
  }
  sim3DOtherError (sim3DPortNotFound);
  return PORT (NOELEMENT, 0);
}

void SIM3DElement::SetActorValue (INTEGER portnr, REAL value)
{
  APORTDEF* apdef = GetAPortDef ();
  while ((apdef != NULL) && (portnr > 0)) {
    apdef = apdef->Prev;
    portnr--;
  }
  if (apdef == NULL)
    sim3DOtherError (sim3DBadActorPort);
  else
    apdef->SetValue (this, value);
}

void SIM3DElement::GetSensorValue (INTEGER portnr, PSHORTREAL value)
{
  SPORTDEF* spdef = GetSPortDef ();
  while ((spdef != NULL) && (portnr > 0)) {
    spdef = spdef->Prev;
    portnr--;
  }
  if (spdef == NULL)
    sim3DOtherError (sim3DBadSensorPort);
  else
    spdef->ReadValue (this, value);
}

INTEGER SIM3DElement::GetSensorDim (INTEGER portnr)
{
  SPORTDEF* spdef = GetSPortDef ();
  while ((spdef != NULL) && (portnr > 0)) {
    spdef = spdef->Prev;
    portnr--;
  }
  if (spdef == NULL) {
    sim3DOtherError (sim3DBadSensorPort);
    return 0;
  }
  else
    return spdef->Dimension;
}

INTEGER SIM3DElement::GetSensorDimSize (INTEGER portnr, INTEGER dim)
{
  SPORTDEF* spdef = GetSPortDef ();
  while ((spdef != NULL) && (portnr > 0)) {
    spdef = spdef->Prev;
    portnr--;
  }
  if (spdef == NULL) {
    sim3DOtherError (sim3DBadSensorPort);
    return 0;
  }
  else if ((dim >= 0) && (dim < spdef->Dimension))
    return spdef->GetDimSize (this, dim);
  else
    return 1;
}

INTEGER SIM3DElement::ActorPortCount ()
{
  APORTDEF* apdef = GetAPortDef ();
  INTEGER portcount = 0;
  while (apdef != NULL) {
    apdef = apdef->Prev;
    portcount++;
  }
  return portcount;
}

INTEGER SIM3DElement::SensorPortCount ()
{
  SPORTDEF* spdef = GetSPortDef ();
  INTEGER portcount = 0;
  while (spdef != NULL) {
    spdef = spdef->Prev;
    portcount++;
  }
  return portcount;
}

STRING SIM3DElement::ActorPortName (INTEGER portnr)
{
  APORTDEF* apdef = GetAPortDef ();
  while ((apdef != NULL) && (portnr > 0)) {
    apdef = apdef->Prev;
    portnr--;
  }
  if (apdef == NULL) {
    sim3DOtherError (sim3DBadActorPort);
    return STRING ();
  }
  else
    return apdef->Name;
}

STRING SIM3DElement::SensorPortName (INTEGER portnr)
{
  SPORTDEF* spdef = GetSPortDef ();
  while ((spdef != NULL) && (portnr > 0)) {
    spdef = spdef->Prev;
    portnr--;
  }
  if (spdef == NULL) {
    sim3DOtherError (sim3DBadSensorPort);
    return STRING ();
  }
  else
    return spdef->Name;
}

void SIM3DElement::RegisterAdd ()
{
  Registered = TRUE;
}

void SIM3DElement::RegisterRemove ()
{
  Registered = FALSE;
}

COLOR SIM3DElement::GetColor ()
// Wenn fuer das Objekt keine Farbe definiert ist, wird die Standardfarbe aus
// der World geholt.
{
  if (Color < 0)        
    return World->StandardColor;
  else
    return Color;
}

void SIM3DElement::DoDraw (GRAPHICPORT* gp, INTEGER detail, REAL scale)
{
  MOVEMATRIX M;
  if (World == this)
    M = GetLocalTransform(); 
  gp->NewPicture ();
  Draw (gp,M,detail,scale);
  gp->EndOfPicture ();
}

void SIM3DElement::Draw2 (GRAPHICPORT* gp, const MOVEMATRIX& M, INTEGER detail,
                          REAL scale)
{
  if (detail >= DetailLevel)
    Draw (gp, M * GetLocalTransform (), detail, scale);
}

BOOLEAN SIM3DElement::CalcRayIntersection
       (const RAY&, BOOLEAN, REAL&, SIM3DElement*&, INTEGER&)
{
  return FALSE;         // Defaultmaessig keine Ueberschneidung
}

SENSCLASSVECTOR& SIM3DElement::GetColorOfFaceAt (INTEGER, const VECTOR&)
{
  sim3DFatalError (sim3DInternalError);
  return scvEmpty;
}

SENSCLASSVECTOR& SIM3DElement::GetColorOfFaceAt (INTEGER, REAL, REAL)
{
  sim3DFatalError (sim3DInternalError);
  return scvEmpty;
}

SENSCLASSVECTOR& SIM3DElement::GetColorOfFaceAt (INTEGER, const VECTOR&,
                                                 const VECTOR&)
{
  sim3DFatalError (sim3DInternalError);
  return scvEmpty;
}

SENSCLASSVECTOR& SIM3DElement::GetColorOfFaceAt (INTEGER, const VECTOR&,
                                                 const VECTOR&, REAL, REAL)
{
  sim3DFatalError (sim3DInternalError);
  return scvEmpty;
}

void SIM3DElement::DrawBodyOnZBuffer (ZBUFFER&)
{
}

MOVEMATRIX SIM3DElement::GetLocalTransform ()
{
  return Transform;
}

//--- SIM3DGroup -----------------------------------------------------------


SIM3DGroup::SIM3DGroup (const STRING& Name) : SIM3DElement (Name)
{
  Last = NOELEMENT;
}

SIM3DGroup::SIM3DGroup (const SIM3DGroup& Obj) : SIM3DElement (Obj)
{
  SIM3DElement* CopyObj;

  Last = NOELEMENT;                     // Kopieren der Unterobjekte
  CopyObj = Obj.Last;
  if (CopyObj != NOELEMENT) {
    do {
      CopyObj = CopyObj->Next;
      AddObject (CopyObj->Copy ());
    } while (CopyObj != Obj.Last);
  }
  Paths = Obj.Paths;
}

SIM3DGroup::~SIM3DGroup ()
{
  SIM3DElement* p;

  if (Last != NOELEMENT) {              // Loeschen der Unterobjekte
    while (Last != Last->Next) {
      p = Last->Next->Next;
      delete Last->Next;
      Last->Next = p;
    }
    delete Last;
    Last = NOELEMENT;
  }
}

void SIM3DGroup::AddObject (SIM3DElement* Obj)
{
  if (Obj == NOELEMENT)
    sim3DOtherError (sim3DInvalidObject);
  else {
    if (Last == NOELEMENT)              // Einfuegen in Ringliste
      Obj->Next  = Obj;
    else {
      Obj->Next  = Last->Next;
      Last->Next = Obj;
    }
    Obj->Owner = this;
    Last = Obj;
    Obj->World = World;
    if (Registered)   Obj->RegisterAdd ();
    BoundingBox.AddBox (Obj->BoundingBox);
  }
}

BOOLEAN SIM3DGroup::RemoveObject (SIM3DElement* Obj)
{
  if(Last)
  {
    SIM3DElement* LoopObj = Last;
    do
    {
      if(LoopObj->Next == Obj)            // Objekt gefunden?
      {
        if (Obj->Next == Obj)             // Loeschen aus Ringliste
          Last = NOELEMENT;
        else {
          LoopObj->Next = Obj->Next;
          if (Last == Obj)
            Last = LoopObj;
        }
        if (Registered) Obj->RegisterRemove ();
        Obj->Next  = NOELEMENT;
        Obj->Owner = NOELEMENT;
        return TRUE;
      }
      else if(LoopObj->Next->IsA(S3DGroup) && // Objekt in Untergruppe?
        ((SIM3DGroup*) LoopObj->Next)->RemoveObject(Obj))
        return TRUE;
      LoopObj = LoopObj->Next;
    }
    while(LoopObj != Last);
  }
  return FALSE;
}

void SIM3DGroup::RegisterAdd ()
// RegisterAdd fuer SIM3DGroup fuehrt ein RegisterAdd fuer alle Unterobjekte
// aus und berechnet das umgebende Rechteck neu.
// Das erfordert, dass die umgebenden Rechtecke der Unterobjekte korrekt
// berechnet wurden. Ausserdem muessen alle Lagedaten in den Unterobjekten
// korrekt behandelt werden.
{
  SIM3DElement::RegisterAdd ();
  SIM3DElement* LoopObj = Last;

  BoundingBox.Clear ();
  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->RegisterAdd ();
      BoundingBox.AddBox (LoopObj->BoundingBox);
    } while (LoopObj != Last);
  }
}

void SIM3DGroup::RegisterRemove ()
// RegisterRemove fuehrt RegisterRemove fuer alle Unterobjekte durch
{
  SIM3DElement::RegisterRemove ();
  SIM3DElement* LoopObj = Last;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->RegisterRemove ();
    } while (LoopObj != Last);
  }
}

INTEGER SIM3DGroup::SubObjectCount ()
{
  SIM3DElement* LoopObj = Last;
  INTEGER       Count   = 0;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      Count++;
    } while (LoopObj != Last);
  }
  return Count;
}

SIM3DElement* SIM3DGroup::SubObject (INTEGER Count)
{
  SIM3DElement* LoopObj = Last;

  if (Count < 0) {
    sim3DOtherError (sim3DOutOfRange);
    return NOELEMENT;
  }

  Count++;
  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      Count--;
    } while ((LoopObj != Last) && (Count > 0));
  }
  if (Count > 0) {
    sim3DOtherError (sim3DOutOfRange);
    return NOELEMENT;
  }
  else
    return LoopObj;
}

SIM3DElement* SIM3DGroup::FirstSubObject ()
{
  if (Last == NOELEMENT)
    return NOELEMENT;
  else
    return Last->Next;
}

SIM3DElement* SIM3DGroup::NextSubObject (SIM3DElement* Obj)
{
  if (Obj == Last)      // Es existiert keine Abfrage, ob das Obj ueberhaupt
    return NOELEMENT;   // Unterobjekt von 'this' ist. Vorsicht!
  else
    return Obj->Next;
}

void SIM3DGroup::AddPath (const STRING& a, const STRING& b)
{
  Paths.AddElement (a + STRING ("=") + b);
}

STRING SIM3DGroup::ExpandPath (const STRING& p)
{
  INTEGER equalpos;

  INTEGER dotpos = p.Pos ('.');         // Position des ersten Punkt im Pfad
  STRING  FirstSeg = p.First (dotpos);  // Erstes Segment des Pfads (oder
                                        // ganzer Pfad)
  INTEGER pathCount = Paths.HowMany (); // Wieviele Aliase sind gesetzt
  INTEGER i=0;

  while (i<pathCount) {                 // Aliase durchgehen, bis Match gefund.
    equalpos = Paths [i].Pos ('=');     
    if ((equalpos < Paths [i].Length ()) &&
        (Paths [i].First (equalpos) == FirstSeg)) break;
    i++;
  }

  if (i < pathCount)
    return Paths [i].ButFirst (equalpos+1) + p.ButFirst (dotpos);
  else
    return p;
}

void SIM3DGroup::ReadAttr (const STRING& attr, SCANNER& scan)
{
  STRING  s1 = "", s2 = "";

  if (attr == "PATH") {
    if (GetCorrectString (scan, s1)) {
      scan.Expect (sEqual);
      if (GetCorrectString (scan, s2))
        AddPath (s1,s2);
    }
  }
  else
    SIM3DElement::ReadAttr (attr, scan);
}

void SIM3DGroup::WriteAttrs (BACKUP& backup)
{
  SIM3DElement::WriteAttrs (backup);
  INTEGER Count = Paths.HowMany ();
  for (INTEGER i=0;i<Count;i++) {
    STRING p = Paths [i];
    INTEGER equalpos = p.Pos ('=');
    backup.WriteIdent ("PATH");
    backup.WriteString (p.First (equalpos));
    backup.WriteSymbol (sEqual);
    backup.WriteString (p.ButFirst (equalpos+1));
    backup.ListNext ();
  }
}

void SIM3DGroup::WriteObj (BACKUP& backup)
{
  SIM3DElement::WriteObj (backup);
  backup.StartListI (sLBracket,sSemicolon,sRBracket,FALSE);
  WriteSubObjects (backup);
  backup.EndList ();
}

void SIM3DGroup::WriteSubObjects (BACKUP& backup)
{
  SIM3DElement* LoopObj = Last;
  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->WriteObj (backup);
      backup.ListNext ();
    } while (LoopObj != Last);
  }
}

ACTORPORT SIM3DGroup::GetActorPort (const STRING& s)
{
  SIM3DElement* LoopObj = Last;
  STRING RealName = ExpandPath (s);
  INTEGER dotpos = RealName.Pos ('.');
  if (dotpos >= RealName.Length ())
    return SIM3DElement::GetActorPort (RealName);
  else {
    if (LoopObj != NOELEMENT) {
      STRING FirstSeg = RealName.First (dotpos);
      do {
        LoopObj = LoopObj->Next;
        if (FirstSeg == LoopObj->GetName ())
          return LoopObj->GetActorPort (RealName.ButFirst (dotpos+1));
      } while (LoopObj != Last);
    }
    sim3DOtherError (sim3DPortNotFound);
    return PORT (NOELEMENT, 0);
  }
}

SENSORPORT SIM3DGroup::GetSensorPort (const STRING& s)
{
  SIM3DElement* LoopObj = Last;
  STRING RealName = ExpandPath (s);
  INTEGER dotpos = RealName.Pos ('.');
  if (dotpos >= RealName.Length ())
    return SIM3DElement::GetSensorPort (RealName);
  else {
    if (LoopObj != NOELEMENT) {
      STRING  FirstSeg = RealName.First (dotpos);
      do {
        LoopObj = LoopObj->Next;
        if (FirstSeg == LoopObj->GetName ())
          return LoopObj->GetSensorPort (RealName.ButFirst (dotpos+1));
      } while (LoopObj != Last);
    }
    sim3DOtherError (sim3DPortNotFound);
    return PORT (NOELEMENT, 0);
  }
}

void SIM3DGroup::MakeTimeStep (const MOVEMATRIX& M)
{
  SIM3DElement* LoopObj = Last;

  MOVEMATRIX M2 = M * GetLocalTransform ();

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->MakeTimeStep (M2);
    } while (LoopObj != Last);
  }
}

void SIM3DGroup::MakeStep (const MOVEMATRIX& M)
{
  SIM3DElement* LoopObj = Last;

  MOVEMATRIX M2 = M * GetLocalTransform ();

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->MakeStep (M2);
    } while (LoopObj != Last);
  }
}

void SIM3DGroup::StepBack ()
{
  SIM3DElement* LoopObj = Last;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->StepBack ();
    } while (LoopObj != Last);
  }
}

void SIM3DGroup::ComputeSensors ()
{
  SIM3DElement* LoopObj = Last;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->ComputeSensors ();
    } while (LoopObj != Last);
  }
}

BOOLEAN SIM3DGroup::DoesIntersect (SIM3DElement* Obj)
{
  SIM3DElement* LoopObj = Last;

// Ein Objekt schneidet sich nicht mit sich selbst:
  if (Obj == this) return FALSE;
// Und ein Schnitt kann nur vorliegen, wenn die umgebenden Rechtecke sich
// ueberschneiden:
  if (!BoundingBox.DoesIntersect (Obj->BoundingBox)) return FALSE;

// Aber dann muss der Test mit allen Unterobjekten durchgefuehrt werden.
  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      if (LoopObj->DoesIntersect (Obj)) return TRUE;
    } while (LoopObj != Last);
  }
  return FALSE;
}

void SIM3DGroup::MakeNewBoundingBox ()
{
  SIM3DElement* LoopObj = Last;

  BoundingBox.Clear ();

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->MakeNewBoundingBox ();
      BoundingBox.AddBox (LoopObj->BoundingBox);
    } while (LoopObj != Last);
  }
}

void SIM3DGroup::RegisterBoundingBox ()
{
  SIM3DElement* LoopObj = Owner->Last;

  Owner->BoundingBox.Clear ();

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      Owner->BoundingBox.AddBox (LoopObj->BoundingBox);
    } while (LoopObj != Owner->Last);
  }

  Owner->RegisterBoundingBox ();
}

void SIM3DGroup::Draw (GRAPHICPORT* p, const MOVEMATRIX& M, INTEGER detail,
                       REAL scale)
{
  SIM3DElement* LoopObj = Last;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->Draw2 (p, M, detail, scale);
    } while (LoopObj != Last);
  }
}

RaytrSortObj* MergeRSO (RaytrSortObj* list, RaytrSortObj* elem)
// Trage ein neues Objekt in die (sortierte) RaytrSortObj-Liste ein
{
  if (list == NULL)
    return elem;
  else if (elem->Range.Min < list->Range.Min) {
    elem->Next = list;
    return elem;
  }
  else {
    list->Next = MergeRSO (list->Next, elem);
    return list;
  }
}

BOOLEAN SIM3DGroup::CalcRayIntersection
       (const RAY& ray, BOOLEAN infinite,
        REAL& where, SIM3DElement*& obj, INTEGER& facenr)
{
  RaytrSortObj* RSO = NULL;
  SIM3DElement* LoopObj = Last;
  INTERVALL iv;

// Bestimmte die Liste aller Objekte, deren umgebendes Rechteck vom Strahl ray
// getroffen wird.
  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      iv = Intersection (ray, LoopObj->BoundingBox);
      if ((! iv.Empty) && (iv.Max > EPSILON) && (infinite || (iv.Min < 1)))
        RSO = MergeRSO (RSO, new RaytrSortObj (iv, LoopObj));
    } while (LoopObj != Last);
  }

  RaytrSortObj* RSOLoop = RSO;
  REAL          where2;
  SIM3DElement* obj2;
  INTEGER       facenr2;

  obj = NOELEMENT;

// Bestimme den ersten Schnitt des Strahls mit einem Objekt
  while ((RSOLoop != NULL) &&
         ((obj == NOELEMENT) || (where > RSOLoop->Range.Min))) {
    if (RSOLoop->Object->CalcRayIntersection (ray, infinite, 
        where2, obj2, facenr2)  &&  ((obj == NOELEMENT) || (where > where2))) {
      where    = where2;
      obj      = obj2;
      facenr   = facenr2;
    }
    RSOLoop = RSOLoop->Next;
  }

  delete RSO;
  return (obj != NOELEMENT);
}

void SIM3DGroup::DrawBodyOnZBuffer (ZBUFFER& zb)
{
  SIM3DElement* LoopObj = Last;

  if (LoopObj != NOELEMENT) {
    do {
      LoopObj = LoopObj->Next;
      LoopObj->DrawBodyOnZBuffer (zb);
    } while (LoopObj != Last);
  }
}

//--- SIM3DGlobal --------------------------------------------------------

void SIM3DGlobal::MakeTimeStep (const MOVEMATRIX& M)
{
  GlobalPos = M * Transform;
}

void SIM3DGlobal::RegisterAdd ()
{
  SIM3DElement::RegisterAdd ();
  GlobalPos = GetPosRelTo (NOELEMENT);
}

