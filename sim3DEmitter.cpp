/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3dEmitter.cc                                       ****
 **** Inhalt: Implementation der Klassen SIM3DEmitter und Unterarten ***
 ****                                                               ****
 ***********************************************************************/

#include "sim3DEmitter.h"
#include "sim3DErrors.h"
#include "sim3DWorld.h"
#include "math.h"

inline REAL max (REAL a, REAL b)
{
  return (a > b ? a : b);
}

//-- Portdefinitionen -----------------------------------------------------
//
// Siehe auch "sim3DPortDef.h"


void SetEmitterActor  (SIM3DElement* obj, REAL value)
{
  if (value < 0) value = 0;
  else if (value > 1) value = 1;

  ((SIM3DEmitter*) obj)->Intensity = value;
}

APORTDEF S3DEmitterAPort (NULL, "EMIT", SetEmitterActor);

//-- SIM3DEmitter ---------------------------------------------------------
//
// Siehe auch "sim3DEmitter.h" und "sim3DBasics.h"

SIM3DEmitter::SIM3DEmitter (const STRING& Name) : SIM3DGlobal (Name)
{
  Radiation   = NULL;
  NextEmitter = NULL;
  Factor      = 1.0;
  Intensity   = 1.0;
}

SIM3DEmitter::SIM3DEmitter (const SIM3DEmitter& Obj) : SIM3DGlobal (Obj)
{
  if (Obj.Radiation != NULL)
    Radiation = new SENSCLASSVECTOR (*Obj.Radiation);
  NextEmitter = NULL;
  Factor      = Obj.Factor;
  Intensity   = Obj.Intensity;
}

SIM3DEmitter::~SIM3DEmitter ()
{
  if (Radiation != NULL) delete Radiation;
}

void SIM3DEmitter::ReadAttr (const STRING& attr, SCANNER& scan)
{
  REAL r;

  if (attr == "FACTOR") {
    if (GetCorrectReal (scan, r))
      Factor = r;
    else
      scan.SyntaxError (sim3DExpectPositive);
  }
  else if (attr == "RADIATION") {
    Radiation = new SENSCLASSVECTOR (0, 0.0F);
    Radiation->ReadVector (scan);
  }
  else if (attr == "START")
    (void) GetCorrectReal (scan, Intensity);
  else
    SIM3DGlobal::ReadAttr (attr, scan);
}

void SIM3DEmitter::WriteAttrs (BACKUP& backup)
{
  SIM3DGlobal::WriteAttrs (backup);
  backup.WriteIdRealNext ("FACTOR", Factor);
  backup.WriteIdRealNext ("START", Intensity);
  if (Radiation != NULL) {
    backup.WriteIdent ("RADIATION");
    Radiation->WriteVector (backup);
    backup.ListNext ();
  }
}

APORTDEF* SIM3DEmitter::GetAPortDef ()
{
  return &S3DEmitterAPort;
}

void SIM3DEmitter::RegisterAdd ()
{
  SIM3DGlobal::RegisterAdd ();
  World->AddEmitter (this);
}

void SIM3DEmitter::RegisterRemove ()
{
  World->RemoveEmitter (this);
  SIM3DGlobal::RegisterRemove ();
}

SENSCLASSVECTOR* SIM3DEmitter::GetRadiation ()
{
  if (Radiation != NULL)
    return Radiation;
  else
    return &(World->StandardRadiation);
}

VECTOR SIM3DEmitter::LightDirectionAt (const VECTOR& p)
{
  VECTOR dir = p - GlobalPos.Offset;
  dir.Norm ();
  return dir;
}

COLOR SIM3DEmitter::GetColor ()
{
  if (Color < 0)
    return World->EmitterColor;
  else
    return Color;
}

//-- SIM3DRadialEmitter ----------------------------------------------------

REAL SIM3DRadialEmitter::LightStrengthAt (const VECTOR& p)
{
  VECTOR diff = p - GlobalPos.Offset;
  REAL dist_2 = diff*diff;              // Bilde das Quadrat der Entfernung
  if (dist_2 < 1.0) dist_2 = 1.0;       // Mindestabstand ist 1
  return Intensity * Factor / dist_2;   // Faktor, Intensitaet und Abstand
                                        // bestimmen die Lichtstaerke
}

void SIM3DRadialEmitter::Draw (GRAPHICPORT* gp, const MOVEMATRIX& M,
                               INTEGER, REAL scale)
// Ein Radialemitter wird als 3D-Kreuz gezeichnet
{
  scale = scale * max (Intensity, 0.1);

  COLOR col = GetColor ();
  VECTOR v1 = M * VECTOR (scale,0,0);
  VECTOR v2 = M * VECTOR (-scale,0,0);
  VECTOR v3 = M * VECTOR (0,scale,0);
  VECTOR v4 = M * VECTOR (0,-scale,0);
  VECTOR v5 = M * VECTOR (0,0,scale);
  VECTOR v6 = M * VECTOR (0,0,-scale);
  gp->DrawLine (v1,v2,col);
  gp->DrawLine (v3,v4,col);
  gp->DrawLine (v5,v6,col);
}

//-- SIM3DSpotEmitter ------------------------------------------------------


SIM3DSpotEmitter::SIM3DSpotEmitter (const STRING& Name) : SIM3DEmitter (Name)
{
  Angle = 180;
  Exponent = 1.0;
}

void SIM3DSpotEmitter::ReadAttr (const STRING& attr, SCANNER& scan)
{
  if (attr == "ANGLE")
    (void) GetCorrectReal (scan, Angle);
  else if (attr == "EXPONENT")
    (void) GetCorrectReal (scan, Exponent);
  else
    SIM3DEmitter::ReadAttr (attr, scan);
}

void SIM3DSpotEmitter::WriteAttrs (BACKUP& backup)
{
  SIM3DEmitter::WriteAttrs (backup);
  backup.WriteIdRealNext ("ANGLE", Angle);
  backup.WriteIdRealNext ("EXPONENT", Exponent);
}

REAL SIM3DSpotEmitter::LightStrengthAt (const VECTOR& p)
{
  VECTOR diff = p - GlobalPos.Offset;
  REAL dist_2 = diff * diff;
// Bestimme den Winkel des Lichtvektors zur dritten (z-) Achse: (in Radiant)
  REAL angleToMain = acos (GlobalPos.Matrix.e3 * LightDirectionAt (p));
// Bestimme Maximalwinkel (in Radiant)
  REAL angle = Angle * PI / 180;
// Wenn Winkel > Maximalwinkel, dann kein Licht sichtbar
  if (angle < angleToMain) return 0.0;
// Strecke die Winkeldifferenz auf PI/2
  angleToMain = angleToMain / angle * (PI/2);
// Mindestabstand ist 1
  if (dist_2 < 1.0) dist_2 = 1.0;
// Intensitaet, Faktor, Abstand, Winkel, Maximalwinkel und Streuungsgrad
// bestimmen die Lichtstaerke eines Spotemitters:
  return Intensity * Factor / dist_2 * pow (cos (angleToMain), Exponent);
}

void SIM3DSpotEmitter::Draw (GRAPHICPORT* gp, const MOVEMATRIX& M, INTEGER,
                             REAL scale)
// Ein Spotemitter wird als 3D-Kreuz gezeichnet, dass in der z-Achse nur in 
// eine Richtung gezeichnet.
{
  scale = scale * max (Intensity, 0.1);

  COLOR col = GetColor ();
  VECTOR v1 = M * VECTOR (scale,0,0);
  VECTOR v2 = M * VECTOR (-scale,0,0);
  VECTOR v3 = M * VECTOR (0,scale,0);
  VECTOR v4 = M * VECTOR (0,-scale,0);
  VECTOR v5 = M * VECTOR (0,0,2*scale);
  VECTOR v6 = M * VECTOR (0,0,0);
  gp->DrawLine (v1,v2,col);
  gp->DrawLine (v3,v4,col);
  gp->DrawLine (v5,v6,col);
}
