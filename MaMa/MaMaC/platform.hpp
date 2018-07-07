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

// platform.hpp : platform-specific stvari
//

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

// PLATFORM_WIN32_MSVC ... Windows @ x86; MS Visual C++
// PLATFORM_WIN64_MSVC ... Windows @ x64; MS Visual C++
// PLATFORM_x86_GNU    ... Windows, POSIX @ x86; GNU C++
// PLATFORM_x64_GNU    ... POSIX @ x64; GNU C++

//////////////////////////////////////////////////
//                                              //
// OVDJE DEFINIRAJTE PRIKLADNI PLATFORM_* MACRO //
//                                              //
//////////////////////////////////////////////////

#if defined(PLATFORM_WIN32_MSVC) || defined(PLATFORM_WIN64_MSVC)
#define PLATFORM_WINDOWS
#elif defined(PLATFORM_x86_GNU) || defined(PLATFORM_x64_GNU)
#define PLATFORM_POSIX
#elif !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_POSIX)
#error neither WINDOWS nor POSIX defined
#endif

typedef signed char        S1;
typedef signed short       S2;
typedef signed int         S4;
typedef signed long long   S8;

typedef unsigned char      U1;
typedef unsigned short     U2;
typedef unsigned int       U4;
typedef unsigned long long U8;

#if defined(PLATFORM_WIN32_MSVC) || defined(PLATFORM_WIN64_MSVC) || defined(PLATFORM_x86_GNU) || defined(PLATFORM_x64_GNU)
#define PLATFORM_CHECKED
#endif

#endif // PLATFORM_HPP
