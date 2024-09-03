/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <iomanip>
#include <queue>
#include <random>
#include <unordered_map>
#include <utility>


using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;
enum class divisorType { POS_UNATE,
                         NEG_UNATE,
                         BI_NATE,
                         CONST0,
                         CONST1,
                         DUMMY };

// enum class resynType { ZERO_RESUB,
//                        ONE_RESUB,
//                        TWO_RESUB,
//                        THREE_RESUB,
//                        CONST,
//                        DUMMY
// };

struct Divisor {
    CirGate *gate;
    divisorType type;
    bool isNeg;  // is neg literal
    uint64_t pattern;
    // positive unate -> number of onset-minterm (f_l & f_on)
    // negtive unate -> number of offset-minterm (f_l & f_off)
    size_t Ones = 0;
};

// 2 binate divisors can be merged(AND) into one pos/neg unate divisorPair
struct DivisorPair {
    Divisor *divisor1;
    Divisor *divisor2;
    divisorType type;
    uint64_t pattern;
    size_t Ones = 0;
};

class CirMgr {
   public:
    CirMgr() {}
    ~CirMgr() {}

    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate *getGate(unsigned gid) const { return all[gid]; }
    string getFilename() const { return fileName; }
    int getIOsize() const { return PIs.size() + POs.size(); }
    int getAllsize() const { return all.size(); }

    // Member functions about circuit construction
    bool readCircuit(const string &);

    // Member functions about circuit optimization
    void sweep();
    void optimize();

    // Member functions about simulation
    void randomSim();
    void fileSim(ifstream &);
    void setSimLog(ofstream *logFile) { _simLog = logFile; }
    bool findPattern(CirGate *, vector<bool> &isUpdate);
    void simulate64times();
    // void writeLog(vector<uint32_t> &);
    void findPattern_DFS();
    void writeLog();
    bool isNoPattern = true;
    vector<int> inputId2Pattern;

    // Member functions about fraig
    void strash();
    void printFEC() const;
    void fraig();
    void genProofModel(SatSolver &s);
    bool ifFEC(SatSolver &solver, pair<CirGate *, bool> a, pair<CirGate *, bool> b);

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist();
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream &);
    void writeGate(ostream &, CirGate *);
    void DFS(CirGate *, vector<int> &);
    string getIOName(const CirGate *) const;

    // update Function
    void updateFanout();
    void updateUnusedForPI();
    void updateAIGs();
    void updateTrace();
    void updateFloats();
    void updateCircuit();
    void removeUnusedGate(vector<CirGate *> &array);

    // Member functions about windowing
    void doWindow();
    bool getWindow(CirGate *n, size_t cutSize, size_t distance, vector<CirGate *> &Iset,
                   vector<CirGate *> &Oset, vector<CirGate *> &Gset);
    bool findAllCostFreeExpansion(vector<CirGate *> &I, vector<CirGate *> &costFreeSet);
    void removeElement(vector<CirGate *> &I, CirGate *v);
    void removeTheSame(vector<CirGate *> &I);
    bool windowResubstitution(std::vector<CirGate *> &, std::vector<CirGate *> &, std::vector<CirGate *> &);
    bool findMFFC(CirGate *r, vector<CirGate *> &MFFC);
    void getFanoutCone(CirGate *r, vector<CirGate *> &TFO);
    bool resyn(set<size_t> &fOnset, set<size_t> &fOffset, vector<CirGate *> &divisor, vector<Divisor *> &collectList, vector<DivisorPair *> &collectPairList);

    // Member functions about cec
    void doCec(CirMgr* compareMgr);

   private:
    ofstream *_simLog;
    GateList all;  // index = gid
    GateList PIs;
    GateList POs;
    GateList AIGs;
    GateList ConstGates;
    GateList FloatGates;
    GateList UnusedGates;
    // store input name,ouput name(output index= inputsize+ 0,1,2.... )
    vector<string> ioname;
    // for DFS list
    size_t visitMark = 0;
    vector<int> trace;
    string fileName;
    vector<vector<pair<CirGate *, bool>>> SimGroups;  // true for negation
    // for Sim
    // each index correspond each PI pattern
    vector<uint64_t> inputPattern;
    int currWriteLine;
    int allWriteLine;
    // for SAT
    vector<Var> id2Var;
    vector<int> var2id;
    bool isSimulated = false;
    bool isStrashed = false;
};

#endif  // CIR_MGR_H
