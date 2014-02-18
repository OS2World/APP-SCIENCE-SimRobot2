/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DReader.cc                                        ****
 **** Inhalt: Funktionen zum Einlesen von Objekten vom Scanner      ****
 ****                                                               ****
 ***********************************************************************/

#include "sim3DReader.h"

#include "sim3DMoveable.h"
#include "sim3DVehicle.h"
#include "sim3DBody.h"
#include "sim3DEmitter.h"
#include "sim3DWorld.h"
#include "sim3DSensors.h"

// Fuer eine genaue Beschreibung der Syntax der Beschreibungssprache, siehe
// "SimRobotSyntax.doc"

// Lesen eines Objektes vom Scanner:
SIM3DElement* ReadObject (SCANNER& scan, SIM3DWorld* World)
{
  if (scan.Symbol != sIdent) {
    scan.SyntaxError (sim3DExpectObjectType, sSemicolon);
    return NOELEMENT;
  }

  STRING type = scan.GetLastString ();

  scan.NextSymbol ();

  STRING name;
  if (scan.Symbol != sString)
    scan.SyntaxError (sim3DExpectObjectName, sLParen);
  else {
    name = scan.GetLastString ();
    scan.NextSymbol ();
  }

  SIM3DElement* Obj;

  if (!World)
	 if (type == "WORLD") {
		Obj = new SIM3DWorld(name);
		World = (SIM3DWorld*) Obj;
	 }
	 else
	 {
		scan.SyntaxError (sim3DExpectWorld, sString);
		return NOELEMENT;
	 }

  else if (type == "MACRO") {           // Makro statt Elementtyp
	 Obj = World->FindMacro (name);
	 if (Obj == NOELEMENT)
		scan.SyntaxError (sim3DUnknownMacro1, name);
	 else
		Obj = Obj->Copy();
	 if (scan.Symbol == sIdent) {
		if (! (scan.GetLastString () == "AS"))
		  scan.SyntaxError (sim3DExpectAs, sLParen);
		else {
		  scan.NextSymbol ();
		  if (scan.TestFor (sString, sLParen)) {
			 if (Obj != NOELEMENT) {
            Obj->ElementName = scan.GetLastString ();
          }
          scan.NextSymbol ();
        }
      }
    }
    if (Obj == NOELEMENT) return Obj;
  }

  // Hier folgt die Liste mit allen definierten Simulationsobjekt-Klassen.
  // Die Namen muessen mit den Namen uebereinstimmen, die von der jeweiligen
  // GetClass-Methode geliefert werden. 
  // Dieser Schritt ist notwendig, weil die definierten Klassen nirgends
  // aufgelistet sind.
  // Die Form eines Eintrags ist den folgenden Eintraegen zu entnehmen:

  else if (type == "GROUP")
    Obj = new SIM3DGroup (name);

  else if (type == "TURNABLE")
    Obj = new SIM3DTurnable (name);

  else if (type == "EXTENDIBLE")
    Obj = new SIM3DExtendible (name);

  else if (type == "VEHICLE")
    Obj = new SIM3DVehicle (name);

  else if (type == "MANOEUVRABLE")
    Obj = new SIM3DManoeuvrable (name);

  else if (type == "POLYEDER")
    Obj = new SIM3DPolyeder (name);

  else if (type == "RADIALEMITTER")
    Obj = new SIM3DRadialEmitter (name);

  else if (type == "SPOTEMITTER")
    Obj = new SIM3DSpotEmitter (name);

  else if (type == "CAMERA")
    Obj = new SIM3DCamera (name);

  else if (type == "FACETTE")
    Obj = new SIM3DFacette (name);

  else if (type == "WHISKER")
    Obj = new SIM3DWhisker (name);

  else if (type == "TACTILE")
    Obj = new SIM3DTactile (name);

  else if (type == "RECEPTOR")
    Obj = new SIM3DReceptor (name);

  else if (type == "WHISKERFIELD")
    Obj = new SIM3DWhiskerField (name);

  else if (type == "ULTRASONIC")
    Obj = new SIM3DUltraSonic (name);

  else if (type == "COMPASS")
    Obj = new SIM3DCompass (name);

  // Hier weitere Objekttypen einfuegen...

  else {
    scan.SyntaxError (sim3DUnknownObjectType1, type, sSemicolon);
    return NOELEMENT;
  }

  Obj->World = World;

  if (scan.Symbol == sLParen) {         // Lesen der Attribute
    do {
      scan.NextSymbol ();
      if (scan.Symbol == sRParen) break;
      if (scan.TestFor (sIdent, sSemicolon)) {
        STRING attr = scan.GetLastString ();
        scan.NextSymbol ();
        Obj->ReadAttr (attr, scan);
      }
    } while (scan.Symbol == sSemicolon);
    scan.Expect (sRParen, sSemicolon);
  }

  if (Obj->IsA (S3DGroup)) {            // Lesen von Unterobjekten
    if (scan.Symbol == sLBracket) {
      do {
        scan.NextSymbol ();
        if (scan.Symbol == sRBracket) break;
        SIM3DElement* iObj = ReadObject (scan, World);
        if (iObj != NOELEMENT)
         ((SIM3DGroup*)Obj)->AddObject (iObj);
      } while (scan.Symbol == sSemicolon);
      scan.Expect (sRBracket, sSemicolon);
    }
  }

  return Obj;
}
