/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DError.h                                          ****
 **** Inhalt: Beschreibung der moeglichen Fehler und Fehler-        ****
 ****         behandlungsfunktionen                                 ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DERROR_H_
#define _SIM3DERROR_H_

#include "Common.h"

// Ueber die hier definierten Symbole werden alle Texte in der Simulation
// angesprochen. Dadurch ist ein leichte Anpassung an andere Sprachen
// (z.B. Englisch, Franzoesisch) moeglich.
// Alle Texte, deren Symbole auf eine 1 enden, erwarten einen weiteren
// Parameter beim Aufruf von sim3DOtherError. Dieser String wird in den
// Text eingebaut.

enum sim3DError {               // Fehlerarten:
  sim3DNoError,                 //   Kein Fehler
  sim3DOutOfRange,              //   Integer-Wert ausserhalb der zulaess. Grenz.
  sim3DNoMemory,                //   Speicher erschoepft
  sim3DInternalError,           //   Fehler des Programmierers
  sim3DInvalidObject,           //   Falsches Objekt als Parameter angegeben
  sim3DPortNotFound,            //   Angegebener Pfad wurde nicht gefunden
  sim3DBadActorPort,            //   Der angegebene Port existiert nicht
  sim3DBadSensorPort,           //   dito
  sim3DTooFewEdges,             //   Die letzte Flaeche hat zu wenig Ecken
  sim3DAlreadyDefined,          //   Polyeder ist bereits defininiert
  sim3DNotInitialized,          //   Datenstruktur ist nicht initalisiert
  sim3DNotImplemented,          //   Fehlende Implementierung innerhalb der
                                //     Simulation
  sim3DOverflow,                //   Ueberlauffehler
  sim3DInvalidTextureFile,      //   Formatfehler in Texture-Datei 
  sim3DPointUndef,              //   Punktindex ungueltig
  sim3DHighTextureNumber,       //   Zu hohe Textur-Farbnummer
  sim3DEofInComment,            //   Dateiende in Kommentar
  sim3DFileNotFound1,           //   Datei '?' nicht gefunden
  sim3DUnknownTextureType1,     //   Texturtyp '?' unbekannt
  sim3DUnknownTexture1,         //   Textur '?' unbekannt
  sim3DUnknownAttribute1,       //   Attribut '?' unbekannt
  sim3DUnknownMaterial1,        //   Materialbeschreibung '?' unbekannt
  sim3DUnknownMacro1,           //   Makro '?' unbekannt
  sim3DUnknownObjectType1,      //   Objekttyp '?' unbekannt
  sim3DExpectDifferent,         //   Unterschiedliche Werte erwartet
  sim3DExpectPositive,          //   Positive ganze Zahl erwartet
  sim3DExpectTextureType,       //   Texturtyp erwartet
  sim3DExpectObjectType,        //   Objekttyp erwartet
  sim3DExpectObjectName,        //   Objektname erwartet
  sim3DExpectWorld,             //   'WORLD' erwarted
  sim3DExpectAs,                //   'AS' erwartet
  sim3DExpectEof,               //   Dateiende erwartet
  sim3DExpectString,            //   Zeichenkette erwartet
  sim3DExpectInteger,           //   Ganze Zahl erwartet
  sim3DExpectReal,              //   Zahl erwartet
  sim3DExpectIdent,             //   Bezeichner erwartet
  sim3DExpectSymbol1,           //   '?' erwartet
  sim3DOpeningAngle,            //   Zahl im Bereich [0..120] erwartet
  sim3DGroupExpected,           //   Makro muss in eine Gruppe eingefuegt werden
  sim3DTextError,               //   Text: "Fehler: "
  sim3DTextLine,                //   Text: " in Zeile "
  sim3DTextColumn,              //   Text: ", Spalte "
  sim3DTextFatalError           //   Text: "Fataler Fehler: "
};

// Position innerhalb einer Datei.

struct FILEPOS {
  INTEGER Line, Column;

  FILEPOS ()
    { Line = 0; Column = 0; }
  FILEPOS (INTEGER l, INTEGER c)
    { Line = l; Column = c; };
  BOOLEAN operator == (FILEPOS p)
    { return ((Line == p.Line) && (Column == p.Column)); };
};

sim3DError sim3DTakeLastError ();               // Hole und loesche Fehler
sim3DError sim3DGetLastError ();                // Hole Fehler
void       sim3DErrorOutputOn ();               // Fehler nach stderr ausgeben
void       sim3DErrorOutputOff ();              // Fehler nicht ausgeben
void       sim3DFatalError (sim3DError);        // Fataler Fehler: error
void       sim3DOtherError (sim3DError, const STRING& s = ""); // 'Harmloser' Fehler 
void       sim3DOtherError (sim3DError,const FILEPOS&);        // mit und ohne Zusatztext
void       sim3DOtherError (sim3DError,const STRING&,const FILEPOS&); // und Position
void       sim3DCheckAlloc (void*);             // Test, ob Speicher vorhanden

#endif
