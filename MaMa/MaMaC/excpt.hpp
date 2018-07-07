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

// excpt.hpp : namespace sviju exceptiona
//

#ifndef EXCPT_HPP
#define EXCPT_HPP

#include <exception>
#include <typeinfo>
#include <sstream>
#include <string>
using namespace std;

namespace excpt {

  // Svaka iznimka je Exception

  class Exception : public exception {

    mutable bool exgen;

  protected:

    mutable string expl;

  public:

    Exception() throw() : exgen(true) {}

    Exception(const char *const file, const int line) throw() : exgen(false) {
      ostringstream ost;
      ost << "! " << (file ? file : "<no_file>") << " @ ";
      (line > 0 ? ost << line : ost << "<no_line>") << " : ";
      expl = ost.str();
    }

    Exception(const Exception &e) throw() : exception(), exgen(true), expl(e.what()) {}

    Exception &operator=(const Exception &e) throw() {
      if (this != &e) {
	    exgen = true;
	    expl = e.what();
      }
      return *this;
    }

    ~Exception() throw() {}

    const char *what() const throw() {
      if (!exgen) {
	    expl = string(typeid(*this).name()) + expl;
	    exgen = true;
      }
      return expl.c_str();
    }

  };

  struct EFileNotReadable : public Exception {
    explicit EFileNotReadable(const char *const fn) throw() : Exception() {
      expl = string("FILE NOT READABLE: ") + fn;
    }
    EFileNotReadable(const char *const file, const int line, const char *const fn) throw()
      : Exception(file,line) { expl += fn; }
  };

  struct EInternal : public Exception {
    EInternal(const char *const file, const int line) throw() : Exception(file,line) {}
  };

  struct EWriteError : public Exception {
    EWriteError(const char *const file, const int line) throw() : Exception(file,line) {}
  };

}

#endif // EXCPT_HPP
