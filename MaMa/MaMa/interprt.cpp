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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <ctime>
#include <new>
using namespace std;

#include "stri_ng.hpp"
using namespace stri_ng;

#include "endian.hpp"
using namespace endian;

#include "excpt.hpp"

namespace excpt {

  struct EBytecodeNotSupported : public Exception {
    EBytecodeNotSupported() : Exception() {
      expl = "UNSUPPORTED BYTECODE VERSION";
    }
  };

  struct EInputTooBig : public Exception {
    EInputTooBig() : Exception() {
      expl = "MORE INPUT ELEMENTS THAN REGISTERS";
    }
  };

  struct ETooManyArgs : public Exception {
    ETooManyArgs() : Exception() {
      expl = "MORE CALL ARGUMENTS THAN REGISTERS";
    }
  };

  struct ENoDebug : public Exception {
    ENoDebug() : Exception() {
      expl = "VERBOSE :: DEBUGGING BYTECODE REQUIRED";
    }
  };

  struct ESrcNotInSync : public Exception {
    ESrcNotInSync() : Exception() {
      expl = "BYTECODE NOT IN SYNC WITH THE SOURCE";
    }
  };

  struct EIllegalOpcode : public Exception {
    explicit EIllegalOpcode(const U4 fp = 0) : Exception() {
      expl = "ILLEGAL OPCODE @ ";
      ostringstream ost;
      ost << hex << showbase << fp;
      expl += ost.str();
    }
  };

}

using namespace excpt;

#include "interprt.hpp"

#include "instruct.hpp"
using namespace bytecode;

namespace interprt {

  // CONTEXT implementacija

  Interpreter::Context::Context(ifstream &bcode, const bool interactive)
    : desc(bcode), dbgRecs(interactive && desc.d() ? new DbgRecVec(desc.i()) : 0)
  {
    if (desc.v())
      throw EBytecodeNotSupported();
    if (desc.d())
      for (U4 i = 0; i < desc.i(); ++i) {
        U4 tmp = 0;

        if (!bcode.read(reinterpret_cast<char*>(&tmp), sizeof(tmp)))
          throw EUnexpectedEOF(__FILE__, __LINE__);
        if (dbgRecs)
          (*dbgRecs)[i].first = tmp;

        if (!bcode.read(reinterpret_cast<char*>(&tmp), sizeof(tmp)))
          throw EUnexpectedEOF(__FILE__, __LINE__);
        if (dbgRecs)
          (*dbgRecs)[i].second = tmp;
      }
  }

  Interpreter::Context::~Context() {
    delete dbgRecs;
  }

  // fp mora postojati u dbg zapisima!
  U4 Interpreter::Context::FptrToIndex(const U4 fp) const {
    DbgRecVec::const_iterator it =
      lower_bound(dbgRecs->begin(), dbgRecs->end(), DbgRec(fp,0), FptrLT());
    assert(dbgRecs->end() != it && fp == it->first);
    return U4(it - dbgRecs->begin());
  }

  // vraca indeks zapisa prve linije >= lno ili size() ako takvog nema
  U4 Interpreter::Context::LineNoToIndex(const U4 lno) const {
    DbgRecVec::const_iterator it =
      lower_bound(dbgRecs->begin(), dbgRecs->end(), DbgRec(0,lno), LineNoLT());
    return U4(dbgRecs->end() == it ? dbgRecs->size() : it - dbgRecs->begin());
  }

  // fp mora postojati u dbg zapisima!
  U4 Interpreter::Context::FptrToLineNo(const U4 fp) const {
    DbgRecVec::const_iterator it =
      lower_bound(dbgRecs->begin(), dbgRecs->end(), DbgRec(fp,0), FptrLT());
    assert(dbgRecs->end() != it && fp == it->first);
    return it->second;
  }

  // vraca fptr iz zapisa prve linije >= lno ili labEnd ako takvog nema
  U4 Interpreter::Context::LineNoToFptr(const U4 lno) const {
    DbgRecVec::const_iterator it =
      lower_bound(dbgRecs->begin(), dbgRecs->end(), DbgRec(0,lno), LineNoLT());
    return dbgRecs->end() == it ? labEnd : it - dbgRecs->begin();
  }

  // INTERPRETER implementacija

  Interpreter::Interpreter(const CmdLineFlags &clf, const string &fn, const DataVec &input)
    : progName(fn), arity(U4(input.size())),
      progCnt(0), countdown(1), step(1), files(static_cast<ifstream*>(0),static_cast<ifstream*>(0)),
      filePos(0), oldPos(0), parent(0),
      cmdLineFlags(clf), context(0), srcLines(0)
  {
    initFiles();
    initContext(cmdLineFlags.verbose);
    if (cmdLineFlags.verbose && !context->dbgRecs)
      throw ENoDebug();
    allocRegs();
    initRegs(input);
    if (files.second)
      srcLineCache();
  }

  Interpreter::Interpreter(Interpreter &caca, const CALL &ci)
    : progName(ci.prog()), arity(U4(ci.args().size()-1)),
      progCnt(0), countdown(1), step(1), files(static_cast<ifstream*>(0),static_cast<ifstream*>(0)),
      filePos(0), oldPos(0), parent(&caca),
      cmdLineFlags(false,false), context(0), srcLines(0)
  {
    assert(!ci.args().empty());
    initFiles();
    initContext(false);
    allocRegs();
    initRegs(ci.args());
  }

  Interpreter::~Interpreter() {
    delete srcLines;
    delete context;
    files.first->clear();
    files.first->seekg(oldPos);
  }

  void Interpreter::interpret() {
    if (cmdLineFlags.verbose) {
      introduction();
      regDump();
    }

    for (const Instruction *i = 0; (i = fetch()); delete i) {
      if (cmdLineFlags.verbose) {
        cout << endl;
        printInstr(*i);
      }
      if (cmdLineFlags.steps && step && !--countdown) {
        const U4 oldPC = progCnt;
        if (!interact()) {
          delete i;
          break;
        }
        if (oldPC != progCnt)
          continue;
      }
      if (execute(*i))
        i = 0;
      else
        break;
    }

    if (cmdLineFlags.verbose) {
      cout << endl;
      regDump();
      cout << "RESULT ";
    }
    if (!parent)
      cout << setw(20) << getReg(0) << endl;
  }

  U8 &Interpreter::getReg(const U4 r) {
    if (r == regTemp)
      return regs.back().data;
    RegMap::const_iterator it = rmap.find(r);
    if (rmap.end() == it)
      return regs.at(r).data;
    U8 *const p = regs.at(r).ptr;
    assert(p);
    return *p;
  }

  U8 Interpreter::getReg(const U4 r) const {
    return const_cast<Interpreter*>(this)->getReg(r);
  }

  U8 &Interpreter::translateReg(const U4 r) {
    if (r == regTemp)
      return regs.back().data;
    RegMap::const_iterator it = rmap.find(r);
    if (rmap.end() == it)
      return regs.at(r).data;
    assert(parent);
    return parent->getReg(it->second);
  }

  U8 Interpreter::translateReg(const U4 r) const {
    return const_cast<Interpreter*>(this)->translateReg(r);
  }

  void Interpreter::initFiles() {
    BindTable::const_iterator it = dynBindTable.tbl.find(progName);
    if (dynBindTable.tbl.end() == it) {
      files = MAMA_PATH.search(progName.c_str(), cmdLineFlags.verbose);
      if (!files.first)
        throw EFileNotReadable(progName.c_str());
      dynBindTable.tbl[progName] = files.first;
    }
    else {
      oldPos = (files.first = it->second)->tellg();
      files.first->seekg(0);
    }
  }

  void Interpreter::initContext(const bool interactive) {
    if (!(context = new Context(*files.first, interactive)))
      throw bad_alloc();
  }

  void Interpreter::allocRegs() {
    regs.resize(context->desc.r() + 1);  // +1 == regTemp
  }

  // slobodno je zauzeti sve registre od %1 nadalje, ukljucivo i %%
  // premda se ne savjetuje koristiti privremeni kao input registar
  void Interpreter::initRegs(const DataVec &input) {
    if (input.size() >= regs.size())
      throw EInputTooBig();
    copy(input.begin(), input.end(), regs.begin() + 1);
  }

  void Interpreter::initRegs(const ArgVec &args) {
    if (args.size() > regs.size())
      throw ETooManyArgs();
    for (U4 i = 0; i < args.size(); ++i) {
      if (args[i].first)
        regs[i] = args[i].second;
      else
        rmap[i] = static_cast<U4>(args[i].second);
    }
    transCache();
  }

  // vrsi inicijalno cachiranje translacija
  // na odgovarajuca mjesta registarske trake
  void Interpreter::transCache() {
    for (RegMap::const_iterator it = rmap.begin(); it != rmap.end(); ++it)
      regs[it->first].ptr = &translateReg(it->first);
  }

  // Originalna ideja bila je cachirati samo file pozicije svih nepraznih linija
  // ali ifstream::tellg() je sve pokvario, specijalno seekg(tellg()) ! nije !
  // identiteta na source fajlu, a mozda niti opcenito...?

  void Interpreter::srcLineCache() {
    if ((srcLines = new LineMap)) {
      String s;
      for (U4 lno = 1; getline(*files.second,s); ++lno)
        if (!(s.trimRight().empty() || '#' == s[s.find_first_not_of(String::spc)]))
          (*srcLines)[lno] = s;
      delete files.second;
      files.second = 0;
    }
    else
      throw bad_alloc();
  }

  const Instruction *Interpreter::fetch() {
    if (cmdLineFlags.verbose && progCnt >= context->desc.i())
      return 0;
    const U4 lno = cmdLineFlags.verbose ? context->IndexToLineNo(progCnt) : 0;
    filePos = files.first->tellg();
    U4 head = 0;

    if (!files.first->read(reinterpret_cast<char*>(&head), sizeof(head)))
      return 0;
    head = eConvertFromTo(head, context->desc.e());
    if (cmdLineFlags.verbose)
      ++progCnt;

    if (head & 0x80000000)
      return new GOTO(head, filePos, lno);

    else if (head & 0x40000000) {
      if (head & 0x20000000)
        return new SET(head, *files.first, context->desc.e(), filePos, lno);
      if (CALL *ci = new CALL(head, *files.first, context->desc.e(), filePos, lno)) {
        if (ci->prog().empty())
          ci->prog(progName);
        return ci;
      }
      throw bad_alloc();
    }

    else if (head & 0x20000000) {
      U4 t = 0;
      if (!files.first->read(reinterpret_cast<char*>(&t), sizeof(t)))
        throw EUnexpectedEOF(__FILE__, __LINE__);
      t = eConvertFromTo(t, context->desc.e());
      return new DECK(head, t, filePos, lno);
    }

    else if (head & 0x10000000) {
      return new INC(head, filePos, lno);
    }

    throw EIllegalOpcode(filePos);
  }

  bool Interpreter::jumpTo(const U4 fp) {
    if (labEnd == fp)
      return false;
    else if (labThis == fp) {
      files.first->seekg(filePos);
      if (cmdLineFlags.verbose)
        --progCnt;
    }
    else {
      files.first->seekg(fp);
      if (cmdLineFlags.verbose)
        progCnt = context->FptrToIndex(fp);
    }
    return true;
  }

  bool Interpreter::execute(const Instruction &i) {

    if (const GOTO *gi = dynamic_cast<const GOTO*>(&i)) {
      const U4 fp = gi->target();
      delete gi;
      return jumpTo(fp);
    }

    else if (const SET *si = dynamic_cast<const SET*>(&i)) {
      const U4 lhs = si->lhs();
      if (si->test()) {
        U8 c = 0;
        const bool ok = si->grhs(c);
        assert(ok);
        delete si;
        getReg(lhs) = c;
        if (cmdLineFlags.verbose)
          regDump(lhs);
      }
      else {
        U4 r = regTemp;
        const bool ok = si->grhs(r);
        assert(ok);
        delete si;
        getReg(lhs) = getReg(r);
        if (cmdLineFlags.verbose) {
          regDump(lhs);
          regDump(r);
        }
      }
    }

    else if (const CALL *ci = dynamic_cast<const CALL*>(&i)) {
      if (Interpreter *ip = new Interpreter(*this, *ci)) {
        if (!cmdLineFlags.verbose)
          delete ci;
        ip->interpret();
        delete ip;
      }
      else
        throw bad_alloc();
      if (cmdLineFlags.verbose)
        for (ArgVec::const_iterator it = ci->args().begin(); it != ci->args().end(); ++it)
          if (!it->first)
            regDump(static_cast<U4>(it->second));
    }

    else if (const DECK *di = dynamic_cast<const DECK*>(&i)) {
      // normalizacija na system 3 (za k equ == not xor)
      const bool c = bool(di->dek() ^ di->syst());
      const U4 r = di->reg(), fp = di->target();
      delete di;
      bool ret = true;
      if (U8 &val = getReg(r)) {
        --val;
        ret = c || jumpTo(fp);
      }
      else if (c)
        ret = jumpTo(fp);
      if (cmdLineFlags.verbose)
        regDump(r);
      return ret;
    }

    else if (const INC *ii = dynamic_cast<const INC*>(&i)) {
      const U4 r = ii->reg();
      delete ii;
      ++getReg(r);
      if (cmdLineFlags.verbose)
        regDump(r);
    }

    else
      throw EInternal(__FILE__, __LINE__);

    return true;
  }

  void Interpreter::regDump(const U4 r) const {
    string s;
    regToStr(r,s,true);
    cout << setw(10) << s << ' ' << setw(20) << getReg(r) << endl;
  }

  void Interpreter::regDump() const {
    for (U4 i = 0; i < context->desc.r(); ++i)
      regDump(i);
    regDump(regTemp);
  }

  void Interpreter::introduction() const {
    const time_t t = time(0);
    cout << "# MaMa started @ " << ctime(&t) << endl;
    cout << "PROGRAM " << progName << endl;
    cout << "ARITY " << arity << endl << endl;
    cout << '%' << setw(10) << context->desc.r() << endl;
    cout << ':' << setw(10) << context->desc.i() << endl << endl;
    cout << "SYSTEM" << setw(5) << (context->desc.s() ? 3 : 2) << endl;
    cout << "VERSION" << setw(4) << static_cast<unsigned>(context->desc.v()) << endl << endl;
  }

  void Interpreter::printInstr(const Instruction &i) const {
    ostringstream ost;
    ost << setw(10) << i.lineno() << ' ';
    if (srcLines) {
      LineMap::const_iterator it = srcLines->find(i.lineno());
      if (srcLines->end() == it)
        throw ESrcNotInSync();
      cout << ost.str() << it->second << endl;
    }
    else {
      // progCnt-1 radit ce samo ako printInstr() slijedi odmah nakon
      // fetch(), inace upotrijebiti context->FptrToIndex(i.fptr())
      ost << ':' << (progCnt - 1) << ' ';
      string s;
      i.toString(s, *context);
      cout << ost.str() << s << endl;
    }
  }

  bool Interpreter::interact() {
    static const char *const prompt =
      "Step [[+/-]i] | setPc [[+/-/.]a] | %[...] [value] \? ";
    cout << prompt;
    countdown = step;
    for (String ans; getline(cin,ans); cout << prompt) {
      StrVec parts;
      StrVec::size_type pcnt;
      if (!(pcnt = ans.trimBoth().toUpper().tokenize(parts)) || parts[0] == "S" || parts[0] == "STEP") {
        if (pcnt <= 1)
          return true;
        else if (2 == pcnt) {
          bool ok = false;
          if (parts[1][0] == '+' || isdigit(parts[1][0])) {
            if (const U4 s = static_cast<U4>(strToS4(parts[1].c_str(),ok))) {
              countdown = s;
              return true;
            }
            else if (ok) {
              countdown = step = 0;
              return true;
            }
            continue;
          }
          else if (parts[1][0] == '-') {
            if (const S4 s = strToS4(parts[1].c_str(),ok)) {
              countdown = step = static_cast<U4>(-s);
              return true;
            }
            else if (ok) {
              countdown = step = 0;
              return true;
            }
            continue;
          }
          continue;
        }
        continue;
      }
      else if (parts[0] == "P" || parts[0] == "SETPC") {
        if (1 == pcnt)
          return false;
        else if (2 == pcnt) {
          bool ok = false;
          if (parts[1][0] == '+') {
            if (const U4 p = strToU4(parts[1].c_str() + 1, ok)) {
              if ((progCnt += p) >= context->desc.i())
                return false;
              files.first->seekg(context->IndexToFptr(progCnt));
              return true;
            }
            else if (ok)
              return true;
            continue;
          }
          else if (parts[1][0] == '-') {
            if (const U4 p = strToU4(parts[1].c_str() + 1, ok)) {
              if (p > progCnt)
                return false;
              files.first->seekg(context->IndexToFptr(progCnt -= p));
              return true;
            }
            else if (ok)
              return true;
            continue;
          }
          else if (isdigit(parts[1][0])) {
            if (const U4 p = strToU4(parts[1].c_str(),ok)) {
              if ((progCnt = p) >= context->desc.i())
                return false;
              files.first->seekg(context->IndexToFptr(progCnt));
              return true;
            }
            else if (ok) {
              files.first->seekg(context->IndexToFptr(progCnt = 0));
              return true;
            }
            continue;
          }
          else if (parts[1][0] == '.') {
            if (const U4 l = strToU4(parts[1].c_str() + 1, ok)) {
              if ((progCnt = context->LineNoToIndex(l)) >= context->desc.i())
                return false;
              files.first->seekg(context->IndexToFptr(progCnt));
              return true;
            }
            continue;
          }
          continue;
        }
        continue;
      }
      else if (parts[0] == "%") {
        if (1 == pcnt)
          regDump();
        continue;
      }
      else if (parts[0][0] == '%') {
        if (1 == pcnt) {
          bool ok = false;
          if (parts[0][1] == '%') {
            if (parts[0].size() == 2)
              regDump(regTemp);
            continue;
          }
          else if (const U4 r = strToU4(parts[0].c_str() + 1, ok)) {
            if (r < context->desc.r())
              regDump(r);
            continue;
          }
          else if (ok)
            regDump(0);
          continue;
        }
        else if (2 == pcnt) {
          bool ok = false;
          U4 r = regTemp;
          if (parts[0][1] == '%') {
            if (parts[0].size() != 2)
              continue;
          }
          else if ((r = strToU4(parts[0].c_str() + 1, ok)) || ok) {
            if (r >= context->desc.r())
              continue;
          }
          else
            continue;
          if (const U8 c = strToU8(parts[1].c_str(), ok))
            getReg(r) = c;
          else if (ok)
            getReg(r) = 0;
        }
      }
    }
    return false;
  }

  Interpreter::DynBindTable Interpreter::dynBindTable;
  Interpreter *root = 0;

}
