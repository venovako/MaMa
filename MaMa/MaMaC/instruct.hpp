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

// instruct.hpp : instrukcije
//

#ifndef INSTRUCT_HPP
#define INSTRUCT_HPP

#include <fstream>
#include <cstddef>
#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include <new>
using namespace std;

#include "descript.hpp"

namespace bytecode {

  extern const U4 regTemp, labThis, labEnd;

  class Instruction {

  protected:

    U4 head, fp, lno;

  public:

    U4 header() const {
      return head;
    }

    U4 fptr() const {
      return fp;
    }

    void fptr(const U4 nfp) {
      assert(nfp < 0x80000000);
      fp = nfp;
    }

    U4 lineno() const { return lno; }

    // 1 <= lineno < 2^31
    // lineno == 0 <==> lineno nedefiniran

    void lineno(const U4 nlno) {
      assert(lno < 0x80000000);
      lno = nlno;
    }

    virtual size_t size() const = 0;

    static size_t dbgRecSize() {
      return 8;
    }

    explicit Instruction(const U4 nhdr = 0, const U4 nfp = 0, const U4 nlno = 0)
      : head(nhdr), fp(nfp), lno(nlno) {}

    virtual ~Instruction() {}

    void dbgRecEmit(ofstream &os) {
      if (!os.write(reinterpret_cast<const char*>(&fp), sizeof(fp)))
        throw EWriteError(__FILE__, __LINE__);
      if (!os.write(reinterpret_cast<const char*>(&lno), sizeof(lno)))
        throw EWriteError(__FILE__, __LINE__);
    }

    virtual void emit(ofstream&) const = 0;

  };

  // INC

  class INC : public Instruction {

  public:

    size_t size() const {
      return sizeof(head);
    }

    U4 reg() const {
      return head & 0x0FFFFFFF;
    }

    void reg(const U4 nr) {
      assert(nr < 0x10000000);
      head &= 0xF0000000;
      head |= nr;
    }

    void emit(ofstream &os) const {
      if (!os.write(reinterpret_cast<const char*>(&head), sizeof(head)))
        throw EWriteError(__FILE__, __LINE__);
    }

    INC(const U4 nlno, const U4 nr) : Instruction(0x10000000 | nr, 0, nlno) {
      assert(nr < 0x10000000);
    }

  };

  // GOTO

  class GOTO : public Instruction {

  public:

    size_t size() const {
      return sizeof(head);
    }

    U4 target() const {
      return head & 0x7FFFFFFF;
    }

    void target(const U4 nt) {
      assert(nt < 0x80000000);
      head = 0x80000000 | nt;
    }

    void emit(ofstream &os) const {
      if (!os.write(reinterpret_cast<const char*>(&head), sizeof(head)))
        throw EWriteError(__FILE__, __LINE__);
    }

    string tLabel; // target label

    GOTO(const U4 nlno, const U4 nt) : Instruction(0x80000000 | nt, 0, nlno) {
      assert(nt < 0x80000000);
    }

  };

  // CALL

  class CALL : public Instruction {

  public:

    typedef pair<bool,U8> ArgPair;
    typedef vector<ArgPair> ArgVec;

  private:

    string prg;
    
    ArgVec arg;

  protected:

    U1 p() const {
      return U1(head & 0xFF);
    }

    void p(const U1 np) {
      head &= 0xFFFFFF00;
      head |= np;
    }

    U4 d() const {
      return (head & 0x1FFFFF00) >> 8;
    }

    void d(const U4 nd) {
      assert(nd < 0x00200000);
      head &= 0xE00000FF;
      head |= nd << 8;
    }

  public:

    size_t size() const {
      return sizeof(head) + p() + d();
    }

    const string &prog() const {
      return prg;
    }

    void prog(const string &np) {
      assert(np.size() < 256);
      p(U1((prg = np).size()));
    }

    const ArgVec &args() const {
      return arg;
    }

    ArgVec &args() {
      return arg;
    }

    void add(const U8 c) {
      arg.push_back(pair<bool,U8>(true,c));
    }

    void add(const U4 r) {
      assert(r < 0x10000000);
      arg.push_back(pair<bool,U8>(false,r));
    }

    // nuzno pozivati samo ako se dodaju argumenti

    void finalize() {
      U4 hd = 0;
      for (ArgVec::const_iterator it = arg.begin(); it != arg.end(); ++it)
        hd += (it->first ? sizeof(U8) : sizeof(U4)) + 1;
      d(hd);
    }

    void emit(ofstream &os) const {
      if (!os.write(reinterpret_cast<const char*>(&head), sizeof(head)))
        throw EWriteError(__FILE__, __LINE__);
      if (!os.write(prg.c_str(), p()))
        throw EWriteError(__FILE__, __LINE__);

      for (ArgVec::const_iterator it = arg.begin(); it != arg.end(); ++it) {
        const char q = static_cast<char>(it->first);
        if (!os.write(&q, sizeof(q)))
          throw EWriteError(__FILE__, __LINE__);

        if (q) {
          const U8 c = it->second;
          if (!os.write(reinterpret_cast<const char*>(&c), sizeof(c)))
            throw EWriteError(__FILE__, __LINE__);
        }
        else {
          const U4 r = static_cast<U4>(it->second);
          if (!os.write(reinterpret_cast<const char*>(&r), sizeof(r)))
            throw EWriteError(__FILE__, __LINE__);
        }
      }
    }

    CALL(const U4 nlno, const string &np) : Instruction(0x40000000, 0, nlno), prg() {
      prog(np);
    }

  };

  // SET

  class SET : public Instruction {

    union Rhs {
      U8 c;
      U4 r;
      explicit Rhs(const U8 nc = 0) : c(nc) {}
      explicit Rhs(const U4 nr) : c(0) {
        assert(nr < 0x10000000);
        r = nr;
      }
    } rhs;

  protected:

    void c(const bool nc) {
      head &= 0xEFFFFFFF;
      head |= nc << 28;
    }

  public:

    bool test() const {
      return bool(head & 0x10000000);
    }

    size_t size() const {
      return sizeof(head) + (test() ? sizeof(rhs.c) : sizeof(rhs.r));
    }

    U4 lhs() const {
      return head & 0x0FFFFFFF;
    }

    void lhs(const U4 nr) {
      assert(nr < 0x10000000);
      head &= 0xF0000000;
      head |= nr;
    }

    bool grhs(U8 &a) const {
      if (test()) {
        a = rhs.c;
        return true;
      }
      return false;
    }

    void srhs(const U8 a) {
      c(true);
      rhs.c = a;
    }

    bool grhs(U4 &a) const {
      if (test())
        return false;
      a = rhs.r;
      return true;
    }

    void srhs(const U4 a) {
      assert(a < 0x10000000);
      c(false);
      rhs.r = a;
    }

    void emit(ofstream &os) const {
      if (!os.write(reinterpret_cast<const char*>(&head), sizeof(head)))
        throw EWriteError(__FILE__, __LINE__);
      if (!(test() ? os.write(reinterpret_cast<const char*>(&rhs.c), sizeof(rhs.c)) :
        os.write(reinterpret_cast<const char*>(&rhs.r), sizeof(rhs.r))))
        throw EWriteError(__FILE__, __LINE__);
    }

    SET(const U4 nlno, const U4 nr, const U8 rc)
      : Instruction(0x70000000 | nr, 0, nlno), rhs(rc)
    {
      assert(nr < 0x10000000);
    }

    SET(const U4 nlno, const U4 nr, const U4 rr)
      : Instruction(0x60000000 | nr, 0, nlno), rhs(rr)
    {
      assert(nr < 0x10000000);
    }

  };

  // DECK

  class DECK : public Instruction {

    U4 targ;

  public:

    size_t size() const { return sizeof(head) + sizeof(targ); }

    U4 reg() const {
      return head & 0x0FFFFFFF;
    }

    void reg(const U4 nr) {
      assert(nr < 0x10000000);
      head &= 0xF0000000;
      head |= nr;
    }

    U4 target() const {
      return targ & 0x7FFFFFFF;
    }

    void target(const U4 nt) {
      assert(nt < 0x80000000);
      targ &= 0x80000000;
      targ |= nt;
    }

    LangSys syst() const {
      return LangSys((targ & 0x80000000) >> 31);
    }

    void syst(const LangSys ns) {
      targ &= 0x7FFFFFFF;
      targ |= ns << 31;
    }

    bool dek() const {
      return bool(head & 0x10000000);
    }

    void dek(const bool nk) {
      head &= 0xEFFFFFFF;
      head |= nk << 28;
    }

    void emit(ofstream &os) const {
      if (!os.write(reinterpret_cast<const char*>(&head), sizeof(head)))
        throw EWriteError(__FILE__, __LINE__);
      if (!os.write(reinterpret_cast<const char*>(&targ), sizeof(targ)))
        throw EWriteError(__FILE__, __LINE__);
    }

    string tLabel; // target label

    DECK(const U4 nlno, const LangSys ns, const bool nk, const U4 nr, const U4 nt)
      : Instruction(0x20000000 | nk << 28 | nr, 0, nlno), targ(ns << 31 | nt)
    {
      assert(nr < 0x10000000);
      assert(nt < 0x80000000);
    }

  };

}

#endif // INSTRUCT_HPP
