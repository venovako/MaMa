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

// interprt.hpp : interpreter
//

#ifndef INTERPRT_HPP
#define INTERPRT_HPP

#include <functional>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#include "env.hpp"
using namespace env;

#include "descript.hpp"
#include "commdecl.hpp"

namespace bytecode {

  class Instruction;
  class CALL;

}

using namespace bytecode;

#include "platform.hpp"

namespace interprt {

  extern class Interpreter {

    typedef map<string,ifstream*> BindTable;
    static struct DynBindTable {
      BindTable tbl;
      DynBindTable() : tbl() {}
      ~DynBindTable() {
        for (BindTable::const_iterator it = tbl.begin(); tbl.end() != it; ++it)
          delete it->second;
      }
    } dynBindTable;

    const string progName;
    const U4 arity;
    U4 progCnt, countdown, step;
    MaMaPath::FilePair files;
    ifstream::pos_type filePos, oldPos;

  public:
    
    Interpreter *const parent;

    const struct CmdLineFlags {
      bool steps;
      bool verbose;
      explicit CmdLineFlags(const bool s = false, const bool v = false)
        : steps(s), verbose(v || s) {}
    } cmdLineFlags;

    const class Context {

      struct FptrLT : public binary_function<const DbgRec&,const DbgRec&,bool> {
        bool operator()(const DbgRec &a, const DbgRec &b) const {
          return a.first < b.first;
        }
      };

      struct LineNoLT : public binary_function<const DbgRec&,const DbgRec&,bool> {
        bool operator()(const DbgRec &a, const DbgRec &b) const {
          return a.second < b.second;
        }
      };

    public:

      typedef vector<DbgRec> DbgRecVec;

      Descriptor desc;
      DbgRecVec *const dbgRecs;

      Context(ifstream&, const bool);
      ~Context();

      // O(1)

      U4 IndexToFptr(const U4 ix) const {
        return dbgRecs->at(ix).first;
      }

      U4 IndexToLineNo(const U4 ix) const {
        return dbgRecs->at(ix).second;
      }

      // O(log)

      U4 FptrToIndex(const U4) const;
      U4 LineNoToIndex(const U4) const;
      U4 FptrToLineNo(const U4) const;
      U4 LineNoToFptr(const U4) const;

    } *context;

  private:

    typedef map<U4,string> LineMap;

    LineMap *srcLines;

  public:

    typedef vector<U8> DataVec;

    // kreiranje root instance
    Interpreter(const CmdLineFlags&, const string&, const DataVec&);

    Interpreter(Interpreter&, const CALL&);

    ~Interpreter();

    void interpret();

    // translatira registre na nivou trenutne instance Interpretera,
    // ali tek nakon sto se pomocu translateReg() cachiraju sve
    // translacije u pripadna mjesta na registarskoj traci
    U8 &getReg(const U4);
    U8 getReg(const U4) const;

  private:

    union RegSlot {

      U8 data;
      U8 *ptr;

      RegSlot(const U8 d = 0)
        : data(d) {}
      RegSlot(const RegSlot &rs)
        : data(rs.data) {}

      RegSlot &operator=(const RegSlot &rs) {
        if (this != &rs)
          data = rs.data;
        return *this;
      }
      RegSlot &operator=(const U8 d) {
        data = d;
        return *this;
      }

    };

    typedef vector<RegSlot> RegSlotVec;
    typedef map<U4,U4> RegMap;

    RegSlotVec regs;
    RegMap rmap;

    // rekurzivno translatira registre, uz tri osnovne pretpostavke:
    // -> %% je uvijek registar s izravnim pristupom (nije kljuc)
    // -> root nema mapiranje registara (kraj rekurzije)
    // -> parent je vec proveo inicijalno cachiranje translacija
    U8 &translateReg(const U4);
    U8 translateReg(const U4) const;

    void initFiles();
    void initContext(const bool);
    void allocRegs();
    void initRegs(const DataVec&);
    void initRegs(const ArgVec&);
    void transCache();
    void srcLineCache();

    const Instruction *fetch();
    bool jumpTo(const U4);
    bool execute(const Instruction&);

    void regDump(const U4) const;
    void regDump() const;

    void introduction() const;
    void printInstr(const Instruction&) const;
    bool interact();

  } *root;

}

#endif // INTERPRT_HPP
