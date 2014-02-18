/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DFile.cc                                          ****
 **** Inhalt: Implementation der Dateifunktionen                    ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DFile.h"
#include <ctype.h>
#include <math.h>

static const myEOF = '\0';

static const myEOL = '\12';

char SymbolRepr (SYMBOL sym)
// Liefert das Zeichen zu einem Symbol
{
  switch (sym) {
    case sLParen:    return '(';
    case sRParen:    return ')';
    case sComma:     return ',';
    case sColon:     return ':';
    case sSemicolon: return ';';
    case sEqual:     return '=';
    case sLBracket:  return '[';
    case sRBracket:  return ']';
    case sPeriod:    return '.';
    default:         return '?';
  }
}

//-- SCANNER -------------------------------------------------------------

SCANNER::SCANNER (const STRING& name)
{
  File = new ifstream (name, ios::in | ios::nocreate);
  if (File->fail ()) {
    sim3DOtherError(sim3DFileNotFound1, name);
    ErrorOccured = TRUE;
  }
  else {
    ErrorOccured = FALSE;
    LastError= scerrNoError;
    FileName = name;
    Pos      = FILEPOS (1, 1);
    LastInt  = 0;
    LastReal = 0.0;
    LastString = "";
    LastPos  = FILEPOS (0, 0);
    File->get (LastChar);
    ErrorPos = LastPos;
  }
}

SCANNER::~SCANNER ()
{
  delete File;
}

LONGINT SCANNER::GetLastInt ()
{
  return LastInt;
}

REAL SCANNER::GetLastReal ()
{
  return LastReal;
}

STRING SCANNER::GetLastString ()
{
  return LastString;
}

FILEPOS SCANNER::GetLastPos ()
{
  return LastPos;
}

SCANERR SCANNER::GetLastError ()
{
  SCANERR le = LastError;
  LastError = scerrNoError;
  return le;
}

FILEPOS SCANNER::GetFilePos ()
{
  return Pos;
}

void SCANNER::Error (SCANERR err)
{
  LastError = err;
  ErrorOccured = TRUE;
}

void SCANNER::ReadChar ()
{
  Pos.Column++;
  if (LastChar == myEOL) {
    Pos.Line++;
    Pos.Column = 1;
  }
  if (File->eof ()) {
    LastChar = myEOF;
    return;
  }
  File->get (LastChar);
}

void SCANNER::ReadString ()
// Lesen eines Strings.
// Folgende Backslash-Zeichen sind zugelassen:
// \\ = Backslash
// \n = Zeilenvorschub
// \" = "
// \Kontrollzeichen = Leerstelle
// \sonst. Zeichen = Zeichen selbst
{
  INTEGER k = 0;
  BOOLEAN stop = FALSE;

  char Input [256];

  do {
    if (LastChar == myEOF) {
      stop = TRUE;
      Error (scerrAbruptEnd);
    }
    else {
      ReadChar ();
      if (LastChar == '"')
        stop = TRUE;
      else if (LastChar == '\\') {
        if (LastChar == myEOF) {
          stop = TRUE;
          Error (scerrAbruptEnd);
        }
        else {
          ReadChar ();
          if (LastChar == 'n') LastChar = myEOL;
          else if (iscntrl (LastChar)) LastChar = ' ';
        }
      }
      if (! stop) {
        if (k >= 256) {
          stop = TRUE;
          while ((LastChar != myEOF) && (LastChar != '"'))
            ReadChar ();
          Error (scerrStringTooLong);
        }
        else {
          Input [k] = LastChar;
          k++;
        }
      }
    }
  } while (! stop);

  Input [k] = '\0';
  LastString = STRING (Input);
}

void SCANNER::ReadIdent ()
// Lesen eines Bezeichners. Bezeichner werden automatisch gross geschrieben.
// Zugelassen sind alle Buchstaben, Ziffern und der Unterstrich
{
  char Input [80];

  INTEGER k = 0;

  do {
    Input [k] = LastChar;
    k++;
    ReadChar ();
    LastChar = toupper (LastChar);
  } while (isalnum (LastChar) || (LastChar == '_'));
  Input [k] = '\0';
  LastString = STRING (Input);
}

void SCANNER::ReadInteger ()
// Lesen einer ganzen Zahl
{
  INTEGER i = 0;

  BOOLEAN sign = FALSE;

  char digit [32];

  if ((LastChar == '+') || (LastChar == '-')) { // Vorzeichen lesen
    sign = (LastChar == '-');
    ReadChar ();
  }

  if (! isdigit (LastChar)) {
    Error (scerrBadNumber);
    LastInt = 0;
    return;
  }
  do {                                          // Ziffern lesen
    digit [i] = LastChar;
    i++;
    ReadChar ();
  } while (isdigit (LastChar) && (i < 31));

  LONGINT sum = 0;
  INTEGER j;

  if (i >= 32)                                  // Ziffern aufaddieren
    Error (scerrNumberTooLong);
  else {
    for (j=0;j<i;j++)
      sum = sum * 10 + (digit [j] - '0');
  }

  if (sign)                                     // Vorzeichen beachten
    LastInt = -sum;
  else
    LastInt = sum;
}

void SCANNER::ReadReal ()
{
  REAL r = LastInt;                     // Ein Real beginnt mit einem Integer

  INTEGER pw = -1;

  while (isdigit (LastChar)) {
    r = r + (LastChar - '0') * pow (10.0, pw);  // Lesen der Nachkommastellen
    pw--;
    ReadChar ();
  }

  while (isspace(LastChar) && (LastChar != myEOF))  ReadChar ();

  if (toupper (LastChar) == 'E') {                // Exponent lesen
    ReadChar ();

    while (isspace(LastChar) && (LastChar != myEOF))  ReadChar ();

    LONGINT help = LastInt;
    ReadInteger ();
    r = r * pow (10.0, LastInt);
    LastInt = help;
  }

  LastReal = r;
}

// Kommentare: zugelassen sind die aus C und C++ bekannten Kommentarzeichen

void SCANNER::ReadLineComment ()
{
  while ((LastChar != myEOL) && (LastChar != myEOF)) ReadChar ();
  if (LastChar != myEOF) ReadChar ();
}

void SCANNER::ReadNestedComment ()
{
  INTEGER CommentLevel = 1;
  while (CommentLevel > 0) {
    char LC = LastChar;
    ReadChar ();
    if (LC == '/') {
      if (LastChar == '*') {
        ReadChar ();
        CommentLevel++;
      }
      else if (LastChar == '/') {
        ReadChar ();
        ReadLineComment ();
      }
    }
    else if (LC == '*') {
      if (LastChar == '/') {
        ReadChar ();
        CommentLevel--;
      }
    }
    else if (LC == myEOF) {
      SyntaxError (sim3DEofInComment);
      return;
    }
  }
}

SYMBOL SCANNER::GetSymbol ()
// Lese ein Symbol aus der Datei
{
  do {
    while (isspace(LastChar) && (LastChar != myEOF))   ReadChar ();
    if (LastChar == '/') {
      ReadChar ();
      if (LastChar == '*') {
        ReadChar ();
        ReadNestedComment ();
      }
      else if (LastChar == '/') {
        ReadChar ();
        ReadLineComment ();
      }
      else
        return sNull;
    }
  } while ((isspace(LastChar) && (LastChar != myEOF)) || (LastChar == '/'));

  LastPos = Pos;

  switch (LastChar) {
    case myEOF:ReadChar (); return sEof;
    case '"':  ReadString ();
               ReadChar (); return sString;
    case '(':  ReadChar (); return sLParen;
    case ')':  ReadChar (); return sRParen;
    case ',':  ReadChar (); return sComma;
    case '.':  ReadChar (); return sPeriod;
    case ':':  ReadChar (); return sColon;
    case ';':  ReadChar (); return sSemicolon;
    case '=':  ReadChar (); return sEqual;
    case '[':  ReadChar (); return sLBracket;
    case ']':  ReadChar (); return sRBracket;
    default:
      if ((LastChar == '+') || (LastChar == '-') || isdigit (LastChar)) {
        BOOLEAN Negative = LastChar == '-';
        ReadInteger ();
        if (LastChar == '.') {
          ReadChar ();
          if (Negative)
            LastInt = -LastInt;
          ReadReal ();
          if (Negative)
            LastReal = -LastReal;
          return sReal;
        }
        else {
          LastReal = LastInt;
          if (LastInt >= 0)     // Nur ganze Zahlen groesser null werden als
            return sInteger;    // Integer bezeichnet
          else
            return sReal;       // alle anderen werden als Real betrachtet
        }
      }
      else if (isalpha (LastChar)) {
        ReadIdent ();
        return sIdent;
      }
      else {
        ReadChar ();
        return sNull;
      }
  }
}

void SCANNER::NextSymbol ()
{
  Symbol = GetSymbol ();
}

void SCANNER::Skip (SYMBOL s)
{
  while (Symbol < s)  NextSymbol ();
}

void SCANNER::SyntaxError (sim3DError error, SYMBOL sym)
{
  SyntaxError (error,"",sym);
}

void SCANNER::SyntaxError (sim3DError error, const STRING& s, SYMBOL sym)
{
  if (! (ErrorPos == LastPos)) {
    sim3DOtherError(error,s,LastPos);
    Error (scerrSyntaxError);
    ErrorPos = LastPos;
  }
  if (sym != sNull)
    Skip (sym);
}

BOOLEAN SCANNER::TestFor (SYMBOL sym1, SYMBOL sym2)
// Siehe "sim3DFile.h"
{
  if ((Symbol == sym1) || ((sym1 == sReal) && (Symbol == sInteger)))
    return TRUE;

  char* sym;

  switch (sym1) {
    case sEof:       SyntaxError (sim3DExpectEof);     return FALSE;
    case sString:    SyntaxError (sim3DExpectString);  return FALSE;
    case sInteger:   SyntaxError (sim3DExpectInteger); return FALSE;
    case sReal:      SyntaxError (sim3DExpectReal);    return FALSE;
    case sIdent:     SyntaxError (sim3DExpectIdent);   return FALSE;
    case sLParen:    sym = "("; break;
    case sRParen:    sym = ")"; break;
    case sComma:     sym = ","; break;
    case sColon:     sym = ":"; break;
    case sSemicolon: sym = ";"; break;
    case sEqual:     sym = "="; break;
    case sLBracket:  sym = "["; break;
    case sRBracket:  sym = "]"; break;
    case sPeriod:    sym = "."; break;
    default:         sym = "???";
  }
  SyntaxError (sim3DExpectSymbol1, sym, sym2);
  return FALSE;
}

void SCANNER::Expect (SYMBOL sym1, SYMBOL sym2)
// Siehe "sim3DFile.h"
{
  if (TestFor (sym1, sym2)) NextSymbol ();
}

//-- Lesefunktionen zum SCANNER ------------------------------------------
//
// Siehe "sim3DFile.h"

BOOLEAN GetCorrectInteger (SCANNER& scan, INTEGER& i)
{
  if (scan.TestFor (sInteger, sComma)) {
    i = (INTEGER) scan.GetLastInt ();
    scan.NextSymbol ();
    return TRUE;
  }
  else
    return FALSE;
}

BOOLEAN GetCorrectReal (SCANNER& scan, REAL& r)
{
  if (scan.TestFor (sReal, sComma)) {
    r = scan.GetLastReal ();
    scan.NextSymbol ();
    return TRUE;
  }
  else
    return FALSE;
}


BOOLEAN GetCorrectIntPair (SCANNER& scan, INTEGER& i1, INTEGER& i2)
{
  if (! scan.TestFor (sLParen, sComma)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sInteger, sRParen)) return FALSE;
  i1 = (INTEGER) scan.GetLastInt ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sInteger, sRParen)) return FALSE;
  i2 = (INTEGER) scan.GetLastInt ();
  scan.NextSymbol ();
  if (! scan.TestFor (sRParen, sSemicolon)) return FALSE;
  scan.NextSymbol ();
  return TRUE;
}

BOOLEAN GetCorrectRealPair (SCANNER& scan, REAL& r1, REAL& r2)
{
  if (! scan.TestFor (sLParen, sComma)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  r1 = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  r2 = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sRParen, sSemicolon)) return FALSE;
  scan.NextSymbol ();
  return TRUE;
}


BOOLEAN GetCorrectString (SCANNER& scan, STRING& s)
{
  if (scan.TestFor (sString, sComma)) {
    s = scan.GetLastString ();
    scan.NextSymbol ();
    return TRUE;
  }
  else
    return FALSE;
}

BOOLEAN GetCorrectVector (SCANNER& scan, VECTOR& v)
{
  REAL r1, r2, r3;

  if (! scan.TestFor (sLParen, sComma)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  r1 = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  r2 = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sComma, sRParen)) return FALSE;
  scan.NextSymbol ();
  if (! scan.TestFor (sReal, sRParen)) return FALSE;
  r3 = scan.GetLastReal ();
  scan.NextSymbol ();
  if (! scan.TestFor (sRParen, sSemicolon)) return FALSE;
  scan.NextSymbol ();
  v = VECTOR (r1,r2,r3);
  return TRUE;
}

//-- LISTREC --------------------------------------------------------------

// Die LISTREC-Struktur bildet eine Listenebene ab, und enthaelt Daten darueber,
// mit welchen Zeichen die Liste gebildet wird, ob schon Listenelemente
// geschrieben wurden usw.

class LISTREC {
public:
  LISTREC* Next;        // Rest des Listenstacks
  SYMBOL   ListStart;   // Startsymbol fuer die Liste
  SYMBOL   ListSep;     // Trennsymbol fuer die Liste
  SYMBOL   ListEnd;     // Endsymbol fuer die Liste
  BOOLEAN  Force;       // Sollen Start- und Endsymbol auf jeden Fall geschr.
                        // werden
  BOOLEAN  Started;     // Wurden schon Listenelemente geschrieben
  BOOLEAN  Pending;     // Muss ein Trennsymbol vor dem naechsten Element 
                        // ausgegeben werden?
  BOOLEAN  Indent;      // Wird die Liste eingerueckt?

  LISTREC (LISTREC*, SYMBOL, SYMBOL, SYMBOL, BOOLEAN, BOOLEAN);
};

// Zugehoeriger Konstruktor:

LISTREC::LISTREC (LISTREC* next, SYMBOL lstart, SYMBOL lsep, SYMBOL lend,
                  BOOLEAN force, BOOLEAN indent)
{
  Next = next;
  ListStart = lstart;
  ListSep = lsep;
  ListEnd = lend;
  Force = force;
  Started = FALSE;
  Pending = TRUE;
  Indent = indent;
}

//-- BACKUP --------------------------------------------------------------
//
// Siehe auch "sim3DFile.h"

BACKUP::BACKUP (const STRING& name)
{
  File = new ofstream (name);
  ListStack = NULL;
  FileOpen = TRUE;
  FileName = name;
  IndentLevel = 0;
  ErrorOccured = File->fail ();
}

BACKUP::~BACKUP ()
{
  delete File;
  while (ListStack != NULL) {
    LISTREC* help = ListStack;
    ListStack = ListStack->Next;
    delete help;
  }
}

void BACKUP::WriteSymbol (SYMBOL sym)
{
  PrepareSymbol ();
  if (sym != sNull)
    *File << SymbolRepr (sym);
}

void BACKUP::WriteIdent (const STRING& id)
{
  PrepareSymbol ();
  *File << id << ' ';
}

void BACKUP::WriteString (const STRING& s)
{
  PrepareSymbol ();
  *File << '"' << s << '"';
}

void BACKUP::WriteReal (REAL r)
{
  PrepareSymbol ();
  *File << r;
}

void BACKUP::WriteInteger (INTEGER i)
{
  PrepareSymbol ();
  *File << i;
}

void BACKUP::WriteVector (const VECTOR& v)
{
  PrepareSymbol ();
  *File << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}

void BACKUP::WriteLine ()
{
  *File << '\n';
  for (INTEGER i=0;i<IndentLevel;i++)   *File << "  ";
}

void BACKUP::WriteIdRealNext (const STRING& id, REAL r)
{
  WriteIdent (id);
  WriteReal (r);
  ListNext ();
}

void BACKUP::WriteIdIntNext (const STRING& id, INTEGER i)
{
  WriteIdent (id);
  WriteInteger (i);
  ListNext ();
}

void BACKUP::WriteIdStrNext (const STRING& id, const STRING& s)
{
  WriteIdent (id);
  WriteString (s);
  ListNext ();
}

void BACKUP::WriteComment (const STRING& comment)
{
  *File << "// " << comment;
  WriteLine ();
}


void BACKUP::StartList (SYMBOL lstart, SYMBOL lsep, SYMBOL lend, BOOLEAN force)
{
  LISTREC* listdescr
    = new LISTREC (ListStack, lstart, lsep, lend, force, FALSE);
  ListStack = listdescr;
}

void BACKUP::StartListI (SYMBOL lstart, SYMBOL lsep, SYMBOL lend, BOOLEAN force)
{
  LISTREC* listdescr
    = new LISTREC (ListStack, lstart, lsep, lend, force, TRUE);
  ListStack = listdescr;
  IndentLevel++;
}

void BACKUP::ListNext ()
{
  if (ListStack != NULL)
    ListStack->Pending = TRUE;  // Jetzt muesste ein Trennsymbol geschrieben
                                // werden (wird aber noch nicht, weil es ja
                                // auch das Endsymbol sein koennte)
}

void BACKUP::EndList ()
{
  if (ListStack != NULL) {
    LISTREC* ld = ListStack;
    ListStack = ListStack->Next;
    if (ld->Indent) IndentLevel--;
    if (!(ld->Started)) {               // Keine Listenelement geschrieben?
      if (ld->Force) {                  // Aber die Klammern sollen ausgegeben
        WriteSymbol (ld->ListStart);    // werden?
        WriteSymbol (ld->ListEnd);
      }
    }
    else {                              // sonst:
      if (ld->Indent) WriteLine ();     // Bei Indent noch ein Zeilenvorschub
      WriteSymbol (ld->ListEnd);        // und Listenende ausgeben
    }
    delete ld;
  }
}

void BACKUP::PrepareSymbol ()
// Diese Routine wird fuer jedes Symbol aufgerufen, das ausgegeben wird
{
  if (ListStack != NULL) {              // Wenn eine Liste geschrieben wird
    LISTREC* ld = ListStack;
    ListStack = ListStack->Next;        // Indem 'ListStack' auf die Vorgaenger-
                                        // Liste gesetzt wird, werden verschach-
                                        // telte Listen behandelt (Rekursion).
                                        // Schliesslich ruft WriteSymbol wieder
                                        // PrepareSymbol auf
    if (ld->Pending) {                  // Es muss noch ein Listensymbol geschr.
                                        // werden
      if (ld->Started)                  // Wenn die Liste schon begonnen wurde
        WriteSymbol (ld->ListSep);      // einen Trenner,
      else {                            // sonst
        WriteSymbol (ld->ListStart);    // den Listenstart.
        ld->Started = TRUE;
      }
      if (ld->Indent) WriteLine ();     // Einrueckung beachten
      ld->Pending = FALSE;              // Listensymbol ist geschrieben!
    }
    ListStack = ld;                     // Stack wieder herstellen.
  }
}

