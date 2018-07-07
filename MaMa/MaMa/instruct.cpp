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

#include <sstream>
using namespace std;

#include "interprt.hpp"

#include "instruct.hpp"

namespace bytecode {

  const U4 regTemp = 0x0FFFFFFF, labThis = 0, labEnd = 0x7FFFFFFF;

  void regToStr(const U4 r, string &s, const bool c) {
    ostringstream ost;
    ost << '%';
    regTemp == r ? ost << '%' : ost << r;
    c ? (s = ost.str()) : (s += ost.str());
  }

  void labToStr(const U4 l, const interprt::Interpreter::Context &ctxt, string &s) {
    ostringstream ost;
    ost << ':';
    switch (l) {
      case labThis:
        ost << '!';
        break;
      case labEnd:
        ost << ':';
        break;
      default:
        ost << ctxt.FptrToIndex(l);
    }
    s += ost.str();
  }

  void INC::toString(string &s, const interprt::Interpreter::Context&) const {
    s += "INC ";
    regToStr(reg(), s);
  }

  void GOTO::toString(string &s, const interprt::Interpreter::Context &ctxt) const {
    s += "GOTO ";
    labToStr(target(), ctxt, s);
  }

  void CALL::toString(string &s, const interprt::Interpreter::Context&) const {
    s += "CALL";
    if (p()) {
      s += ' ';
      s += prg;
    }
    for (ArgVec::const_iterator it = arg.begin(); it != arg.end(); ++it) {
      s += ' ';
      if (it->first) {
        ostringstream ost;
        ost << it->second;
        s += ost.str();
      }
      else
        regToStr(static_cast<U4>(it->second), s);
    }
  }

  void SET::toString(string &s, const interprt::Interpreter::Context&) const {
    s += "SET ";
    regToStr(lhs(), s);
    s += ' ';
    if (test()) {
      ostringstream ost;
      ost << rhs.c;
      s += ost.str();
    }
    else
      regToStr(rhs.r, s);
  }

  void DECK::toString(string &s, const interprt::Interpreter::Context &ctxt) const {
    s += "DE";
    s += dek() ? 'K' : 'C';
    s += ' ';
    regToStr(reg(), s);
    s += ' ';
    labToStr(target(), ctxt, s);
  }

}
