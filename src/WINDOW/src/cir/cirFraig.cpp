/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <map>
#include <sstream>

#include "cirGate.h"
#include "cirMgr.h"
#include "myHashMap.h"
#include "sat.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void CirMgr::strash() {
    if (isStrashed) {
        cout << "Error: strash operation has already been performed !!" << endl;
    }
    map<string, CirGate*> m;

    updateTrace();
    // build a hashmap
    for (size_t i = 0; i < trace.size(); ++i) {
        CirGate* curr = all[trace[i]];
        stringstream ss;
        if (curr != 0 && curr->type == AIG_GATE) {
            // gen key
            string key;
            if (curr->fanin0id < curr->fanin1id) {
                ss << curr->fanin0id << curr->fanin0cp << "2" << curr->fanin1id << curr->fanin1cp;
            } else if (curr->fanin0id > curr->fanin1id) {
                ss << curr->fanin1id << curr->fanin1cp << "2" << curr->fanin0id << curr->fanin0cp;
            } else if (!curr->fanin0cp) {
                ss << curr->fanin0id << curr->fanin0cp << "2" << curr->fanin1id << curr->fanin1cp;
            } else {
                ss << curr->fanin1id << curr->fanin1cp << "2" << curr->fanin0id << curr->fanin0cp;
            }

            ss >> key;

            // check key exist & merge
            if (m.find(key) != m.end()) {
                CirGate* need = m[key];
                // delete curr
                for (size_t j = 0; j < curr->fanoutid.size(); ++j) {
                    if (!all[curr->fanoutid[j].first]) continue;

                    if (all[curr->fanoutid[j].first]->fanin0id == curr->id) {
                        all[curr->fanoutid[j].first]->fanin0id = need->id;

                    } else if (all[curr->fanoutid[j].first]->fanin1id == curr->id) {
                        all[curr->fanoutid[j].first]->fanin1id = need->id;
                    }
                }
                cout << "Strashing: " << need->id << " merging " << curr->id << "..." << endl;
                all[curr->id] = 0;
                delete curr;
                curr = NULL;

            }
            // don't exist
            else {
                m[key] = curr;
            }
        }
    }

    // update AIG
    updateAIGs();

    // update undefind gates (AIG)
    updateFloats();

    // updateFanout
    updateFanout();
    isStrashed = true;

    // update DFS
    updateTrace();
}

void CirMgr::fraig() {
    SatSolver solver;
    solver.initialize();

    // construct circuit info
    genProofModel(solver);
    vector<int> later;
    // find FEC
    int numPattern = 0;
    inputPattern.clear();
    inputPattern = vector<uint64_t>(PIs.size());
    // cout << "TOTAL #FEC GROUP = " << SimGroups.size() << endl;
    int sizeNum = 0;
    int prevSize = 0;
    while (SimGroups.size() != 0) {
        for (int i = 0; i < SimGroups.size(); ++i) {

            if (SimGroups[i].size() <= 1) {
                SimGroups.erase(SimGroups.begin() + i);
                cout << "\rTotal #FEC Group = " << SimGroups.size();
                --i;
                continue;
            }
            for (size_t j = 1; j < SimGroups[i].size(); ++j) {
                // a XOR b
                if (ifFEC(solver, SimGroups[i][0], SimGroups[i][j])) {
                    // merge
                    // cout << "Fraig: ";
                    // if (SimGroups[i][0].second) cout << "!";
                    // cout << SimGroups[i][0].first->id << " merging ";
                    // if (SimGroups[i][j].second) cout << "!";
                    // cout << SimGroups[i][j].first->id << "..." << endl;

                    CirGate* need = SimGroups[i][0].first;
                    CirGate* curr = SimGroups[i][j].first;
                    if ((need->fanin0id == curr->id) && need->id != 0) {
                        CirGate* temp = need;
                        need = curr;
                        curr = temp;
                    }
                    // delete curr
                    for (size_t j = 0; j < curr->fanoutid.size(); ++j) {
                        if (!all[curr->fanoutid[j].first]) continue;

                        if (all[curr->fanoutid[j].first]->fanin0id == curr->id) {
                            all[curr->fanoutid[j].first]->fanin0id = need->id;
                            if (SimGroups[i][j].second) all[curr->fanoutid[j].first]->fanin0cp = !all[curr->fanoutid[j].first]->fanin0cp;

                        } else if (all[curr->fanoutid[j].first]->fanin1id == curr->id) {
                            all[curr->fanoutid[j].first]->fanin1id = need->id;
                            if (SimGroups[i][j].second) all[curr->fanoutid[j].first]->fanin1cp = !all[curr->fanoutid[j].first]->fanin1cp;
                        }
                    }
                    // all[curr->id] = 0;
                    // delete curr;
                    // curr = NULL;
                    SimGroups[i].erase(SimGroups[i].begin() + j);
                    --j;
                }
                // not equal -> inputpattern ++
                else {
                    // cout << SimGroups[i][0].first->id << " " << SimGroups[i][j].first->id << " is not equal" << endl;
                    ++numPattern;
                    if (numPattern == 64) {
                        simulate64times();
                        inputPattern.clear();
                        inputPattern = vector<uint64_t>(PIs.size());
                        cout << "Updating by UNSAT... " << endl;
                        cout << "Total #FEC Group = " << SimGroups.size() << endl;
                        numPattern = 0;
                        // restart again
                        i = 0;
                        // update AIG
                        updateAIGs();

                        // updateFanout
                        updateFanout();

                        if (SimGroups.size() == prevSize) {
                            ++sizeNum;
                        } else {
                            sizeNum = 0;
                        }
                        if (sizeNum == 10) {
                            for (int i = 0; i < SimGroups.size(); ++i) {
                                SimGroups[i].erase(SimGroups[i].begin());
                            }
                            cout << "stuck!!" << endl;
                        }
                        if (sizeNum == 20) {
                            SimGroups.clear();
                            cout << "force to stop ! " << endl;
                            break;
                        }
                        prevSize = SimGroups.size();

                        // break;
                    }
                }

                if (SimGroups[i].size() <= 1) {
                    SimGroups.erase(SimGroups.begin() + i);
                    // cout << "\rTotal #FEC Group = " << SimGroups.size();
                    continue;
                }
            }
            if (sizeNum == 20) {
                // SimGroups.clear();
                // cout << "force to stop ! " << endl;
                break;
            }
        }
        if (sizeNum == 20) {
            // SimGroups.clear();
            // cout << "force to stop ! " << endl;
            break;
        }
    }
    // cout << "END..." << endl;
    cout << "Total #FEC Group = " << SimGroups.size() << endl;
    // update UnusedList (only PI)
    updateUnusedForPI();

    // // update DFS
    updateTrace();

    // update AIGs
    updateAIGs();
    
    // update fanout
    updateFanout();

    isSimulated = false;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void CirMgr::genProofModel(SatSolver& solver) {
    // vector<Var> id2Var;
    // vector<int> var2id;

    Var newV = solver.newVar();
    id2Var = vector<Var>(all.size());
    var2id = vector<int>(newV + all.size());
    id2Var[0] = newV;
    var2id[newV] = 0;
    solver.assertProperty(id2Var[0], false);

    for (size_t i = 2; i < all.size(); ++i) {
        if (all[i] == 0) continue;
        Var newV = solver.newVar();
        id2Var[all[i]->id] = newV;
        var2id[newV] = all[i]->id;
    }

    for (size_t i = PIs.size() + 1; i < all.size(); ++i) {
        if (all[i] == 0) continue;
        if (all[i]->type == PO_GATE) {
            solver.addAigCNF(id2Var[all[i]->id], id2Var[all[i]->fanin0id], all[i]->fanin0cp, id2Var[all[i]->fanin0id], all[i]->fanin0cp);
        } else if (all[i]->type == AIG_GATE) {
            solver.addAigCNF(id2Var[all[i]->id], id2Var[all[i]->fanin0id], all[i]->fanin0cp, id2Var[all[i]->fanin1id], all[i]->fanin1cp);
        }
    }
}

// return true = equvilent
bool CirMgr::ifFEC(SatSolver& solver, pair<CirGate*, bool> a, pair<CirGate*, bool> b) {
    Var newV = solver.newVar();
    solver.addXorCNF(newV, id2Var[a.first->id], a.second, id2Var[b.first->id], b.second);
    solver.assumeRelease();
    solver.assumeProperty(newV, true);
    bool result;
    result = solver.assumpSolve();
    // cout << "size : " << all.size() << endl;
    // solver getValue
    // cout << "a: " << a.first->id << " b: " << b.first->id << endl;
    if (result) {
        for (size_t i = 0; i < PIs.size(); ++i) {
            int v = solver.getValue(id2Var[PIs[i]->id]);
            if (v == 0)
                inputPattern[i] = inputPattern[i] << 1;
            else if (v == 1) {
                inputPattern[i] = inputPattern[i] << 1;
                inputPattern[i] += 1;
            }
            // cout << v;
        }
        // cout << "  output:  ";
        // for (size_t i = 0; i < POs.size(); ++i) {
        //     cout << solver.getValue(id2Var[POs[i]->id]);
        // }
        // cout << " 1296: " << solver.getValue(id2Var[1296]) << " 1307: " << solver.getValue(id2Var[1307]) << " 1465: " << solver.getValue(id2Var[1465]) << " 1560: " << solver.getValue(id2Var[1560]) << " 1579: " << solver.getValue(id2Var[1579]) << endl;
        // cout << endl;
    }
    return !result;
}