/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DPortDef.cc                                       ****
 **** Inhalt: Implementation von Konstruktoren fuer Portdefinitions- ***
 ****         Strukturen                                            ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DPortDef.h"

// Definition der Konstruktoren, siehe auch "sim3DPortDef.h"

APORTDEF::APORTDEF (APORTDEF* prev, char* name, SetValueProc setvalue)
{
  Prev = prev;
  Name = name;
  SetValue = setvalue;
}

INTEGER GetStdDimSize (SIM3DElement*, INTEGER)
{
  return 1;
}

SPORTDEF::SPORTDEF (SPORTDEF* prev, char* name, ReadValueProc readvalue)
{
  Prev = prev;
  Name = name;
  Dimension = 0;
  GetDimSize = GetStdDimSize;
  ReadValue = readvalue;
}

SPORTDEF::SPORTDEF (SPORTDEF* prev, char* name, INTEGER dim,
                    GetDimSizeProc getdimsize, ReadValueProc readvalue)
{
  Prev = prev;
  Name = name;
  Dimension = dim;
  GetDimSize = getdimsize;
  ReadValue = readvalue;
}
