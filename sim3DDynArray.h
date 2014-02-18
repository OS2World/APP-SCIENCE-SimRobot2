/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor: Uwe Siems                                              ****
 ****                                                               ****
 **** Datei:  sim3DDynArray.h                                       ****
 **** Inhalt: Beschreibung der dynamischen Arrays fuer VECTOR,      ****
 ****         STRING und INTEGER                                    ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DDYNARRAY_H_
#define _SIM3DDYNARRAY_H_

#include "Vector.h"

// Dieses Modul existiert nur, weil mir die existierenden ungetypten Arrays
// suspekt sind und ich nicht voraussetzen wollte, dass der Compiler schon
// Templates kennt. 

// DYNARRAY: abstrakte Klasse zur Realisierung der realen Arrays

class DYNARRAY
{
protected:
  INTEGER Count;
  INTEGER Size, Delta;
public:
           DYNARRAY (INTEGER delta);            // Setzt Delta
  virtual ~DYNARRAY () {};                      // Speicher freigeben
  INTEGER HowMany () const { return Count; };   // Anzahl der Elemente
  void Remove (INTEGER n);                      // Elemente hinten entfernen
protected:
  virtual void Resize (INTEGER to) = 0;         // Array auf 'to' Elemente
                                                // vergroessern/verkleinern
          void NewElementCount (INTEGER count); // Speicher fuer 'count' 
                                                // Elemente beschaffen (benutzt
                                                // Resize)
};

class VECTORARRAY : public DYNARRAY
{
  VECTOR* Data;
public:
           VECTORARRAY ();
           VECTORARRAY (const VECTORARRAY&);    // zusaetzlicher Kopierkonstr.
  virtual ~VECTORARRAY ();
  virtual void Resize (INTEGER to);             // Speicher fuer 'to' Elemente
                                                // allozieren
  void    AddElement (const VECTOR& v);                // Vektor anfuegen
  VECTOR& operator [] (INTEGER n) const;        // Indizierungsoperator
  VECTORARRAY& operator = (const VECTORARRAY&); // Zuweisungsoperator wird fuer
                                                // "sim3DBody.cc" gebraucht
};

class STRINGARRAY : public DYNARRAY             // Wie fuer VECTORARRAY
{
  STRING* Data;
public:
           STRINGARRAY ();
           STRINGARRAY (const STRINGARRAY&);
  virtual ~STRINGARRAY ();
  virtual void Resize (INTEGER to);             
  void    AddElement (const STRING& s);
  STRING& operator [] (INTEGER n) const;
};

class INTEGERARRAY : public DYNARRAY            // Wie fuer VECTORARRAY
{
  INTEGER* Data;
public:
            INTEGERARRAY ();
            INTEGERARRAY (const INTEGERARRAY&);
  virtual  ~INTEGERARRAY ();
  virtual  void Resize (INTEGER by);
  void     AddElement (INTEGER );
  INTEGER& operator [] (INTEGER) const;
};

#endif
