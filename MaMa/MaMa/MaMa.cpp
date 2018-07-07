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
#include <cstring>
#include <iostream>
#include <exception>
#include <new>
using namespace std;

#include "stri_ng.hpp"
using namespace stri_ng;

#include "interprt.hpp"
using namespace interprt;

namespace {

  inline void about() {
    cout << endl <<
      "MaMa version 001-20051010  Macro Machine bytecode interpreter" << endl <<
      "Autorska prava (C) 2005 Vedran Novakovic <venovako@gmail.com>" << endl <<
      "Sva prava pridrzana.  Za detalje konzultirajte dokumentaciju." << endl <<
    endl;
  }

  inline void usage(const char *const xn) {
    about();
    cout << xn << " [-v|-s] MaMa_program registers" << endl;
    cout << endl;
    cout << "-v\tverbose mode (prints out instructions and their effects)" << endl;
    cout << "-s\tstep mode (verbose + basic debugging capabilities)" << endl;
    cout << endl;
    cout << "Default: prints out result or error message only." << endl;
    cout << "For verbose and step modes debugging bytecode is required!" << endl;
    exit(EXIT_FAILURE);
  }

}

int main(const int argc, const char *const argv[]) {
  if (argc < 2)
    usage(*argv);

  Interpreter::CmdLineFlags clf;

  int next = 2;
  if (!strcmp("-\?", argv[1]) || !strcmp("/\?", argv[1]) || !strcmp("-h", argv[1]) || !strcmp("--help", argv[1]))
    usage(*argv);
  else if (!strcmp("-v", argv[1]))
    clf.verbose = true;
  else if (!strcmp("-s", argv[1]))
    clf.verbose = clf.steps = true;
  else
    next = 1;

  if (argc <= next)
    usage(*argv);

  const char *const fn = argv[next++];

  try {
    if (Interpreter::DataVec *input = new Interpreter::DataVec) {
      for (bool ok = false; next < argc; ++next) {
        const U8 c = strToU8(argv[next], ok);
        if (!ok)
          usage(*argv);
        input->push_back(c);
      }

      if ((root = new Interpreter(clf, fn, *input))) {
        delete input;
        root->interpret();
        next = EXIT_SUCCESS;
      }
      else
        throw bad_alloc();
    }
    else
      throw bad_alloc();
  }
  catch (const exception &e) {
    cerr << e.what() << endl;
    next = EXIT_FAILURE;
  }

  delete root;
  root = 0;
  return next;
}
