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

// endian.hpp : endian informacije i konverzije
//

#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <cstddef>

#include "platform.hpp"

namespace endian {

  enum Endian { eLittle, eBig };

  extern const struct EndianInfo {
    Endian native;
    EndianInfo();
  } eInfo;

  // Konvertira jedan byte ordering u drugi

  template<typename T>
  inline T eConvert(const T u) {
    const U1 *ub = reinterpret_cast<const U1*>(&u);
    T r = 0;  // Provjera da tip T donekle imitira integralni
    U1 *rb = reinterpret_cast<U1*>(&r);
    const size_t s = sizeof(T);
    for (size_t i = 0; i < s; ++i)
      rb[i] = ub[s-i-1];
    return r;
  }

  // Konvertira iz danog byte orderinga u drugi, po defaultu nativni

  template<typename T>
  inline T eConvertFromTo(const T u, const Endian from, const Endian to = eInfo.native) {
    return from == to ? u : eConvert(u);
  }

}

#endif // ENDIAN_HPP
