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

#include "endian.hpp"
using namespace endian;

#include "descript.hpp"
#include "commdecl.hpp"

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

    virtual void toString(string&, const interprt::Interpreter::Context&) const = 0;

    explicit Instruction(const U4 nhdr = 0, const U4 nfp = 0, const U4 nlno = 0)
      : head(nhdr), fp(nfp), lno(nlno) {}

    virtual ~Instruction() {}

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

    void toString(string&, const interprt::Interpreter::Context&) const;

    INC(const U4 rw, const U4 nfp, const U4 nlno = 0)
      : Instruction(rw, nfp, nlno) {}

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

    void toString(string&, const interprt::Interpreter::Context&) const;

    GOTO(const U4 rw, const U4 nfp, const U4 nlno = 0)
      : Instruction(rw, nfp, nlno) {}

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

    void toString(string&, const interprt::Interpreter::Context&) const;

    CALL(const U4 rwh, ifstream &is, const Endian end, const U4 nfp, const U4 nlno = 0)
      : Instruction(rwh, nfp, nlno), prg()
    {
      if (const U1 cp = p()) {
        if (char *const tmp = new char[cp + 1]) {
          tmp[cp] = '\0';
          if (!is.read(tmp, cp))
            throw EUnexpectedEOF(__FILE__, __LINE__);
          prg = tmp;
          delete [] tmp;
        }
        else
          throw bad_alloc();
      }

      const U4 cd = d();
      for (U4 offset = 0; offset < cd; ) {
        char q = 0;
        if (!is.read(&q, sizeof(q)))
          throw EUnexpectedEOF(__FILE__, __LINE__);
        ++offset;
        if (q) {
          U8 c = 0;
          if (!is.read(reinterpret_cast<char*>(&c), sizeof(c)))
            throw EUnexpectedEOF(__FILE__, __LINE__);
          offset += sizeof(c);
          add(eConvertFromTo(c,end));
        }
        else {
          U4 r = regTemp;
          if (!is.read(reinterpret_cast<char*>(&r), sizeof(r)))
            throw EUnexpectedEOF(__FILE__, __LINE__);
          offset += sizeof(r);
          r = eConvertFromTo(r,end);
          assert(r < 0x10000000);
          add(r);
        }
      }
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

    void toString(string&, const interprt::Interpreter::Context&) const;

    SET(const U4 rwh, ifstream &is, const Endian end, const U4 nfp, const U4 nlno = 0)
      : Instruction(rwh, nfp, nlno), rhs()
    {
      if (test()) {
        if (!is.read(reinterpret_cast<char*>(&rhs.c), sizeof(rhs.c)))
          throw EUnexpectedEOF(__FILE__, __LINE__);
        rhs.c = eConvertFromTo(rhs.c, end);
      }
      else {
        if (!is.read(reinterpret_cast<char*>(&rhs.r), sizeof(rhs.r)))
          throw EUnexpectedEOF(__FILE__, __LINE__);
        rhs.r = eConvertFromTo(rhs.r, end);
        assert(rhs.r < 0x10000000);
      }
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

    void toString(string&, const interprt::Interpreter::Context&) const;

    DECK(const U4 rwh, const U4 rwt, const U4 nfp, const U4 nlno = 0)
      : Instruction(rwh, nfp, nlno), targ(rwt) {}

  };

}

#endif // INSTRUCT_HPP
