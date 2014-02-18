/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  mystring.h                                            ****
 **** Inhalt: Definition des Typs STRING                            ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __MYSTRING_H_
#define __MYSTRING_H_

#include "Common.h"

// Eigener Stringtyp (zum n-ten Mal definiert)

class STRING
{
  INTEGER Len;
  char*   String;
public:
  STRING () {Len = 0; String = 0;}
  STRING (const char *);
  STRING (const STRING&);
  ~STRING();
  INTEGER Length () const { return Len; }
  STRING& operator =  (const STRING&);
  STRING  operator +  (const STRING&) const;
  STRING  operator *  (INTEGER n) const;
  BOOLEAN operator == (const STRING&) const;
  BOOLEAN operator == (const char*) const;
  operator const char *() const;

  INTEGER Pos (char c) const;           // Wo kommt das Zeichen im String vor?
  STRING  First (INTEGER n) const;      // Liefert die ersten n Zeichen
  STRING  ButFirst (INTEGER n) const;   // Liefert String ohne die ersten n Z.
};

#endif
