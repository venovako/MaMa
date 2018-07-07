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

#include "compiler.hpp"
using namespace compiler;

namespace {

  inline void about() {
    cout << endl <<
      "MaMaC version 001-20051010 :: Macro Machine bytecode compiler" << endl <<
      "Autorska prava (C) 2005 Vedran Novakovic <venovako@gmail.com>" << endl <<
      "Sva prava pridrzana.  Za detalje konzultirajte dokumentaciju." << endl <<
    endl;
  }

  inline void usage(const char *const xn) {
    about();
    cout << xn << " [-v] [-g] MaMa_source_code_file" << endl;
    cout << endl;
    cout << "-v\tshow compilation steps and statistics" << endl;
    cout << "-g\temit debugging bytecode" << endl;
    exit(EXIT_FAILURE);
  }

  inline int min(const int a, const int b) {
    return a < b ? a : b;
  }

}

int main(const int argc, const char *const argv[]) {
  Compiler::CmdLineFlags clf;

  const int m = min(2,argc-2);
  for (int i = 1; i <= m; ++i) {
    if (!strcmp("-v", argv[i])) {
      if (clf.verbose)
        usage(*argv);
      else
        clf.verbose = true;
    }
    else if (!strcmp("-g", argv[i])) {
      if (clf.debug)
        usage(*argv);
      else
        clf.debug = true;
    }
    else
      usage(*argv);
  }

  int cnt = 0;
  if (clf.verbose)
    ++cnt;
  if (clf.debug)
    ++cnt;
  const char *const last = argv[argc-1];
  if (argc != cnt+2 || *last == '-')
    usage(*argv);

  if (clf.verbose)
    about();

  try {
    if ((comp = new Compiler(clf,last)))
      comp->compile();
    else
      throw bad_alloc();
    cnt = EXIT_SUCCESS;
  }
  catch (const exception &e) {
    cerr << e.what() << endl;
    cnt = EXIT_FAILURE;
  }

  delete comp;
  comp = 0;
  return cnt;
}
