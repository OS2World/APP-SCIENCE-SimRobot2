/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DError.cc                                         ****
 **** Inhalt: Fehlerbehandlungsroutinen                             ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DErrors.h"
#include "Common.h"
#include <iostream.h>
#include <stdio.h>
extern "C" void exit(int);

// Siehe auch "sim3DError.h"

static sim3DError LastError = sim3DNoError;
static BOOLEAN OutputErrors = TRUE;

char* sim3DErrorMessage (sim3DError error)
{
  switch (error) {
#ifdef _GERMAN
    case sim3DNoError:            return "Kein Fehler";
    case sim3DOutOfRange:         return "Bereichsueberlauf";
    case sim3DNoMemory:           return "Speicher erschoepft";
    case sim3DInternalError:      return "Interner Fehler";
    case sim3DInvalidObject:      return "Kein gueltiges Objekt";
    case sim3DPortNotFound:       return "Port nicht gefunden";
    case sim3DBadActorPort:       return "Fehlerhafter ActorPort";
    case sim3DBadSensorPort:      return "Fehlerhafter SensorPort";
    case sim3DTooFewEdges:        return "Flaeche hat zu wenig Kanten";
    case sim3DAlreadyDefined:     return "Koerperdefinition ist bereits beendet";
    case sim3DNotInitialized:     return "Nicht initialisiert";
    case sim3DOverflow:           return "Ueberlauf in Datenstruktur";
    case sim3DInvalidTextureFile: return "Ungueltige Texturdatei";
    case sim3DPointUndef:         return "Punktindex ist ungueltig";
    case sim3DHighTextureNumber:  return "Zu hohe Textur-Farbnummer";
    case sim3DEofInComment:       return "Dateiende in Kommentar";
    case sim3DFileNotFound1:      return "Datei '%s' nicht gefunden";
    case sim3DUnknownTextureType1:return "Texturtyp '%s' unbekannt";
    case sim3DUnknownTexture1:    return "Textur '%s' unbekannt";
    case sim3DUnknownAttribute1:  return "Attribut '%s' unbekannt";
    case sim3DUnknownMaterial1:   return "Materialbeschreibung '%s' unbekannt"; 
    case sim3DUnknownMacro1:      return "Makro '%s' unbekannt";
    case sim3DUnknownObjectType1: return "Objekttyp '%s' unbekannt";
    case sim3DExpectDifferent:    return "Unterschiedliche Werte erwartet";
    case sim3DExpectPositive:     return "Positive ganze Zahl erwartet";
    case sim3DExpectTextureType:  return "Texturtyp erwartet";
    case sim3DExpectObjectType:   return "Objekttyp erwartet";
    case sim3DExpectObjectName:   return "Objektname erwartet";
    case sim3DExpectWorld:        return "'WORLD' erwarted";
    case sim3DExpectAs:           return "'AS' erwartet";
    case sim3DExpectEof:          return "Dateiende erwartet";
    case sim3DExpectString:       return "Zeichenkette erwartet";
    case sim3DExpectInteger:      return "Ganze Zahl erwartet";
    case sim3DExpectReal:         return "Zahl erwartet";
    case sim3DExpectIdent:        return "Bezeichner erwartet";
    case sim3DExpectSymbol1:      return "'%s' erwartet";
    case sim3DOpeningAngle:       return "Zahl im Bereich [0..120] erwartet";
    case sim3DGroupExpected:      return "Makro muss in eine Gruppe eingefuegt werden";
    case sim3DTextError:          return "Fehler: ";
    case sim3DTextLine:           return " in Zeile ";
    case sim3DTextColumn:         return ", Spalte ";
    case sim3DTextFatalError:     return "Fataler Fehler: ";
    default:                      return "*** (Text fehlt) ***";
#else
    case sim3DNoError:            return "no error";
    case sim3DOutOfRange:         return "index overflow";
    case sim3DNoMemory:           return "out of memory";
    case sim3DInternalError:      return "internal error";
    case sim3DInvalidObject:      return "no valid object";
    case sim3DPortNotFound:       return "port not found";
    case sim3DBadActorPort:       return "invalid actorport";
    case sim3DBadSensorPort:      return "invalid sensorport";
    case sim3DTooFewEdges:        return "surface has too little edges";
    case sim3DAlreadyDefined:     return "body definition is already finished";
    case sim3DNotInitialized:     return "not initialized";
    case sim3DOverflow:           return "overflow in data structure";
    case sim3DInvalidTextureFile: return "invalid texture file";
    case sim3DPointUndef:         return "point index is bad";
    case sim3DHighTextureNumber:  return "texture color number too big";
    case sim3DEofInComment:       return "end of file in comment";
    case sim3DFileNotFound1:      return "file '%s' not found";
    case sim3DUnknownTextureType1:return "texture type '%s' unknown";
    case sim3DUnknownTexture1:    return "texture '%s' unknown";
    case sim3DUnknownAttribute1:  return "attribute '%s' unknown";
    case sim3DUnknownMaterial1:   return "material description '%s' unknown"; 
    case sim3DUnknownMacro1:      return "macro '%s' unknown";
    case sim3DUnknownObjectType1: return "objekt type '%s' unknown";
    case sim3DExpectDifferent:    return "different values expected";
    case sim3DExpectPositive:     return "positive integer expected";
    case sim3DExpectTextureType:  return "texture type expected";
    case sim3DExpectObjectType:   return "object type expected";
    case sim3DExpectObjectName:   return "object name expected";
    case sim3DExpectWorld:        return "'WORLD' expected";
    case sim3DExpectAs:           return "'AS' expected";
    case sim3DExpectEof:          return "end of file expected";
    case sim3DExpectString:       return "string expected";
    case sim3DExpectInteger:      return "integer expected";
    case sim3DExpectReal:         return "number expected";
    case sim3DExpectIdent:        return "identifier expected";
    case sim3DExpectSymbol1:      return "'%s' expected";
    case sim3DOpeningAngle:       return "number in the range [0..120] expected";
    case sim3DGroupExpected:      return "macro must be inserted in a group";
    case sim3DTextError:          return "Error: ";
    case sim3DTextLine:           return " in line ";
    case sim3DTextColumn:         return ", column ";
    case sim3DTextFatalError:     return "Fatal error: ";
    default:                      return "*** (missing text) ***";
#endif
  }
}

sim3DError sim3DTakeLastError ()
{
  sim3DError error = LastError;
  LastError = sim3DNoError;
  return error;
}

sim3DError sim3DGetLastError ()
{
  return LastError;
}

#if defined(__WIN16__) || defined (__WIN32__) || defined (__OS2__)

extern void WinExit(const char*);
extern char ErrorMessage[];
extern int ErrorLine,ErrorColumn;

void sim3DFatalError (sim3DError error)
{
  STRING s = STRING(sim3DErrorMessage(sim3DTextFatalError)) +
				 STRING(sim3DErrorMessage (error)) + STRING(".");
  WinExit(s);
}

void sim3DOtherError (sim3DError error, const STRING& s)
{
  if (OutputErrors && !*ErrorMessage)
  {
    
    char buf[200];
    sprintf(buf,sim3DErrorMessage (error), (const char*) s);
    sprintf(ErrorMessage,"%s%s.",sim3DErrorMessage(sim3DTextError),buf);
    ErrorLine = 0;
  }
  LastError = error;
}

void sim3DOtherError (sim3DError error,const STRING& s,const FILEPOS& pos)
{
  if (OutputErrors && !*ErrorMessage)
  {
    sim3DOtherError(error,s);
    ErrorLine = pos.Line;
    ErrorColumn = pos.Column;
  }
  LastError = error;
}

#else

void sim3DFatalError (sim3DError error)
{
  cerr << sim3DErrorMessage(sim3DTextFatalError) << sim3DErrorMessage (error)
		 << ".\n";
  exit (1);
}

void sim3DOtherError (sim3DError error, const STRING& s)
{
  if (OutputErrors) {
    char buf[200];
    sprintf(buf, sim3DErrorMessage (error), (const char*) s);
    cerr << sim3DErrorMessage(sim3DTextError) << buf << ".\n";
  }
  LastError = error;
}

void sim3DOtherError (sim3DError error,const STRING& s,const FILEPOS& pos)
{
  if (OutputErrors)
  {
    char buf[200];
    sprintf(buf, sim3DErrorMessage (error), (const char*) s);
    cerr << sim3DErrorMessage(sim3DTextError) << buf
         << sim3DErrorMessage(sim3DTextLine) << pos.Line
         << sim3DErrorMessage(sim3DTextColumn) << pos.Column << ".\n";
  }
  LastError = error;
}

#endif // __WIN16__ || __WIN32__ || __OS2__

void sim3DOtherError (sim3DError error,const FILEPOS& pos)
{
  sim3DOtherError (error, "", pos);
}

void sim3DErrorOutputOn ()
{
  OutputErrors = TRUE;
}

void sim3DErrorOutputOff ()
{
  OutputErrors = FALSE;
}

void sim3DCheckAlloc (void* p)
{
  if (p == NULL) sim3DFatalError (sim3DNoMemory);
}
