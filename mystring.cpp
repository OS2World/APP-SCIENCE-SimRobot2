/***********************************************************************
 ****                                                               ****
 **** SimRobot                                                      ****
 **** Universitaet Bremen, Fachbereich 3 - Informatik               ****
 **** Zentrum fuer Kognitionswissenschaften                         ****
 **** Autor:  Uwe Siems                                             ****
 ****                                                               ****
 **** Datei:  mystring.cc                                           ****
 **** Inhalt: Implementation des String-Typs                        ****
 ****                                                               ****
 ***********************************************************************/

#include "mystring.h"
#include <string.h>


STRING::STRING (const char *p)
{
  if ( p && *p ) {
    Len = strlen(p);
    String = new char [Len+1];
    (void) strncpy (String, p, Len+1);
  }
  else {
    Len = 0;
    String = 0;
  }
}

STRING::STRING (const STRING& s)
{
  Len = s.Len;
  if (Len > 0) {
    String = new char [Len+1];
    (void) strncpy (String, s.String, Len+1);
  }
  else 
    String = NULL;
}

STRING::~STRING()
{
  if (Len > 0)
    delete [] String;
}

BOOLEAN STRING::operator == (const STRING& s) const
{
  return ( Len == s.Len && ((Len == 0) || (!strcmp (String, s.String))) );
}

BOOLEAN STRING::operator == (const char* s) const
{
  if (Len == 0)
    return (s == NULL) || (*s == '\0');
  else if (s == NULL)
    return FALSE;
  else
    return (! strcmp (String, s));
}

STRING& STRING::operator = (const STRING& s)
{
  if (this != &s ) {
    if (Len != s.Len ) {
      if (Len > 0) delete String;
      Len = s.Len;
      if (Len > 0) 
        String = new char [Len+1];
      else
        String = NULL;
    }
    if (Len > 0)
      strcpy (String, s.String);
  }
  return *this;
}

STRING STRING::operator + (const STRING& s) const
{
  if (Len > 0 && s.Len > 0) {
    STRING result;
    result.Len = Len + s.Len;
    result.String = new char [result.Len+1];
    strcpy (result.String, String);
    strcat (result.String, s.String);
    return result;
  }
  else if (Len > 0)
    return *this;
  else
    return s;
}

STRING STRING::operator * (INTEGER n) const
{
  STRING result;

  if (Len > 0 && n > 0) {
    result.Len = Len * n;
    result.String = new char [result.Len+1];
    for (INTEGER i=0;i<n;i++)
      strcpy (&result.String[i*Len], String);
  }
  return result;
}

STRING::operator const char *() const
{
  if (Len > 0)
    return String;
  else
    return "";
}

INTEGER STRING::Pos (char c) const
{
  INTEGER res = 0;

  while (res < Len && c != String [res]) res++;
  return res;
}

STRING STRING::First (INTEGER l) const
{
  if (l > Len) l = Len;

  STRING result ("");

  if (l > 0) {
    result.Len = l;
    result.String = new char [Len+1];
    strncpy (result.String, String, l);
    result.String [l] = 0;
  }

  return result;
}

STRING STRING::ButFirst (INTEGER f) const
{
  if (f < 0) f=0;

  INTEGER l = Len - f;

  STRING result ("");

  if (l > 0) {
    result.Len = l;
    result.String = new char [Len+1];
    strncpy (result.String, &String[f], l);
    result.String [l] = 0;
  }

  return result;
}
