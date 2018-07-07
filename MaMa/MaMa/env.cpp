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

#include <cstdlib>
#include <cassert>
#include <new>
using namespace std;

#include "env.hpp"

namespace env {

  namespace path {

#if defined(PLATFORM_WINDOWS)

    const char separator = '\\', delimiter = ';';

#elif defined(PLATFORM_POSIX)

    const char separator =  '/', delimiter = ':';

#endif

  }

  class GetEnv {
    String e;
  public:
    explicit GetEnv(const char *const name, const char *const dflt = 0) : e() {
      const char *ge = name ? getenv(name) : 0;
      if (!ge)
        ge = dflt;
      if (ge && !(e = ge).trimBoth().empty() && path::separator != e[e.size()-1])
        e += path::separator;
    }
    operator const string&() const { return e; }
  };

#if defined(PLATFORM_WINDOWS)

  const char *const MAMA_HOME_default = "C:\\MaMa\\";

#elif defined(PLATFORM_POSIX)

  const char *const MAMA_HOME_default = "/opt/MaMa/";

#endif

  const string MAMA_HOME(GetEnv("MAMA_HOME", MAMA_HOME_default));

  MaMaPath::MaMaPath() : dirs(1) {
    String mp;
    if (const char *const rmp = getenv("MAMA_PATH"))
      (mp = rmp).trimBoth();
    else
#if defined(PLATFORM_WINDOWS)
      mp = static_cast<const string&>(GetEnv("MAMA_HOME", MAMA_HOME_default)) + "lib";
#elif defined(PLATFORM_POSIX)
      mp = static_cast<const string&>(GetEnv("MAMA_HOME", MAMA_HOME_default)) + "libx";
#endif
    mp.tokenize(dirs, path::delimiter, false);
    for (StrVec::iterator it = dirs.begin(); it != dirs.end(); ++it)
      if (!it->trimBoth().empty() && path::separator != (*it)[it->size()-1])
        *it += path::separator;
  }

  MaMaPath::FilePair MaMaPath::search(const char *const fn, const bool src) const {
    assert(fn);
    String bcode(fn), source(fn);
    FilePair fp(static_cast<ifstream*>(0),static_cast<ifstream*>(0));
    if (bcode.trimBoth().empty())
      return fp;
    bcode += ".MMb";
    source.trimBoth() += ".MMs";

    for (StrVec::const_iterator it = dirs.begin(); it != dirs.end(); ++it) {
      if ((fp.first = new ifstream((*it + bcode).c_str(), ios_base::in | ios_base::binary))) {
        if (!*fp.first) {
          delete fp.first;
          fp.first = 0;
          continue;
        }
        if (src) {
          if ((fp.second = new ifstream((*it + source).c_str()))) {
            if (!*fp.second) {
              delete fp.second;
              fp.second = 0;
            }
          }
          else
            throw bad_alloc();
        }
      }
      else
        throw bad_alloc();
      return fp;
    }

    return fp;
  }

  const MaMaPath MAMA_PATH;

}
