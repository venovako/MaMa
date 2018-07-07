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

#include "platform.hpp"

// definirajte ako nemate boost headere, a sigurni ste u donje assertione

#ifndef PLATFORM_CHECKED

#include <climits>
#include <boost/static_assert.hpp>

// Provjera ispravnosti tipovnih definicija
// Ovakva strogost nuzna je za binarni I/O

namespace platform {

  BOOST_STATIC_ASSERT(sizeof(S1) * CHAR_BIT ==  8);
  BOOST_STATIC_ASSERT(sizeof(S2) * CHAR_BIT == 16);
  BOOST_STATIC_ASSERT(sizeof(S4) * CHAR_BIT == 32);
  BOOST_STATIC_ASSERT(sizeof(S8) * CHAR_BIT == 64);

  BOOST_STATIC_ASSERT(sizeof(U1) * CHAR_BIT ==  8);
  BOOST_STATIC_ASSERT(sizeof(U2) * CHAR_BIT == 16);
  BOOST_STATIC_ASSERT(sizeof(U4) * CHAR_BIT == 32);
  BOOST_STATIC_ASSERT(sizeof(U8) * CHAR_BIT == 64);

}

#endif // PLATFORM_CHECKED
