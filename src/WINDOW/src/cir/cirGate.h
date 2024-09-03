/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>

#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate {
   public:
    CirGate(int id, GateType type, int fanin0id, int fanin1id, bool fanin0cp, bool fanin1cp)
        : id(id), type(type), fanin0id(fanin0id), fanin1id(fanin1id), fanin0cp(fanin0cp), fanin1cp(fanin1cp) {
        // this->cirMgr = cirMgr;
    }
    ~CirGate() {}
    friend class CirMgr;
    // Basic access methods
    string getTypeStr() const { return ""; }
    unsigned getLineNo() const { return 0; }

    // Printing functions
    void printGate() const;
    void reportGate() const;
    void reportFanin(int level) const;
    void doReportFanin(int level, string space, bool cp, vector<bool>& markList) const;
    void reportFanout(int level) const;
    void doReportFanout(int level, string space, bool cp, vector<bool>& markList) const;
    string getTypeName() const;
    GateType getType() const { return type; }
    bool isAig() const { return type == AIG_GATE; }

   private:
    int id;
    GateType type;
    int fanin0id;
    int fanin1id;
    bool fanin0cp;
    bool fanin1cp;
    vector<pair<int, bool>> fanoutid;
    // for simulate
    uint64_t pattern = 0;

    // for DFS/BFS (visited)
    size_t visited = 0;
    size_t predecessorId = 0;
};

#endif  // CIR_GATE_H
