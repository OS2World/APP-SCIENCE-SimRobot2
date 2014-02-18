/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  sim3DBasics.h                                         ****
 **** Inhalt: Die Grundklassen der Simulationshierarchie werden     ****
 ****         hier beschrieben.                                     ****
 ****                                                               ****
 ***********************************************************************/

#ifndef __SIM3DBASICS_H_
#define __SIM3DBASICS_H_

#include "Common.h"
#include "sim3DFile.h"
#include "Vector.h"
#include "BasicGraphics.h"
#include "sim3DDynArray.h"
#include "Geometry.h"
#include "SensorClasses.h"
#include "sim3DPortDef.h"
#include "ZBuffer.h"

#define NOELEMENT NULL

const S3DElement = 0;

class SIM3DGroup;
class SIM3DWorld;

// Diese Klasse dient nur dazu, um Benutzerdaten automatisch mit dem Objekt
// zu loeschen

class AUTODEL
{
public:
  virtual ~AUTODEL () {}
};

// SIM3DElement ist das Grundelement der Simulation.
// Es definiert alle Eigenschaften, die allen Objekten der Simulation gemein
// sind, und einige, die eigentlich nur fuer einige Objekte gelten (aber wo
// es zu kompliziert waere, immer zu unterscheiden).

class SIM3DElement
{
public:
  SIM3DElement* Next;           //Bezug auf naechsten Zweig der Hierarchie
  SIM3DGroup* Owner;            //Bezug auf die naechsthoehere Ebene der 
                                // Hierarchie
  SIM3DWorld* World;            //Zeiger auf die Welt mit Zusatzinfo
  BOOLEAN     Registered;       //Ist Objekt Teil der Szene (oder nur Makro)?
  STRING      ElementName;      //Name des Teilelements
  STRING      Remark;           //Platz fuer eigene Daten
  INTEGER     DetailLevel;      //Bis zu welchem Grad soll das Element
                                // dargestellt werden
  COLOR       Color;            //In welcher Farbe soll das Objekt gezeichnet
                                // werden
  BOUNDINGBOX BoundingBox;      //Das umgebende Rechteck des Elements
  MOVEMATRIX  Transform;        //Die Abbildung, die auf das Element angewendet
                                // wird
  AUTODEL*    CustomData;       //(eigentlich) zur freien Verwendung, wird je-
                                // doch von der grafischen Oberflaeche benutzt.

  // Konstruktor, es wird der Name des neuen Objekts uebergeben
           SIM3DElement (const STRING&);

  // Kopierkonstruktor
           SIM3DElement (const SIM3DElement&);

  // Destruktor
  virtual ~SIM3DElement ();

  // Liefert den Namen des Objekts
          STRING GetName () { return ElementName; };

  // Liefert die Klasse des Objekts als String. Dieser Name wird auch fuer das
  // Abspeichern einer Definitionsdatei benutzt. Deshalb muss er mit dem Namen
  // uebereinstimmen, der in "sim3DReader.cc" benutzt wird.
  virtual STRING GetClass ()
          {return "ELEMENT"; }

  // Test auf Klassenzugehoerigkeit. Wird mit den numerischen Konstanten 
  // S3DKlassenname benutzt und testet, ob das Objekt von der angegebenen 
  // Klasse abstammt.
  virtual BOOLEAN     IsA (INTEGER typ)
          {return typ == S3DElement;}

  // Liefert einen Zeiger auf eine Kopie des Objekts
  virtual SIM3DElement* Copy() = 0;

  // Liefert den Namen des Objekts relativ zu einem hoeheren Knoten im Baum
  // (Pfadname, die Knoten werden durch Punkt getrennt)
  virtual STRING      GetNameRelTo (SIM3DElement* obj);

  // Liefert die Position des Objekts relativ zu einem hoeheren Knoten im Baum
  virtual MOVEMATRIX  GetPosRelTo (SIM3DElement* obj);

  // Liefert des n-te Unterobjekt des Knotens
  virtual SIM3DElement* SubObject (INTEGER n);

  // Wieviele Unterobjekte hat dieser Knoten?
  virtual INTEGER       SubObjectCount ();

  // Liefert das erste Unterobjekt dieses Knotens
  virtual SIM3DElement* FirstSubObject ();

  // Liefert das dem angegebenen Unterobjekt folgende Unterobjekt des Knotens
  // Wenn kein gueltiges Unterobjekt angegeben wird, ist das Ergebnis nicht
  // definiert
  virtual SIM3DElement* NextSubObject (SIM3DElement* obj);

  // Liefert einen Zeiger auf die Aktorport-Definitionen dieser Klasse
  // Siehe auch "sim3DPortDef.h"
  virtual APORTDEF*   GetAPortDef () { return NULL; };

  // Liefert einen Zeiger auf die Sensorport-Definitionen dieser Klasse
  // Siehe auch "sim3DPortDef.h"
  virtual SPORTDEF*   GetSPortDef () { return NULL; };

  // Bestimme den Aktorport zu einem Pfad
  virtual ACTORPORT   GetActorPort (const STRING& path);

  // Bestimme den Sensorport zu einem Pfad
  virtual SENSORPORT  GetSensorPort (const STRING& path);

  // Setze das Aktorkommando fuer eine Aktorportnummer des Objekts
          void        SetActorValue (INTEGER portnr, REAL value);

  // Lese den Sensorwert fuer eine Sensorportnummer des Objekts
          void        GetSensorValue (INTEGER portnr, PSHORTREAL value);

  // Wieviele Dimensionen hat der Sensorwert fuer die angeg. Sensorportnummer
          INTEGER     GetSensorDim (INTEGER portnr);

  // Welche Ausdehnung hat der Sensorwert in der angeg. Dimension
          INTEGER     GetSensorDimSize (INTEGER portnr, INTEGER dim);

  // Wieviele Aktorports sind fuer das Objekt insgesamt definiert
          INTEGER     ActorPortCount ();

  // Wieviele Sensorports sind fuer das Objekt insgesamt definiert
          INTEGER     SensorPortCount ();

  // Welchen Namen hat der n-te Aktorport des Objekts
          STRING      ActorPortName (INTEGER n);

  // Welchen Namen hat der n-te Sensorport des Objekts
          STRING      SensorPortName (INTEGER n);

  // Lese das Attribut "attr' aus einer Definitionsdatei (angesprochen durch
  // 'scan').
  virtual void        ReadAttr (const STRING& attr, SCANNER& scan);
  
  // Schreibe alle noetigen Attribute in die Definitionsdatei, die durch
  // 'backup' angesprochen wird.
  virtual void        WriteAttrs (BACKUP& backup);

  // Schreibe das gesamte Objekt in die Definitionsdatei von 'backup'
  virtual void        WriteObj (BACKUP& backup);

  // Diese Methode wird aufgerufen, wenn das Objekt in eine Szene (mit gueltiger
  // Wurzel) eingefuegt wird.
  virtual void        RegisterAdd ();

  // Diese Methode wird aufgerufen, wenn das Objekt aus einem Baum geloescht
  // wird.
  virtual void        RegisterRemove ();

  // Liefert die Position des Objekts bezueglich seines Eigners und unter
  // Beruecksichtigung zusaetzlicher Modifizierer wie z.B. bei Gelenken
  virtual MOVEMATRIX  GetLocalTransform ();

  // Fuehre einen Zeitschritt fuer das Objekt aus.
  // M ist die Lage des absoluten Bezugssystems fuer das Objekt. Zu diesem muss
  // noch die lokale Position zugerechnet werden.
  // Die Ausfuehrung eines Zeitschrittes beinhaltet bei Gelenken und aehnlichem
  // den Versuch der Bewegung des Gelenks (durch MakeStep).
  // Bei Objekten, die ihre absolute Position in der Szene brauchen, wird
  // diese an dieser Stelle aus M und Transform abgeleitet.
  // Als letzter Schritt wird MakeTimeStep fuer alle Unterobjekte (wenn
  // vorhanden) mit neuem Bezugssystem durchgefuehrt. 
  virtual void        MakeTimeStep (const MOVEMATRIX& M) {};

  // Berechnung der Lage des Objekts bei Anwendung der gueltigen Aktorkommandos 
  // M ist die Lage des absolute Bezugssystems fuer das Objekt (ohne Anwendung
  // der MOVEMATRIX LocalTransform() ). Die alte Position wird fuer Koerper
  // zwischengespeichert (um die Berechnung der alten Position bei Misserfolg
  // zu sparen) und kann mit StepBack wiederhergestellt werden.
  virtual void        MakeStep (const MOVEMATRIX& M) {};

  // Wiederherstellung der letzten Position, die mit MakeStep zwischen-
  // gespeichert wurde.
  virtual void        StepBack () {};

  // Berechne alle Sensoren des Objekts neu (Kann durch ein Flag in SIM3DWorld
  // bis zur wirklichen Beschaffung verzoegert werden).
  virtual void        ComputeSensors () {};

  // Berechnet, ob sich das angegebene Objekt (mit allen Unterobjekten)
  // mit diesem Objekt (mit allen Unterobjekten) ueberschneidet.
  // Per Default liegt fuer ein SIM3DElement keine Ueberschneidung vor.
  virtual BOOLEAN     DoesIntersect (SIM3DElement* obj) { return FALSE; };

  // Berechne ein neues umgebendes Rechteck fuer dieses Objekt 
  virtual void        MakeNewBoundingBox () {};

  // Melde das neu erzeugte umgebende Rechteck bei allen Eignern an.
  virtual void        RegisterBoundingBox () {};

  // Liefert die Zeichenfarbe fuer dieses Objekt
  virtual COLOR       GetColor ();

  // Zeichne ein (einzelnes) Objekt auf den angegebenen Graphikport 'gp'.
  // 'detail' gibt eine Grenze fuer das Zeichnen von Details an.
  // 'scale' ist die Standardgroesse fuer die Anzeige von Objekten ohne
  // Ausdehnung.
  // Mit Aufruf von 'GRAPHICPORT::NewPicture' und 'GRAPHICPORT::EndOfPicture'
          void        DoDraw (GRAPHICPORT* gp, INTEGER detail, REAL scale);

  // Wie 'DoDraw', jedoch ohne 'NewPicture' und 'EndOfPicture', dagegen
  // mit Angabe des Bezugsystems. Der Detaillevel wird hier abgefragt.
  // Wird von 'DoDraw' benutzt.
          void        Draw2 (GRAPHICPORT* gp, const MOVEMATRIX& m, INTEGER detail, 
                             REAL scale);

  // 'Draw' wird von 'Draw2' nach dem Test auf Detaillevel aufgerufen und muss
  // von abgeleiteten Klassen definiert werden.
  virtual void        Draw (GRAPHICPORT* gp, const MOVEMATRIX& m, INTEGER detail, 
                            REAL scale) = 0;

  // Berechne den Schnitt eines Sehstrahls 'ray', der endlich oder unendlich
  // ('infinite') sein kann, mit diesem Objekt und liefert als Ergebnis:
  // Wo liegt der Schnitt auf dem Strahl ('where')
  // Welche Flaeche ('facenr') von welchem Objekt ('obj') wurde getroffen
  // Liefert FALSE, wenn kein Objekt getroffen wurde
  virtual BOOLEAN     CalcRayIntersection (const RAY& ray, BOOLEAN infinite, 
                          REAL& where, SIM3DElement*& obj, INTEGER& facenr);

  // Berechne die Farbe der Flaeche n des angesprochenen Objekts am Punkt p.
  // Wird der Sehstrahl v angegeben, wird bereits die Beleuchtungssituation
  // mit beruecksichtigt. Werden die Textur-Koordinaten angegeben, muessen
  // sie nicht neu berechnet werden. Ersteres ist die Schnittstelle fuer den
  // Z-Buffer, der die Koordinaten bereits selbst berechnet.
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, REAL TextureX, REAL TextureY);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p, const VECTOR& v);
  virtual SENSCLASSVECTOR& GetColorOfFaceAt (INTEGER n, const VECTOR& p, const VECTOR& v,
                                             REAL TextureX, REAL TextureY);

  // Zeichne den Koerper des Objekts in einen z-Buffer 
  //(fuer Kamerabild-Berechnung)
  virtual void        DrawBodyOnZBuffer (ZBUFFER&);
};

const S3DGroup = 1;

// SIM3DGroup ist die Grundklasse fuer alle Objekte, die mehrere Unterobjekte
// enthalten koennen.

class SIM3DGroup : public SIM3DElement
{
private:
  SIM3DElement* Last;   // Zeiger auf eine Ringliste von Unterobjekten
  STRINGARRAY Paths;    // Eine Liste von Pfad-Aliasen innerhalb des Objekts
public:

  // Konstruktor, Kopierkonstruktor und Destruktor
           SIM3DGroup (const STRING&);
           SIM3DGroup (const SIM3DGroup&);
  virtual ~SIM3DGroup ();

  // Fuege ein neues Unterobjekt ein
          void        AddObject (SIM3DElement* obj);

  // Loesche ein Unterobjekt, Rueckgabe: konnte Objekt geloescht werden?
          BOOLEAN     RemoveObject (SIM3DElement* obj);

  // Bedeutung wie oben, wird aber an alle Unterobjekte weitergegeben.
  virtual void        RegisterAdd ();

  // Bedeutung wie oben, wird aber an alle Unterobjekte weitergegeben.
  virtual void        RegisterRemove ();

  // Bedeutung wie oben.
  virtual SIM3DElement* SubObject (INTEGER n);
  virtual INTEGER       SubObjectCount ();
  virtual SIM3DElement* FirstSubObject ();
  virtual SIM3DElement* NextSubObject (SIM3DElement* obj);
  virtual STRING      GetClass ()
          {return "GROUP"; }
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DGroup) || SIM3DElement::IsA (typ); }
  virtual SIM3DElement* Copy()
          {return new SIM3DGroup (*this);}

  // Definiere ein neues Pfad-Alias
          void        AddPath (const STRING&, const STRING&);

  // Expandiere den angegebenen Pfad
          STRING      ExpandPath (const STRING&);

  // Bedeutung wie oben, nur dass der Pfad auch in Unterobjekte hineinreichen
  // kann
  virtual ACTORPORT   GetActorPort (const STRING& path);
  virtual SENSORPORT  GetSensorPort (const STRING& path);

  // Bedeutung wie oben, mit zusaetzlichen Attributen fuer SIM3DGroup
  virtual void        ReadAttr (const STRING& attr, SCANNER& scan);
  virtual void        WriteAttrs (BACKUP& backup);
  virtual void        WriteObj (BACKUP& backup);
  // WriteSubObjects wird von WriteObj aufgerufen, um alle Unterobjekte zu
  // speichern.
          void        WriteSubObjects (BACKUP& backup);

  // Bedeutung wie oben - alle Aufrufe werden auch an Unterobjekte
  // weitergereicht
  virtual void        MakeTimeStep (const MOVEMATRIX& m);
  virtual void        MakeStep (const MOVEMATRIX& m);
  virtual void        StepBack ();
  virtual void        ComputeSensors ();
  virtual BOOLEAN     DoesIntersect (SIM3DElement* obj);
  virtual void        MakeNewBoundingBox ();
  virtual void        RegisterBoundingBox ();
  virtual void        Draw (GRAPHICPORT* gp, const MOVEMATRIX& m, INTEGER detail,
                            REAL scale);
  virtual BOOLEAN     CalcRayIntersection (const RAY& ray, BOOLEAN infinite, 
                          REAL& where, SIM3DElement*& obj, INTEGER& facenr);
  virtual void        DrawBodyOnZBuffer (ZBUFFER&);
};

const
  S3DGlobal = 2;

// Weil ein SIM3DElement seine absolute Position innerhalb der Szene nicht
// kennt, speichert SIM3DGlobal diese ab, wenn es sie in MakeTimeStep
// berechnet. Ausserdem wird die absolute Position bei einem RegisterAdd
// berechnet

class SIM3DGlobal : public SIM3DElement
{
public:
  MOVEMATRIX GlobalPos;  // Die globale Position und Lage innerhalb der Szene

  SIM3DGlobal (const STRING& Name) : SIM3DElement (Name) {};
  virtual STRING      GetClass ()
          {return "GLOBAL"; };
  virtual BOOLEAN     IsA (INTEGER typ)
          {return (typ == S3DGlobal) || SIM3DElement::IsA (typ); };
  virtual SIM3DElement* Copy() = 0;

  virtual void        MakeTimeStep (const MOVEMATRIX&);
  virtual void        RegisterAdd ();
};

#endif
