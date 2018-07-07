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

// descript.hpp : bytecode deskriptor
//

#ifndef DESCRIPT_HPP
#define DESCRIPT_HPP

#include <fstream>
#include <cassert>
#include <cstddef>
using namespace std;

#include "endian.hpp"
using namespace endian;

#include "excpt.hpp"
using namespace excpt;

namespace bytecode {

  enum LangSys { system2, system3 };

  class Descriptor {

    U4 w0, w1;

  public:

    Endian e() const {
      return Endian((w0 & 0x80000000) >> 31);
    }

    LangSys s() const {
      return LangSys((w0 & 0x40000000) >> 30);
    }

    bool d() const {
      return bool(w0 & 0x20000000);
    }

    U4 i() const {
      return w0 & 0x1FFFFFFF;
    }

    U1 v() const {
      return U1((w1 & 0xF0000000) >> 28);
    }

    U4 r() const {
      return w1 & 0x0FFFFFFF;
    }

    void s(const LangSys ns) {
      w0 &= 0xBFFFFFFF;
      w0 |= ns << 30;
    }

    void i(const U4 ni) {
      assert(ni < 0x20000000);
      w0 &= 0xE0000000;
      w0 |= ni;
    }

    void v(const U1 nv) {
      assert(nv < 0x10);
      w1 &= 0x0FFFFFFF;
      w1 |= nv << 28;
    }

    void r(const U4 nr) {
      assert(nr < 0x10000000);
      w1 &= 0xF0000000;
      w1 |= nr;
    }

    static size_t size() {
      return 8;
    }

    explicit Descriptor(ifstream &is) : w0(0), w1(0) {
      if (is.read(reinterpret_cast<char*>(&w0), sizeof(w0))) {
        w0 = eConvertFromTo(w0, eLittle);
        assert(i());
        if (is.read(reinterpret_cast<char*>(&w1), sizeof(w1))) {
          w1 = eConvertFromTo(w1, e());
          assert(r());
        }
        else
          throw EUnexpectedEOF(__FILE__, __LINE__);
      }
      else
        throw EUnexpectedEOF(__FILE__, __LINE__);
    }

  };

}

#endif // DESCRIPT_HPP
