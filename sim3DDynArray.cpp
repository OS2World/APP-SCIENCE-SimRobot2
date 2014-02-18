/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DDynArray.cc                                      ****
 **** Inhalt: Implementation der dynamischen Arrays fuer VECTOR,    ****
 ****         STRING und INTEGER                                    ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DDynArray.h"
#include "sim3DErrors.h"

// Siehe auch "sim3DDynArray.h"

//--- DYNARRAY ------------------------------------------------------------

DYNARRAY::DYNARRAY (INTEGER d)
{
  Count = 0;
  Size  = 0;
  Delta = d;
}

void DYNARRAY::NewElementCount (INTEGER c)
// Wenn sich die Elementanzahl aendert, wird die Groesse des Arrays auf ein
// vielfaches von 'Delta' gebracht.
{
  INTEGER newsize = ((c+Delta-1) / Delta) * Delta;
  if (newsize != Size) Resize (newsize);
}

void DYNARRAY::Remove (INTEGER c)
{
  if (Count > c) {
    NewElementCount (Count-c);
    Count = Count - c;
  }
  else {
    NewElementCount (0);
    Count = 0;
  }
}

//--- VECTORARRAY ---------------------------------------------------------

VECTORARRAY::VECTORARRAY () : DYNARRAY (4)
{
  Data = NULL;
}

VECTORARRAY::VECTORARRAY (const VECTORARRAY& v) : DYNARRAY (v.Delta)
{
  Count = v.Count;
  Delta = v.Delta;
  Size  = v.Count;
  if(Size)
    Data  = new VECTOR [Size];
  INTEGER i;
  for (i=0;i<Count;i++) {Data [i] = v.Data [i];}
}

VECTORARRAY::~VECTORARRAY ()
{
  if (Size > 0) delete [] Data;
}

void VECTORARRAY::Resize (INTEGER newsize)
{
  VECTOR* newdata = 0;
  if (newsize > 0)
  {
    newdata = new VECTOR [newsize];
    INTEGER copysize = ((newsize > Size) ? Size : newsize);
    INTEGER i;
    for (i=0;i<copysize;i++) {newdata [i] = Data [i];}
  }
  if (Size > 0) delete [] Data;
  Data = newdata;
  Size = newsize;
}

void VECTORARRAY::AddElement (const VECTOR& v)
{
  NewElementCount (Count+1);
  Data [Count] = v;
  Count++;
}

VECTOR& VECTORARRAY::operator [] (INTEGER i) const
{
  if (i < 0 || i >= Count)
    sim3DFatalError (sim3DOutOfRange);
  return Data [i];
}

VECTORARRAY& VECTORARRAY::operator = (const VECTORARRAY& varr)
{
  if (this != &varr) {
    if (Size != varr.Size) {
      if (Size != 0) delete [] Data;
      Size  = varr.Size;
      Count = varr.Count;
      if(Size)
        Data  = new VECTOR [Size];
    }
    for (INTEGER i=0;i<Count;i++)
      Data [i] = varr.Data [i];
  }
  return *this;
}


//--- STRINGARRAY ---------------------------------------------------------

STRINGARRAY::STRINGARRAY () : DYNARRAY (12)
{
  Data = NULL;
}

STRINGARRAY::STRINGARRAY (const STRINGARRAY& v) : DYNARRAY (v.Delta)
{
  Count = v.Count;
  Delta = v.Delta;
  Size  = v.Count;
  if(Size)
    Data  = new STRING [Size];
  INTEGER i;
  for (i=0;i<Count;i++) {Data [i] = v.Data [i];}
}

STRINGARRAY::~STRINGARRAY ()
{
  if (Size > 0) delete [] Data;
}

void STRINGARRAY::Resize (INTEGER newsize)
{
  STRING* newdata = 0;
  if (newsize > 0)
  {
    newdata = new STRING [newsize];
    INTEGER copysize = ((newsize > Size) ? Size : newsize);
    INTEGER i;
    for (i=0;i<copysize;i++) {newdata [i] = Data [i];}
  }
  if (Size > 0) delete [] Data;
  Data = newdata;
  Size = newsize;
}

void STRINGARRAY::AddElement (const STRING& s)
{
  NewElementCount (Count+1);
  Data [Count] = s;
  Count++;
}

STRING& STRINGARRAY::operator [] (INTEGER i) const
{
  if (i < 0 || i >= Count)
    sim3DFatalError (sim3DOutOfRange);
  return Data [i];
}

//--- INTEGERARRAY --------------------------------------------------------

INTEGERARRAY::INTEGERARRAY () : DYNARRAY (24)
{
  Data = NULL;
}

INTEGERARRAY::INTEGERARRAY (const INTEGERARRAY& v) : DYNARRAY (v.Delta)
{
  Count = v.Count;
  Delta = v.Delta;
  Size  = v.Count;
  if(Size)
    Data  = new INTEGER [Size];
  INTEGER i;
  for (i=0;i<Count;i++) {Data [i] = v.Data [i];}
}

INTEGERARRAY::~INTEGERARRAY ()
{
  if (Size != 0) delete [] Data;
}

void INTEGERARRAY::Resize (INTEGER newsize)
{
  INTEGER* newdata = 0;
  if (newsize > 0)
  {
    newdata = new INTEGER [newsize];
    INTEGER copysize = ((newsize > Size) ? Size : newsize);
    INTEGER i;
    for (i=0;i<copysize;i++) {newdata [i] = Data [i];}
  }
  if (Size > 0) delete [] Data;
  Data = newdata;
  Size = newsize;
}

void INTEGERARRAY::AddElement (INTEGER i)
{
  NewElementCount (Count+1);
  Data [Count] = i;
  Count++;
}

INTEGER& INTEGERARRAY::operator [] (INTEGER i) const
{
  if (i < 0 || i >= Count)
    sim3DFatalError (sim3DOutOfRange);
  return Data [i];
}
