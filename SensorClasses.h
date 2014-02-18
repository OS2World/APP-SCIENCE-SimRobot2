/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  SensorClasses.h                                       ****
 **** Inhalt: Der Datentyp SURFACEDESCR zur Beschreibung von Ober-  ****
 ****         flaecheneigenschaften wird hier definiert.            ****
 ****                                                               ****
 ***********************************************************************/

#ifndef _SENSORCLASSES_H_
#define _SENSORCLASSES_H_

#include "sim3DFile.h"
#include "sim3DDynArray.h"

// Das Lichtmodell von SimRobot sieht die Unterscheidbarkeit von verschiedenen
// Sensorklassen vor. Diese sind am ehesten als verschiedene Wellenlaengen-
// bereiche zu interpretieren. Diese sind einfach von 0 an durchnummeriert.
// Eine moegliche Aufteilung waere Rot, Gruen, Blau, aber es sind beliebig
// viele Klassen zugelassen, die zum Beispiel auch Geraeuschquellen darstellen
// koennen.

// SENSCLASSVECTOR bildet die Oberflaechen-Eigenschaft in allen Sensorklassen
// ab. Die Anzahl der Sensorklassen wird nicht festgelegt: Neben der Groesse
// des Vektors gibt man ein Default-Element an, dass fuer alle Indices
// ausserhalb der festgelegten Groesse geliefert wird.
// Als Methoden definiert wurden: zwei Arten von Indizierung (als Wert und als
// Refernz), elementweise Addition und Multiplikation, skalare Multiplikation,
// Zuweisung und Lesen und Schreiben von/zu einer Definitionsdatei.

class SENSCLASSVECTOR
{
  INTEGER Size;
  SHORTREAL Default;
  SHORTREAL* Data;
public:
  SENSCLASSVECTOR (INTEGER,SHORTREAL);
  SENSCLASSVECTOR (const SENSCLASSVECTOR&);
  ~SENSCLASSVECTOR ();

  SHORTREAL&      operator [] (INTEGER);
  SHORTREAL       Index       (INTEGER) const;
  void            Set         (INTEGER,SHORTREAL);
  SENSCLASSVECTOR operator *  (const REAL) const;
  SENSCLASSVECTOR operator +  (const SENSCLASSVECTOR&) const;
  SENSCLASSVECTOR operator *  (const SENSCLASSVECTOR&) const;
  SENSCLASSVECTOR& operator *=  (const SENSCLASSVECTOR&);
  void            MulAndAdd(const SENSCLASSVECTOR& scv,REAL factor);
  void            operator =  (const SENSCLASSVECTOR&);
  void            ReadVector  (SCANNER&);
  void            WriteVector (BACKUP&);
};

// TEXTUREDESCR liefert Texturen

class TEXTUREDESCR
{
  TEXTUREDESCR* Next;
  friend class SIM3DWorld;
public:
  STRING Name;
  
           TEXTUREDESCR (const STRING& name);
  virtual ~TEXTUREDESCR ();
  virtual void      WriteProperties (BACKUP& backup);
  virtual void      GetSize (INTEGER& planes, REAL& width, REAL& height);
  virtual SHORTREAL GetValueAt (INTEGER plane, REAL x, REAL y);
};


TEXTUREDESCR* TextureLoader (SCANNER& scan);


// SURFACEDESCR enthaelt eine vollstaendige Materialdefinition.
// Eine Materialdefinition enthaelt neben den Eigenschaftsvektoren einen
// Namen, der neben der Groesse der Vektoren im Konstruktor angegeben wird.
// Methoden: Zugriffsoperationen und Schreiben/Lesen zu/von Definitionsdateien.

class SURFACEDESCR
{
  SURFACEDESCR* Next;
  SENSCLASSVECTOR scvTemp;
  friend class SIM3DWorld;
public:
  STRING Name;
  SENSCLASSVECTOR Color;
  SENSCLASSVECTOR TextureBgColor;
  SENSCLASSVECTOR TextureFgColor;
  INTEGERARRAY    TexturePlaneNr;

  TEXTUREDESCR*   Texture;
  REAL            Width, Height, Angle;
  BOOLEAN         AutoSize;
  BOOLEAN         WrapAround;
  BOOLEAN         NotShading;

  SURFACEDESCR (const STRING&);

  void         ReadAttr (STRING& attr, SCANNER& scan, SIM3DWorld* World);
  void         ReadProperties  (SCANNER& scan, SIM3DWorld* World);
  void         WriteProperties (BACKUP&);

  // Enthaelt die Oberflaechenbeschreibung eine Textur?
  BOOLEAN      HasTexture ();
  BOOLEAN      HasTexture (INTEGER cl);

  // Liefere die Farbe an einer bestimmten Stelle der Textur
  SENSCLASSVECTOR& GetColorAt (REAL x, REAL y);

  // GetSizeOrRatio: Liefere Groesse(nverhaeltnisse) der Textur,
  //      x == 0, y == 0   -->  An Groesse der Flaeche anpassen
  //      x == 0, y != 0   -->  y = Hoehe, Breite an Flaeche angepassen
  //      x != 0, y == 0   -->  x = Breite, Hoehe an Flaeche angepassen
  //      x != 0, y != 0,
  //             autosize  -->  x und y geben Seitenverhaeltniss an,
  //                            Groesse an Flaeche angepassen
  //            !autosize  -->  x = Breite, y = Hoehe
  void         GetSizeOrRatio (REAL& x, REAL& y, BOOLEAN& autosize);
};


#endif
