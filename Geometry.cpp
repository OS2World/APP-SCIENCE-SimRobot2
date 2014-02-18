/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  Geometry.cc                                           ****
 **** Inhalt: Komplexe Vektor-Operation (umgebende Rechtecke,       ****
 ****         Intervalle, Schnitte zwischen geometrischen Objekten) ****
 ****                                                               ****
 ***********************************************************************/

#include "Geometry.h"

// Fuer Kommentare siehe auch "Geometry.h"

inline REAL min (REAL a, REAL b)
{ return (a < b ? a : b); }

inline REAL max (REAL a, REAL b)
{ return (a > b ? a : b); }


//--- RAY -------------------------------------------------------------

RAY::RAY (const VECTOR& v1, const VECTOR& v2)
{
  Start = v1;
  Direction = v2 - v1;
}

BOOLEAN RAY::DoesIntersect (const BOUNDINGBOX& b)
{
  INTERVALL iv = Intersection (*this, b);
  return ! iv.Empty;
}

INTERVALL Intersection (const RAY& r,const BOUNDINGBOX& b)
{
  if (b.Empty) return INTERVALL ();
  INTERVALL iv (-1e20,1e20);

  if (abs (r.Direction.x) > EPSILON) {
    INTERVALL ivx ((b.Min.x - r.Start.x) / r.Direction.x,
                   (b.Max.x - r.Start.x) / r.Direction.x);
    iv = ivx;
  }
  else if ((b.Min.x > r.Start.x) || (b.Max.x < r.Start.x))
    return INTERVALL ();

  if (abs (r.Direction.y) > EPSILON) {
    INTERVALL ivy ((b.Min.y - r.Start.y) / r.Direction.y,
                   (b.Max.y - r.Start.y) / r.Direction.y);
    iv.Intersect (ivy);
  }
  else if ((b.Min.y > r.Start.y) || (b.Max.y < r.Start.y))
    return INTERVALL ();

  if (abs (r.Direction.z) > EPSILON) {
    INTERVALL ivz ((b.Min.z - r.Start.z) / r.Direction.z,
                   (b.Max.z - r.Start.z) / r.Direction.z);
    iv.Intersect (ivz);
  }
  else if ((b.Min.z > r.Start.z) || (b.Max.z < r.Start.z))
    return INTERVALL ();

  return iv;
}

//--- INTERVALL -------------------------------------------------------

INTERVALL::INTERVALL ()
{
  Empty = TRUE;
}

INTERVALL::INTERVALL (REAL a, REAL b)
{
  Empty = FALSE;
  if (a < b) {
    Min = a;
    Max = b;
  }
  else {
    Min = b;
    Max = a;
  }
}

void INTERVALL::Intersect (const INTERVALL& iv)
{
  if (iv.Empty)
    Empty = TRUE;
  else if (! Empty) {
    if ((iv.Min > Max) || (iv.Max < Min))
      Empty = TRUE;
    else {
      Min = min (Min, iv.Min);
      Max = max (Max, iv.Max);
    }
  }
}

//--- BOUNDINGBOX -----------------------------------------------------

BOUNDINGBOX::BOUNDINGBOX ()
{
  Empty = TRUE;
}

void BOUNDINGBOX::Clear ()
{
  Empty = TRUE;
}

void BOUNDINGBOX::AddPoint (const VECTOR& v)
{
  if (Empty) {
    Empty = FALSE;
    Min = v;
    Max = v;
  }
  else {
    Min = VECTOR (min (Min.x, v.x), min (Min.y, v.y), min (Min.z, v.z));
    Max = VECTOR (max (Max.x, v.x), max (Max.y, v.y), max (Max.z, v.z));
  }
}

void BOUNDINGBOX::AddBox (const BOUNDINGBOX& b)
{
  AddPoint (b.Min);
  AddPoint (b.Max);
}

void BOUNDINGBOX::Intersect (const BOUNDINGBOX& b)
{
  if (b.Empty)
    Empty = TRUE;
  else if (! Empty) {
    INTERVALL ivx (Min.x, Max.x);
    ivx.Intersect (INTERVALL (b.Min.x, b.Max.x));
    INTERVALL ivy (Min.y, Max.y);
    ivy.Intersect (INTERVALL (b.Min.y, b.Max.y));
    INTERVALL ivz (Min.z, Max.z);
    ivz.Intersect (INTERVALL (b.Min.z, b.Max.z));
    if (ivx.Empty || ivy.Empty || ivz.Empty)
      Empty = TRUE;
    else {
      Min = VECTOR (ivx.Min, ivy.Min, ivz.Min);
      Max = VECTOR (ivx.Max, ivy.Max, ivz.Max);
    }
  }
}

BOOLEAN BOUNDINGBOX::DoesIntersect (const BOUNDINGBOX& b)
{
  BOUNDINGBOX thisbox = *this;
  thisbox.Intersect (b);
  return !thisbox.Empty;
}
