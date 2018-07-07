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

// compiler.hpp : kompajler
//

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <functional>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "stri_ng.hpp"
using namespace stri_ng;

#include "descript.hpp"
#include "instruct.hpp"
using namespace bytecode;

namespace compiler {

  extern class Compiler {

    typedef map<U4,U4> RegMap;
    typedef map<string,U4> LabMap;
    typedef vector<Instruction*> InstrVec;

    enum Statement { stmtUnknown, stmtSystem, stmtVersion, stmtGoto, stmtCall, stmtSet, stmtDec, stmtDek, stmtInc };
    enum StmtClass { stclUnknown, stclDirective, stclInstruction };

    enum LabClass { lcNormal, lcSpecial };

    template<typename T>
    class CompilerOp : public unary_function<T,void> {
      typedef void (Compiler::*PCM)(T&);
      Compiler &cp;
      PCM mfun;
    public:
      CompilerOp(Compiler &c, const PCM mf)
        : cp(c), mfun(mf) {}
      void operator()(T &arg) const {
        (cp.*mfun)(arg);
      }
    };

    typedef CompilerOp<Instruction*> InstructionOp;
    typedef CompilerOp<LabMap::value_type> LabelOp;

    Compiler &operator=(const Compiler&) { return *this; }

  public:

    const struct CmdLineFlags {
      bool debug;
      bool verbose;
      explicit CmdLineFlags(const bool g = false, const bool v = false)
        : debug(g), verbose(v) {}
    } cmdLineFlags;

  private:

    int phase;
    U4 lineno;

    const char *const infile;
    ifstream source;

    Descriptor desc;
    U4 offset;

    ofstream *output;

    string label;
    StrVec parsed;
    InstrVec ivec;
    RegMap regMap;
    LabMap labMap;

    int nextPhase();
    bool nextLine();

    static bool isRegister(const string &s, U4 *r = 0) {
      if (s.size() >= 2 && '%' == s[0]) {
        if (s.size() == 2 && '%' == s[1]) {
          if (r)
            *r = regTemp;
          return true;
        }
        bool ok = false;
        const U4 t = strToU4(s.c_str()+1, ok);
        if (ok && t < regTemp) {
          if (r)
            *r = t;
          return true;
        }
      }
      return false;
    }

    static bool isLabel(const string &s, string *l = 0) {
      if (s.size() >= 2 && ':' == s[0]) {
        if (l) // izbacimo pocetnu :
          *l = s.c_str() + 1;
        return true;
      }
      return false;
    }

    static Statement tellStmt(const string &s) {
      if (s == "SYSTEM")
        return stmtSystem;
      else if (s == "VERSION")
        return stmtVersion;
      else if (s == "GOTO")
        return stmtGoto;
      else if (s == "CALL")
        return stmtCall;
      else if (s == "SET")
        return stmtSet;
      else if (s == "DEC")
        return stmtDec;
      else if (s == "DEK")
        return stmtDek;
      else if (s == "INC")
        return stmtInc;
      return stmtUnknown;
    }

    static StmtClass classify(const Statement s) {
      switch (s) {
        case stmtSystem:
        case stmtVersion:
          return stclDirective;
        case stmtGoto:
        case stmtCall:
        case stmtSet:
        case stmtDec:
        case stmtDek:
        case stmtInc:
          return stclInstruction;
        default:
          return stclUnknown;
      }
    }

    // pretpostavljamo da je ulaz validni label
    static U4 tellLabel(const string &s) {
      const string::size_type ss = s.size();
      assert(ss);
      if (ss == 1)
        switch (s[0]) {
          case '!':
            return labThis;
          case ':':
            return labEnd;
        }
      return 1;
    }

    static LabClass labClassify(const U4 l) {
      return labThis == l || labEnd == l ? lcSpecial : lcNormal;
    }

    void regResolve(Instruction*&);
    void offCalc(Instruction*&);
    void labResolve(LabMap::value_type&);
    void updateTgtLabel(Instruction*&);
    void dbgRecEmit(Instruction*&);
    void instrEmit(Instruction*&);

    static void instrCleanup(Instruction*&);

  public:

    Compiler(const CmdLineFlags&, const char *const);

    void compile();

  } *comp;

}

#endif // COMPILER_HPP
