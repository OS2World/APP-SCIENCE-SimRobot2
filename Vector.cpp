/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  Vector.cc                                             ****
 **** Inhalt: Implementation der Klassen VECTOR, MATRIX, MOVEMATRIX ****
 ****                                                               ****
 ***********************************************************************/

#include "Vector.h"
#include <math.h>

// Fuer Kommentare siehe "Vektor.h"

inline REAL sqr     (REAL x) { return x*x;          }
inline REAL MakeDeg (REAL x) { return x / PI * 180; }
inline REAL MakeRad (REAL x) { return x / 180 * PI; }

INTEGER sign (REAL x)
{
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}

VECTOR::VECTOR ()
{
  x = 0.0;
  y = 0.0;
  z = 0.0;
}

VECTOR::VECTOR (REAL xx, REAL yy, REAL zz)
{
  x = xx;
  y = yy;
  z = zz;
}

BOOLEAN VECTOR::Norm ()
{
  REAL len = abs (*this);

  if (len > EPSILON) {
    x = x / len;
    y = y / len;
    z = z / len;
    return TRUE;
  }
  else
    return FALSE;
}

BOOLEAN VECTOR::operator == (const VECTOR& v)
{
  return x == v.x && y == v.y && z == v.z;
}


MATRIX::MATRIX () :
  e1 (1.0, 0.0, 0.0),
  e2 (0.0, 1.0, 0.0),
  e3 (0.0, 0.0, 1.0)
{}

MATRIX::MATRIX (const VECTOR& v1, const VECTOR& v2, const VECTOR& v3)
{
  e1 = v1;
  e2 = v2;
  e3 = v3;
}

MOVEMATRIX::MOVEMATRIX () {}

MOVEMATRIX::MOVEMATRIX (const MATRIX& m, const VECTOR& v)
{
  Matrix = m;
  Offset = v;
}

REAL abs (const VECTOR& V)
{ return sqrt (V * V); }

REAL abs (REAL r)
{ return (r >= 0 ? r : -r); }

VECTOR operator + (const VECTOR& V1, const VECTOR& V2)
{ return VECTOR (V1.x + V2.x, V1.y + V2.y, V1.z + V2.z); }

VECTOR operator - (const VECTOR& V)
{ return VECTOR (-V.x, -V.y, -V.z); }

VECTOR operator - (const VECTOR& V1, const VECTOR& V2)
{ return VECTOR (V1.x - V2.x, V1.y - V2.y, V1.z - V2.z); }

VECTOR operator * (REAL skalar, const VECTOR& V)
{ return VECTOR (skalar * V.x, skalar * V.y, skalar * V.z); }

VECTOR operator * (const VECTOR& V, REAL skalar)
{ return VECTOR (skalar * V.x, skalar * V.y, skalar * V.z); }

VECTOR operator / (const VECTOR& V, REAL skalar)
{ return V * (1/skalar); }

REAL operator * (const VECTOR& V1, const VECTOR& V2)
{ return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z; }

REAL CosAngle (const VECTOR& V1, const VECTOR& V2)
{
  VECTOR vn1(V1),
         vn2(V2);
  if (vn1.Norm () && vn2.Norm ())
    return vn1 * vn2;
  else
    return 0;
}

VECTOR CrossProduct (const VECTOR& V1, const VECTOR& V2)
{
  return VECTOR (V1.y*V2.z - V1.z*V2.y, 
                 V1.z*V2.x - V1.x*V2.z, 
                 V1.x*V2.y - V1.y*V2.x);
}

REAL det (const VECTOR& a, const VECTOR& b, const VECTOR& c)
{
  return a.x * (b.y*c.z - b.z*c.y)
       + a.y * (b.z*c.x - b.x*c.z)
       + a.z * (b.x*c.y - b.y*c.x);
}


VECTOR operator * (const MATRIX& M, const VECTOR& V)
{ return V.x * M.e1 + V.y * M.e2 + V.z * M.e3; }

MATRIX operator * (const MATRIX& M1, const MATRIX& M2)
{ return MATRIX (M1 * M2.e1, M1 * M2.e2, M1 * M2.e3); }

VECTOR operator * (const MOVEMATRIX& MM, const VECTOR& V)
{ return MM.Matrix * V + MM.Offset; }

MOVEMATRIX operator * (const MOVEMATRIX& MM1, const MOVEMATRIX& MM2)
{ return MOVEMATRIX (MM1.Matrix * MM2.Matrix, MM1 * MM2.Offset); }



MATRIX TurnXYMatrix (REAL deg)
{
  REAL sinv = sin (MakeRad (deg));
  REAL cosv = cos (MakeRad (deg));

  return MATRIX (VECTOR (cosv, sinv, 0.0),
                 VECTOR (-sinv, cosv, 0.0),
                 VECTOR (0.0, 0.0, 1.0));
}

MATRIX TurnXZMatrix (REAL deg)
{
  REAL sinv = sin (MakeRad (deg));
  REAL cosv = cos (MakeRad (deg));

  return MATRIX (VECTOR (cosv, 0.0, -sinv),
                 VECTOR (0.0, 1.0, 0.0),
                 VECTOR (sinv, 0.0, cosv));
}

MATRIX TurnYZMatrix (REAL deg)
{
  REAL sinv = sin (MakeRad (deg));
  REAL cosv = cos (MakeRad (deg));

  return MATRIX (VECTOR (1.0, 0.0, 0.0),
                 VECTOR (0.0, cosv, sinv),
                 VECTOR (0.0, -sinv, cosv));
}


BOOLEAN ExtractAngles (const MATRIX& M, REAL& alpha, REAL& beta, REAL& gamma)
// Bestimme die Winkel, um die die Standard-Matrix gedreht werden
// muss, damit sich die angegebene Matrix ergibt.
// Reihenfolge ist X-Achse, Y-Achse, Z-Achse.
{
  MATRIX m(M);
  if ((! m.e1.Norm ()) ||  // Die Vektoren der Matrix muessen normiert
      (! m.e2.Norm ()) ||  // sein und ungleich (0,0,0)
      (! m.e3.Norm ()) ||
      (m.e1 * m.e2 > EPSILON) ||  // Die Vektoren muessen rechtwinklig
      (m.e2 * m.e3 > EPSILON) ||  // zueinander stehen
      (m.e3 * m.e1 > EPSILON)) return FALSE;

  REAL alphac, betac, gammac, h;

  h = sqrt (sqr (m.e1.x) + sqr (m.e1.y));
  if (h < EPSILON) {
    alpha = 0;
    beta  = -90 * sign (m.e1.z);
    gamma = MakeDeg (acos (m.e2.y)) * -sign (m.e2.x);
  }
  else {
    gammac = m.e1.x / h;
    betac  = h;
    alphac = m.e3.z / h;
    gamma  = MakeDeg (acos (gammac)) * (m.e1.y < 0 ? -1 : 1);
    beta   = MakeDeg (-acos (betac)) * (m.e1.z < 0 ? -1 : 1);
    alpha  = MakeDeg (acos (alphac)) * (m.e2.z < 0 ? -1 : 1);
  }
  return TRUE;
}

#if !defined(_MSC_VER) && (defined(__WIN16__) || defined(__WIN32__))

extern "C" double acos(double d)
{
  if(d < -1)
    return M_PI;
  else if(d > 1)
    return 0;
  else
    return 0.5 * M_PI - asin(d);
}

#endif
