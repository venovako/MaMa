/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// ***  Autorska prava (C) 2005 Vedran Novakovic <venovako@gmail.com>  *** //
//                                                                         //
//    MaMa  specifikacija                                                  //
//    MaMa  interpreter (x0x verzije)                                      //
//    MaMaC kompajler   (x0x verzije)                                      //
//    Biblioteka Standardnih makrOa                                        //
//                                                                         //
//                                                                         //
// ***  Autorska prava (C) 2005 Marko Doko <mdoko.math@gmail.com>  ***     //
//                                                                         //
//    MaMa  specifikacija                                                  //
//    MaMut graficko korisnicko sucelje                                    //
//                                                                         //
//                                                                         //
// MaMa softverski paket, u cjelini i svim dijelovima, vlasnistvo je       //
// autora.  Sva su autorska prava pridrzana.                               //
// Paket i svi njegovi djelovi ponudjeni su od autora u nadi da ce biti    //
// korisni, ali bez IKAKVOG DALJNJEG JAMSTVA!                              //
//                                                                         //
// Koristenje, kopiranje i daljnja distribucija paketa ili bilo kojeg      //
// njegova dijela, osim ako nije naglaseno drugacije, dozvoljeno je i      //
// u izvornom i u binarnom obliku, sa ili bez izmjena, uz uvjet da         //
// ovaj dokument nije modificiran i da je sadrzan u svakoj distribuciji    //
// paketa ili bilo kojeg njegova dijela.                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

// stri_ng.hpp : klasa String
//

#ifndef STRI_NG_HPP
#define STRI_NG_HPP

#include <string>
#include <vector>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <cerrno>
#include <algorithm>
#include <functional>
using namespace std;

#include "platform.hpp"

namespace stri_ng {

  class String;

  typedef vector<String> StrVec;

  class String : public string {
  public:
    // HT 09
    // NL 0a
    // VT 0b
    // FF 0c
    // CR 0d
    // SP 20
    static const char *const spc;   // HT SP

    String() : string() {}
    String(const string &s, const size_type pos = 0, const size_type n = npos) : string(s,pos,n) {}
    String(const char *const s) : string(s) {}
    String(const char *const s, const size_type n) : string(s,n) {}
    String(const size_type n, const char c) : string(n,c) {}

    template <class InputIterator>
    String(InputIterator first, InputIterator last) : string(first,last) {}

    String &operator=(const string &s) {
      static_cast<string*>(this)->operator=(s);
      return *this;
    }

    String &operator=(const char *s) {
      static_cast<string*>(this)->operator=(s);
      return *this;
    }

    String &operator=(const char c) {
      static_cast<string*>(this)->operator=(c);
      return *this;
    }

    String &operator+=(const string &s) {
      static_cast<string*>(this)->operator+=(s);
      return *this;
    }

    String &operator+=(const char *s) {
      static_cast<string*>(this)->operator+=(s);
      return *this;
    }

    String &operator+=(const char c) {
      static_cast<string*>(this)->operator+=(c);
      return *this;
    }

    String &erase(const size_type pos = 0, const size_type n = npos) {
      string::erase(pos,n);
      return *this;
    }

    String substr(const size_type pos = 0, const size_type n = npos) const {
      return string::substr(pos,n);
    }

    String &trimLeft(const char *const s = spc) {
      const size_type p = find_first_not_of(s);
      if (p < size())
        erase(0,p);
      return *this;
    }

    String &trimRight(const char *const s = spc) {
      const size_type p = find_last_not_of(s) + 1;
      if (p < size())
        erase(p);
      return *this;
    }

    String &trimBoth(const char *const s = spc) {
      trimRight(s);
      trimLeft(s);
      return *this;
    }

    String &toLower() {
      transform(begin(), end(), begin(), ::tolower);
      return *this;
    }

    String &toUpper() {
      transform(begin(), end(), begin(), ::toupper);
      return *this;
    }

    // Toknizira string s delimiterima d i dijelove nadodaje na vektor t
    // Ako vektor nije prazan, c odlucuje prazni li se vektor na pocetku

    StrVec::size_type tokenize(StrVec &t, const char d, const bool c = true) const {
      if (c)
        t.clear();
      for (size_type o = 0, p = find_first_of(d);; p = find_first_of(d, o = ++p))
        if (o != p) {
          if (npos != p)
            t.push_back(substr(o, p-o));
          else {
            if (o != size())
              t.push_back(substr(o));
            return t.size();
          } // else
        } // if
    } // tokenize

    StrVec::size_type tokenize(StrVec &t, const char *const d = spc, const bool c = true) const {
      if (c)
        t.clear();
      for (size_type o = 0, p = find_first_of(d);; p = find_first_of(d, o = ++p))
        if (o != p) {
          if (npos != p)
            t.push_back(substr(o, p-o));
          else {
            if (o != size())
              t.push_back(substr(o));
            return t.size();
          } // else
        } // if
    } // tokenize

  };

  inline U4 strToU4(const char *const s, bool &ok) {
    ok = false;
    if (!s || !*s)
      return 0;
    char *e = 0;
    const U4 r = strtoul(s, &e, 0);
    if (!e || *e)
      return 0;
    ok = (r != UINT_MAX) || (errno != ERANGE);
    return r;
  }

  inline S4 strToS4(const char *const s, bool &ok) {
    ok = false;
    if (!s || !*s)
      return 0;
    char *e = 0;
    const S4 r = strtol(s, &e, 0);
    if (!e || *e)
      return 0;
    ok = !((r == INT_MAX) || (r == INT_MIN)) || (errno != ERANGE);
    return r;
  }

#ifdef PLATFORM_WINDOWS
#define strtoull _strtoui64
#endif

  inline U8 strToU8(const char *const s, bool &ok) {
    ok = false;
    if (!s || !*s)
      return 0;
    char *e = 0;
    const U8 r = strtoull(s, &e, 0);
    if (!e || *e)
      return 0;
    ok = (r != ULLONG_MAX) || (errno != ERANGE);
    return r;
  }

}

#endif // STR_HPP
