/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DFile.h                                           ****
 **** Inhalt: Export von Funktionen zum Lesen und Schreiben von     ****
 ****         Definitionsdateien                                    ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SIM3DFILE_H_
#define _SIM3DFILE_H_

#include "Common.h"
#include "Vector.h"
#include "sim3DErrors.h"
#include <fstream.h>

// Dieses Modul stellt Klassen und Funktionen zum Lesen, Analysieren und
// Schreiben von Definitionsdateien der Simulation zur Verfuegung.
// Eine Definitionsdatei wird von einem Scanner in Symbole zerlegt, wie sie
// in SYMBOL aufgelistet sind. Diese Symbole sind gueltige Elemente einer
// Definitionsdatei. Nicht definiert Symbole erzeugen einen Fehler.

enum SYMBOL {
  sNull, sPeriod, sInteger, sReal, sString, sEqual,
  sLParen, sComma, sRParen, sLBracket, sSemicolon, sRBracket,
  sColon, sIdent, sEof
};

// Moegliche Fehler, die beim Scannen auftreten koennen.

enum SCANERR {
  scerrNoError,
  scerrAbruptEnd,
  scerrStringTooLong,
  scerrFileNotClosed,
  scerrNoFileOpen,
  scerrCouldNotOpenFile,
  scerrBadNumber,
  scerrNumberTooLong,
  scerrSyntaxError
};

// Die Klasse SCANNER zerlegt eine Datei in Symbole und liest dabei Integer,
// reelle Zahlen, Strings und Bezeichner ein. Kommentare werden ueberlesen.
// Werden die Konventionen fuer die Symbole nicht eingehalten, wird ein Fehler
// erzeugt.

class SCANNER
{
public:
  SYMBOL  Symbol;               // Letztes gelesenes Symbol
  BOOLEAN ErrorOccured;         // Fehler aufgetreten?
private:
  ifstream* File;               // Datei, von der gelesen wird
  FILEPOS Pos;                  // Position in der Datei
  STRING  FileName;             // Dateiname

  LONGINT LastInt;              // Letzter gelesener Integer
  REAL    LastReal;             // Letzte gelesene reelle Zahl
  STRING  LastString;           // Letzter gelesener String oder Bezeichner
  FILEPOS LastPos;              // Anfangsposition des letzten Symbols
  char    LastChar;             // Letztes aus der Datei gelesenes Zeichen
  SCANERR LastError;            // Letzter aufgetretener Fehler
  FILEPOS ErrorPos;             // Position des letzten Fehlers

  void    ReadInteger ();       // Lese ein Integer aus Datei
  void    ReadReal ();          // usw.
  void    ReadString ();
  void    ReadIdent ();
  void    ReadChar ();
  void    ReadNestedComment (); // (Ueber)lese eine geschachtelten Kommentar
  void    ReadLineComment ();   // (Ueber)lese eine Kommentarzeile
  void    Error (SCANERR err);  // Setze Fehler
  SYMBOL  GetSymbol ();         // Lese ein Symbol aus Datei

public:
  SCANNER (const STRING& filename); // Oeffne den Scanner mit angegebener Datei
  ~SCANNER ();                  // Scanner schliessen

  LONGINT GetLastInt ();        // Gibt den letzten gelesenen Integer zurueck
  REAL    GetLastReal ();       // uws.
  STRING  GetLastString ();
  FILEPOS GetLastPos ();
  SCANERR GetLastError ();
  FILEPOS GetFilePos ();

  void    NextSymbol ();        // Hole das naechste Symbol

  void    Skip (SYMBOL sym);    // Lese solange Symbole, bis ein Symbol kommt,
				// dass gleich 'sym' ist oder in der Reihenfolge
				// von 'SYMBOL' nach diesem kommt.
  void    SyntaxError (sim3DError error, SYMBOL sym = sNull);
  void    SyntaxError (sim3DError error, const STRING& s, SYMBOL sym = sNull);
				// Erzeuge Fehlermeldung und fuehre Skip mit
				// Symbol 'sym' aus, wenn angegeben. 's' ist
				// ein Text, der in die Fehlermeldung anstelle
				// von '%s' eingesetzt wird
  BOOLEAN TestFor (SYMBOL sym1, SYMBOL sym2 = sNull);
				// Teste, ob 'sym1' folgt, fuehre Skip aus, wenn
				// nicht und 'sym2' wurde angegeben.
  void    Expect  (SYMBOL sym1, SYMBOL sym2 = sNull);
				// Teste, ob 'sym1' folgt, gebe Fehlermeldung
				// aus, wenn nicht und fuehre Skip aus, wenn
				// 'sym2' angegeben.
};

// Lesen von Werten durch SCANNER automatisiert:
// Automatische Ausgabe von Fehlermeldung und Skip zum naechsten Aufsetzpunkt.

BOOLEAN GetCorrectInteger (SCANNER&, INTEGER&);
BOOLEAN GetCorrectReal    (SCANNER&, REAL&);
BOOLEAN GetCorrectIntPair (SCANNER&, INTEGER&, INTEGER&);
BOOLEAN GetCorrectRealPair(SCANNER&, REAL&, REAL&);
BOOLEAN GetCorrectString  (SCANNER&, STRING&);
BOOLEAN GetCorrectVector  (SCANNER&, VECTOR&);

class LISTREC;

// BACKUP fuehrt die gegenteilige Funktion von SCANNER durch: Diese Klasse
// dient dazu, auf einfache Weise eine Definitionsdatei zu erzeugen.
// Sie hat Funktionen, um Symbole zu schreiben und Listen mit automatischer
// Einrueckung zu generieren, so dass diese Datei wieder gut lesbar wird.

class BACKUP
{
  ofstream* File;               // Zu schreibende Datei
  LISTREC* ListStack;           // Stack zur Verwaltung der eingerueckten Listen
  void PrepareSymbol ();        // 
  INTEGER IndentLevel;          // Einruecktiefe
public:
  STRING  FileName;             // Dateiname
  BOOLEAN FileOpen;             // Datei geoeffnet?
  BOOLEAN ErrorOccured;         // Fehler aufgetreten?

  BACKUP (const STRING& filename); // Datei mit Namen 'filename' oeffnen
  ~BACKUP ();                   // Datei schliessen

// Starte eine neue Liste mit den angegebenen Symbolen als Startsymbol, Trenn-
// symbol und Endsymbol. 'force' gibt an, ob die Ausgabe der Start- und End-
// symbole auch erfolgen soll, wenn die Liste leer ist.
// Die Elemente werden hintereinander geschrieben.
  void StartList  (SYMBOL start, SYMBOL sep, SYMBOL end, BOOLEAN force);
// Eingerueckte Version: Die Elemente der Liste werden eingerueckt und 
// untereinander geschrieben.
  void StartListI (SYMBOL start, SYMBOL sep, SYMBOL end, BOOLEAN force);
// ListNext wird *nach* jedem einzelnen Listenelement aufgerufen.
  void ListNext   ();
// EndList wird zusaetzlich nach dem Ende der Liste aufgerufen, um (wenn die
// Liste nicht leer war, oder 'force' gesetzt wurde) das Listenende zu
// schreiben.
  void EndList    ();

  void WriteSymbol (SYMBOL);            // Schreiben (Sonderzeichen-)Symbol
  void WriteIdent  (const STRING&);     // Schreibe Bezeichner
  void WriteString (const STRING&);     // Schreibe String
  void WriteReal   (REAL);              // usw.
  void WriteInteger(INTEGER);
  void WriteVector (const VECTOR&);
  void WriteLine   ();                  // Neue Zeile
  void WriteComment(const STRING&);     // Fuege Kommentarzeile ein
  void WriteIdRealNext (const STRING&,REAL); // Kombinierte Ausgabe
  void WriteIdIntNext  (const STRING&,INTEGER);
  void WriteIdStrNext  (const STRING&,const STRING&);
};

#endif
