/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  BasicGraphics.cc                                      ****
 **** Inhalt: Beschreibung des virtuellen Graphikports und          ****
 ****         zugehoeriger Klassen                                  ****
 ****                                                               ****
 ***********************************************************************/

#include "BasicGraphics.h"

//-- COLORENTRY -----------------------------------------------------------

// COLORENTRY enthaelt einen Namen, einen RGB-Vektor und einen Zeiger auf das
// naechstes Element in einer verketteten Liste.
// Die Werte in dem Vektor werden in den Bereich von [1;0] gezwungen.
// IsDefinedBy ist eine Vergleichsoperation, die dann Gleichheit liefert
// wenn beide Eintraege den gleichen Namen tragen und das ausfuehrende Element
// keinen RGB-Wert definiert, oder wenn beide Eintraege RGB-Werte tragen und
// die Unterschiede unter einer gewissen Grenze liegen.

class COLORENTRY
{
public:
  COLORENTRY* Next;
  STRING Name;
  VECTOR* RGB;

  COLORENTRY (STRING);
  COLORENTRY (REAL,REAL,REAL);
  COLORENTRY (STRING,REAL,REAL,REAL);
  ~COLORENTRY ();

  BOOLEAN IsDefinedBy (COLORENTRY* c);
};

COLORENTRY::COLORENTRY (STRING name) : Name (name)
{
  Next = NULL;
  RGB  = NULL;
}

inline void RestrictValue (REAL& r)
{
  if (r < 0) r = 0;
  else if (r > 1) r = 1;
}

void RestrictVector (VECTOR& v)
{
  RestrictValue (v.x);
  RestrictValue (v.y);
  RestrictValue (v.z);
}

COLORENTRY::COLORENTRY (REAL r,REAL g,REAL b)
{
  Next = NULL;
  RGB = new VECTOR (r,g,b);
  RestrictVector (*RGB);
}

COLORENTRY::COLORENTRY (STRING name, REAL r, REAL g, REAL b) :
  Name (name)
{
  Next = NULL;
  RGB = new VECTOR (r,g,b);
  RestrictVector (*RGB);
}

COLORENTRY::~COLORENTRY ()
{
  if (Next != NULL) delete Next;
  if (RGB != NULL)  delete RGB;
}

BOOLEAN COLORENTRY::IsDefinedBy (COLORENTRY* c)
{
  return (Name == c->Name) && ((RGB == NULL) ||
     ((c->RGB != NULL) && (abs (*RGB - *(c->RGB)) < EPSILON)));
}

//-- COLORLIST ------------------------------------------------------------

COLORLIST::COLORLIST ()
{
  First  = NULL;
  Actual = NULL;
  Count  = 0;
}

COLORLIST::~COLORLIST ()
{
  delete First;
}

// FindEntry: Durchsuche die existierende Liste nach uebereinstimmender Farbe,
// wenn keine gefunden, trage neu ein. Ansonsten gebe den Speicher fuer den
// Eintrag frei.

COLOR COLORLIST::FindEntry (COLORENTRY* centry)
{
  if (First == NULL) {
    First = centry;
    Count++;
    return 0;
  }
  else {
    COLORENTRY* LoopCol = First;
    INTEGER nr = 0;
    while (LoopCol != NULL) {
      if (centry->IsDefinedBy (LoopCol)) {
        delete centry;
        return nr;
      }
      nr++;
      if (LoopCol->Next == NULL) {
        LoopCol->Next = centry;
        Count++;
        return nr;
      }
      else
        LoopCol = LoopCol->Next;
    }
  }
  return -1;
}

COLOR COLORLIST::GetColor (const STRING& colname)
{
  COLORENTRY* centry = new COLORENTRY (colname);
  return FindEntry (centry);
}

COLOR COLORLIST::GetColor (REAL r, REAL g, REAL b)
{
  COLORENTRY* centry = new COLORENTRY (r,g,b);
  return FindEntry (centry);
}

COLOR COLORLIST::AddColorDef (const STRING& colname, REAL r, REAL g, REAL b)
{
  COLORENTRY* centry = new COLORENTRY (colname,r,g,b);
  return FindEntry (centry);
}

INTEGER COLORLIST::HowMany ()
{
  COLORENTRY* LoopCol = First;
  INTEGER count = 0;
  while (LoopCol != NULL) {
    LoopCol = LoopCol->Next;
    count++;
  }
  return count;
}

void COLORLIST::FirstColor ()
{
  Actual = First;
}

void COLORLIST::NextColor ()
{
  if (Actual != NULL)
    Actual = Actual->Next;
}

void COLORLIST::NthColor (INTEGER n)
{
  if (n < 0)
    Actual = NULL;
  else {
    Actual = First;
    while ((Actual != NULL) && (n > 0)) {
      Actual = Actual->Next;
      n--;
    }
  }
}

BOOLEAN COLORLIST::EndOfList ()
{
  return (Actual == NULL);
}

BOOLEAN COLORLIST::RGBDefined ()
{
  return ((Actual != NULL) && (Actual->RGB != NULL));
}

VECTOR COLORLIST::ActualRGB ()
{
  if (Actual != NULL)
    return *(Actual->RGB);
  else
    return VECTOR ();
}

STRING COLORLIST::ActualName ()
{
  if (Actual != NULL)
    return Actual->Name;
  else
    return "";
}

// Lese einen Farbwert von einer Datei. Dieser kann ein String sein, oder ein
// Zahlenvektor. Siehe auch "sim3DFile.h"

COLOR COLORLIST::ReadColor (SCANNER& scan)
{
  if (scan.Symbol == sString) {
    STRING colname = scan.GetLastString ();
    scan.NextSymbol ();
    return GetColor (colname);
  }
  else {
    VECTOR rgb;
    if (GetCorrectVector (scan, rgb))
      return GetColor (rgb.x, rgb.y, rgb.z);
    else
      return -1;
  }
}

// Schreibe einen Farbwert in eine Datei. Farbnamen werden bevorzugt.
// Siehe auch "sim3DFile.h"

void COLORLIST::WriteColor (BACKUP& backup, const STRING& id, COLOR col)
{
  NthColor (col);
  if (!EndOfList ()) {
    backup.WriteIdent (id);
    if ((!(ActualName () == "")) || (!RGBDefined ()))
      backup.WriteString (ActualName ());
    else
      backup.WriteVector (ActualRGB ());
    backup.ListNext ();
  }
}

