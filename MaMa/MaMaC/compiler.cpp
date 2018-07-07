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
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <ctime>
#include <new>
using namespace std;

#include "compiler.hpp"

namespace excpt {

  struct EFileNotWritable : public Exception {
    explicit EFileNotWritable(const char *const fn) throw() : Exception() {
      expl = string("FILE NOT WRITABLE: ") + fn;
    }
    EFileNotWritable(const char *const file, const int line, const char *const fn) throw()
      : Exception(file,line) { expl += fn; }
  };

  struct ESyntaxError : public Exception {
    explicit ESyntaxError(const U4 lno, const char *const errDesc = 0) : Exception() {
      expl = "SYNTAX ERROR @ LINE ";
      ostringstream ost;
      ost << setw(10) << lno;
      expl += ost.str() + " : ";
      if (errDesc)
        expl += errDesc;
    }
  };

}

using namespace excpt;

namespace compiler {

  int Compiler::nextPhase() {
    static const char *const phases[] = {
      "procesiranje kompajlerskih direktiva",
      "leksicka i sintakticka analiza instrukcija",
      "racunanje translacijske tablice registara",
      "translatiranje registara u instrukcijama",
      "racunanje instrukcijskih file pointera",
      "mapiranje simbolickih labela na file pointere",
      "translatiranje labela u instrukcijama",
      "emitiranje bytecodea", // ovo ukljucuje sve: deskriptor, debugging zapise i instrukcije
      "zavrsna statistika"
    };
    ++phase;
    assert(static_cast<size_t>(phase) < sizeof(phases) / sizeof(*phases));
    if (cmdLineFlags.verbose)
      cout << "PHASE " << phase << " : " << phases[phase] << endl;
    return phase;
  }

  bool Compiler::nextLine() {
    for (bool more = true; more; ) {

      // ucitaj novu liniju ili EOF
      String s;
      if (!getline(source,s))
        return false;

      ++lineno;
      assert(lineno < 0x80000000);
      more = false;

      // eliminiraj komentare i bjeline i tokeniziraj
      // ako ima tokena, prvi token kapitaliziraj
      const String::size_type c = s.find_first_of('#');
      if (String::npos != c)
        s.erase(c);

      static const char *const incpltLine = "INCOMPLETE LINE";

      switch (s.trimBoth().tokenize(parsed)) {
        case 0:
          more = true;
          break;
        case 1:
          throw ESyntaxError(lineno, incpltLine);
        default:
          label.clear();
          if (isLabel(parsed[0],&label)) {
            if (labClassify(tellLabel(label)) == lcSpecial)
              throw ESyntaxError(lineno, "SPECIAL LABEL AS A LINE LABEL");
            parsed.erase(parsed.begin());  // izbacimo labelu iz linije
            if (parsed.size() < 2)
              throw ESyntaxError(lineno, incpltLine);
          }
          parsed[0].toUpper();
      }

    }
    return true;
  }

  Compiler::Compiler(const CmdLineFlags &clf, const char *const fn)
    : cmdLineFlags(clf), phase(-1), lineno(0), infile(fn), source(fn),
      desc(clf.debug), offset(static_cast<U4>(desc.size())), output(0)
  {
    assert(infile);
    if (!source)
      throw EFileNotReadable(infile);
    regMap[0] = 0; // nulti registar uvijek postoji i fiksan je
  }

  void Compiler::regResolve(Instruction* &i) {
    if (INC *ii = dynamic_cast<INC*>(i))
      ii->reg(regMap[ii->reg()]);
    else if (DECK *di = dynamic_cast<DECK*>(i))
      di->reg(regMap[di->reg()]);
    else if (SET *si = dynamic_cast<SET*>(i)) {
      si->lhs(regMap[si->lhs()]);
      U4 rhs;
      if (si->grhs(rhs))
        si->srhs(regMap[rhs]);
    }
    else if (CALL *ci = dynamic_cast<CALL*>(i)) {
      for (CALL::ArgVec::iterator it = ci->args().begin(); it != ci->args().end(); ++it)
        if (!it->first)
          it->second = static_cast<U8>(regMap[static_cast<U4>(it->second)]);
    }
  }

  void Compiler::offCalc(Instruction* &i) {
    i->fptr(offset);
    offset += static_cast<U4>(i->size());
  }

  void Compiler::labResolve(LabMap::value_type &lmp) {
    lmp.second = ivec[lmp.second]->fptr();
  }

  void Compiler::updateTgtLabel(Instruction* &i) {
    // odredi tip instrukcije
    DECK *di = 0;
    GOTO *const gi = dynamic_cast<GOTO*>(i);
    if (!gi)
      if (!(di = dynamic_cast<DECK*>(i)))
        return;
    // preskoci specijalne labele
    const string &tl = gi ? gi->tLabel : di->tLabel;
    if (labClassify(tellLabel(tl)) == lcSpecial)
      return;
    // provjeri postoji li labela i koja je
    const LabMap::iterator it = labMap.find(tl);
    if (it == labMap.end())
      throw ESyntaxError(i->lineno(), (tl + " UNKNOWN LABEL").c_str());
    gi ? gi->target(it->second) : di->target(it->second);
  }

  void Compiler::dbgRecEmit(Instruction* &i) {
    i->dbgRecEmit(*output);
  }

  void Compiler::instrEmit(Instruction* &i) {
    i->emit(*output);
  }

  void Compiler::instrCleanup(Instruction* &i) {
    delete i;
    i = 0;
  }

  void Compiler::compile() {
    
    const clock_t startTime = clock();

    /*** PHASE 0 ***/

    nextPhase();

    for (bool systSet = false, verSet = false, ok = false; nextLine(); ) {
      const Statement st = tellStmt(parsed[0]);

      U4 r;
      string l;

      switch (classify(st)) {

        case stclDirective:
          if (!label.empty())
            throw ESyntaxError(lineno, "LABELED DIRECTIVE");
          if (phase)
            throw ESyntaxError(lineno, "DIRECTIVE AFTER INSTRUCTION");

          switch (st) {

            case stmtSystem:
              if (systSet)
                throw ESyntaxError(lineno, "SYSTEM ALREADY SET");
              switch (strToU4(parsed[1].c_str(),ok)) {
                case 2:
                  desc.s(system2);
                  break;
                case 3:
                  desc.s(system3);
                  break;
                default:
                  throw ESyntaxError(lineno, "UNSUPPORTED SYSTEM");
              }
              systSet = true;
              break;

            case stmtVersion:
              if (verSet)
                throw ESyntaxError(lineno, "VERSION ALREADY SET");
              if (strToU4(parsed[1].c_str(),ok) || !ok)
                throw ESyntaxError(lineno, "UNSUPPORTED VERSION");
              verSet = true;
              break;

            default:
              throw EInternal(__FILE__, __LINE__);
          }
          break;

        case stclInstruction:
          if (!phase) {
            if (!systSet)
              throw ESyntaxError(lineno, "SYSTEM NOT SPECIFIED");

    /*** PHASE 1 ***/
            
            nextPhase();
          }

          // registriramo labelu buduce instrukcije
          if (!label.empty()) {
            if (labMap.find(label) != labMap.end())
              throw ESyntaxError(lineno, (label + " DUPLICATED LABEL").c_str());
            labMap[label] = U4(ivec.size());
          }

          switch (st) {

            case stmtInc:
              if (parsed.size() != 2)
                throw ESyntaxError(lineno, "INC :: WRONG NUMBER OF ARGUMENTS");
              if (!isRegister(parsed[1], &r))
                throw ESyntaxError(lineno, "INC :: INVALID REGISTER");
              if (INC *ii = new INC(lineno,r)) {
                ivec.push_back(ii);
                if (r < regTemp)
                  regMap[r] = r;
              }
              else
                throw bad_alloc();
              break;

            case stmtGoto:
              if (parsed.size() != 2)
                throw ESyntaxError(lineno, "GOTO :: WRONG NUMBER OF ARGUMENTS");
              if (!isLabel(parsed[1], &l))
                throw ESyntaxError(lineno, "GOTO :: INVALID LABEL");
              if (GOTO *gi = new GOTO(lineno, tellLabel(l))) {
                gi->tLabel = l;
                ivec.push_back(gi);
              }
              else
                throw bad_alloc();
              break;

            case stmtDec:
            case stmtDek:
              if (parsed.size() != 3)
                throw ESyntaxError(lineno, "DEC/DEK :: WRONG NUMBER OF ARGUMENTS");
              if (!isRegister(parsed[1], &r))
                throw ESyntaxError(lineno, "DEC/DEK :: INVALID REGISTER");
              if (!isLabel(parsed[2], &l))
                throw ESyntaxError(lineno, "DEC/DEK :: INVALID LABEL");
              if (DECK *di = new DECK(lineno, desc.s(), stmtDek == st, r, tellLabel(l))) {
                di->tLabel = l;
                ivec.push_back(di);
                if (r < regTemp)
                  regMap[r] = r;
              }
              else
                throw bad_alloc();
              break;

            case stmtSet:
              if (parsed.size() != 3)
                throw ESyntaxError(lineno, "SET :: WRONG NUMBER OF ARGUMENTS");
              if (!isRegister(parsed[1], &r))
                throw ESyntaxError(lineno, "SET :: INVALID TARGET REGISTER");
              SET *si;
              union {
                U8 c;
                U4 r;
              } a;
              if (isRegister(parsed[2], &a.r)) {
                if ((si = new SET(lineno, r, a.r))) {
                  ivec.push_back(si);
                  if (a.r < regTemp)
                    regMap[a.r] = a.r;
                }
                else
                  throw bad_alloc();
              }
              else {
                a.c = strToU8(parsed[2].c_str(), ok);
                if (ok) {
                  if ((si = new SET(lineno, r, a.c)))
                    ivec.push_back(si);
                  else
                    throw bad_alloc();
                }
                else
                  throw ESyntaxError(lineno, "SET :: RHS NEITHER CONSTANT NOR REGISTER");
              }
              if (r < regTemp)
                regMap[r] = r;
              break;

            case stmtCall:
              StrVec::size_type ps, rest;
              ps = parsed.size();
              if (isRegister(parsed[1], &r))
                rest = 2;
              else if (ps >= 3 && isRegister(parsed[2], &r)) {
                l = parsed[1];
                rest = 3;
              }
              else
                throw ESyntaxError(lineno, "CALL :: FIRST ARGUMENT NOT REGISTER");
              if (l.size() >= 256)
                throw ESyntaxError(lineno, "CALL :: |MaMa_PROG_NAME| >= 256");

              if (CALL *ci = new CALL(lineno,l)) {
                ci->add(r);
                if (r < regTemp)
                  regMap[r] = r;
                for (; rest < ps; ++rest) {
                  if (isRegister(parsed[rest], &r)) {
                    ci->add(r);
                    if (r < regTemp)
                      regMap[r] = r;
                  }
                  else {
                    const U8 c = strToU8(parsed[rest].c_str(), ok);
                    if (ok)
                      ci->add(c);
                    else
                      throw ESyntaxError(lineno, "CALL :: ARGUMENT NEITHER CONSTANT NOR REGISTER");
                  }
                }
                ci->finalize();
                ivec.push_back(ci);
              }
              else
                throw bad_alloc();
              break;

            default:
              throw EInternal(__FILE__, __LINE__);
          }
          break;

        default:
          throw ESyntaxError(lineno, "UNSUPPORTED INSTRUCTION/DIRECTIVE");
      }
    }

    if (!phase)
      throw ESyntaxError(lineno, "PROGRAM EMPTY");

    source.close();
    desc.i(static_cast<U4>(ivec.size()));

    /*** PHASE 2 ***/

    nextPhase();

    U4 regCnt = 0;
    for (RegMap::iterator it = regMap.begin(); it != regMap.end(); ++it)
      it->second = regCnt++;

    desc.r(U4(regMap.size()));
    regMap[regTemp] = regTemp;

    /*** PHASE 3 ***/

    nextPhase();

    for_each(ivec.begin(), ivec.end(), InstructionOp(*this,&Compiler::regResolve));

    /*** PHASE 4 ***/

    nextPhase();

    if (cmdLineFlags.debug)
      offset += static_cast<U4>(ivec.size() * Instruction::dbgRecSize());

    for_each(ivec.begin(), ivec.end(), InstructionOp(*this,&Compiler::offCalc));

    /*** PHASE 5 ***/

    nextPhase();

    for_each(labMap.begin(), labMap.end(), LabelOp(*this,&Compiler::labResolve));
    
    /*** PHASE 6 ***/

    nextPhase();

    for_each(ivec.begin(), ivec.end(), InstructionOp(*this,&Compiler::updateTgtLabel));

    /*** PHASE 7 ***/

    nextPhase();

    string outfile = infile;
    const string::size_type ld = outfile.find_last_of('.');
    if (ld != string::npos)
      outfile.erase(ld);
    outfile += ".MMb";
    output = new ofstream(outfile.c_str(), ios_base::out | ios_base::trunc | ios_base::binary);

    if (!output)
      throw bad_alloc();
    if (!*output)
      throw EFileNotWritable(outfile.c_str());

    // emitiraj deskriptor
    desc.emit(*output);

    // emitiraj debugging zapise
    if (cmdLineFlags.debug)
      for_each(ivec.begin(), ivec.end(), InstructionOp(*this,&Compiler::dbgRecEmit));

    // emitiraj instrukcije
    for_each(ivec.begin(), ivec.end(), InstructionOp(*this,&Compiler::instrEmit));

    // zatvori output i pocisti instrukcije
    delete output;
    output = 0;
    for_each(ivec.begin(), ivec.end(), Compiler::instrCleanup);

    /*** PHASE 8 ***/

    nextPhase();

    if (cmdLineFlags.verbose) {
      cout << "Total compile time: " <<
        (double(clock() - startTime) / CLOCKS_PER_SEC) <<
        " s" << endl << endl;
      cout << "Bytecode File: " << outfile << endl;
      cout << "Bytecode Size: " << offset << " B" << endl;
      cout << "Bytecode Version: " << unsigned(desc.v()) << endl;
      cout << "Native Endian: " << (desc.e() ? "big" : "little") << endl;
      cout << "MaMa System: " << (desc.s() ? 3 : 2) << endl;
      cout << "Debugging: " << boolalpha << cmdLineFlags.debug << endl;
      cout << "Lines: " << lineno << endl;
      cout << "Instructions: " << desc.i() << endl;
      cout << "Registers (-%): " << desc.r() << endl;
      cout << "Labels (-!:): " << labMap.size() << endl;
    }

  }

  Compiler *comp = 0;

}
