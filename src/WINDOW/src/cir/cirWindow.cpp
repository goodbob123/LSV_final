#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

#define RED_TEXT "\033[1;31m"
#define GREEN_TEXT "\033[1;32m"
#define YELLOW_TEXT "\033[1;33m"
#define BLUE_TEXT "\033[1;34m"
#define MAGENTA_TEXT "\033[1;35m"
#define CYAN_TEXT "\033[1;36m"
#define WHITE_TEXT "\033[1;37m"
#define RESET_COLOR "\033[0m"

using namespace std;

/****************************************************************************************/
/*   Boolean Rewriting Strikes Back: Reconvergence-Driven Windowing Meets Resynthesis   */
/****************************************************************************************/

void CirMgr::doWindow() {
    cerr << "-------------------[ DO WINDOW START ]---------------------" << endl;

    size_t beforeAIGsize = AIGs.size();
    // preprocess
    sweep();
    size_t AfterSweep = AIGs.size();
    optimize();
    size_t AfterOptimize = AIGs.size();

    size_t cutSize = 6;
    size_t distance = 30;

    for (size_t i = AIGs.size() - 1; i != SIZE_MAX; --i) {
        vector<CirGate*> Iset, Oset, Gset;
        if (i >= AIGs.size()) continue;
        if (getWindow(AIGs[i], cutSize, distance, Iset, Oset, Gset)) {
            cerr << RED_TEXT << "AIG[" << setw(4) << AIGs[i]->id << "] find window" << RESET_COLOR << endl;
            if (windowResubstitution(Iset, Oset, Gset)) {
                cerr << GREEN_TEXT << "AIG[" << setw(4) << AIGs[i]->id << "] resyn" << RESET_COLOR << endl;
            }
        }
    }
    cerr << "-------------------[ DO WINDOW FINISH ]---------------------" << endl;
    cerr << setw(16) << "beforeAIGsize: " << beforeAIGsize << endl;
    cerr << setw(16) << "afterSweep: " << AfterSweep << endl;
    cerr << setw(16) << "aterOptimize: " << AfterOptimize << endl;
    cerr << setw(16) << "afterAIGsize: " << AIGs.size() << endl;


    // output result
    size_t pos = fileName.rfind("ex");
        string result = fileName.substr(pos + 2);
        size_t dotPos = result.find('.');
        if (dotPos != std::string::npos) {
            result.erase(dotPos);
        }
    string file = "./LSV_data/result/" + result + ".txt";
    
    cerr << "output file: " << file << endl;
    ofstream outputFile(file);
    if (outputFile.is_open()) {
        outputFile << setw(16) << "beforeAIGsize: " << beforeAIGsize << endl;
        outputFile << setw(16) << "afterSweep: " << AfterSweep << endl;
        outputFile << setw(16) << "aterOptimize: " << AfterOptimize << endl;
        outputFile << setw(16) << "afterAIGsize: " << AIGs.size() << endl;
        outputFile.close();
    } else
        cerr << "Unable to open file";
}

bool CirMgr::findAllCostFreeExpansion(vector<CirGate*>& I, vector<CirGate*>& costFreeSet) {
    for (size_t i = 0; i < I.size(); ++i) {
        if (I[i]->type == AIG_GATE) {
            if (!((getGate(I[i]->fanin0id)->visited != visitMark) && (getGate(I[i]->fanin1id)->visited != visitMark)))
                costFreeSet.push_back(I[i]);
        }
    }
    return (costFreeSet.size() > 0);
}

void CirMgr::removeElement(vector<CirGate*>& I, CirGate* v) {
    for (size_t i = 0; i < I.size(); ++i) {
        if (I[i]->id == v->id) {
            I.erase(I.begin() + i);
            break;
        }
    }
}

void CirMgr::removeTheSame(vector<CirGate*>& I) {
    for (size_t i = 0; i < I.size(); ++i) {
        for (size_t j = i + 1; j < I.size(); ++j) {
            if (I[i]->id == I[j]->id) {
                I.erase(I.begin() + j);
                break;
            }
        }
    }
}

/*
[ Algorithm S (Heuristic Boolean Resynthesis) ]

    The inputs
    to this algorithm are the target on-set and off-set functions
    f on , f off and a set D = {d_1 , . . . , d_r } of divisors associated
    with divisor functions f d_1 , . . . , f d_r .

*/
// return true = resyn , false = not resyn
bool CirMgr::resyn(set<size_t>& fOnset, set<size_t>& fOffset, vector<CirGate*>& divisor,
                   vector<Divisor*>& collectList, vector<DivisorPair*>& collectPairList) {
    vector<Divisor*> candidateDivisor;

    auto countOnes = [](uint64_t pattern, set<size_t>& careSet) {
        size_t count = 0;
        for (size_t k = 0; k < 64; ++k) {
            if ((pattern >> k & 1) & (careSet.find(k) != careSet.end())) ++count;
        }
        return count;
    };
    // [ S1 ]
    /*
    Check if f on = 0 or if f off = 0 . If so, return
    the constant 0 or 1
    */
    if (fOnset.size() == 0) {
        Divisor* d = new Divisor{getGate(0), divisorType::CONST0, false, 0};
        collectList.emplace_back(d);
        return true;
    }
    if (fOffset.size() == 0) {
        Divisor* d = new Divisor{getGate(0), divisorType::CONST1, false, 0};
        collectList.emplace_back(d);
        return true;
    }

    // init divisor
    for (const auto d : divisor) {
        Divisor* dPos = new Divisor{d, divisorType::DUMMY, false, d->pattern};
        Divisor* dneg = new Divisor{d, divisorType::DUMMY, true, ~(d->pattern)};
        candidateDivisor.emplace_back(dPos);
        candidateDivisor.emplace_back(dneg);
    }

    // [ S2 ]
    /*
    For each divisor d and its negation ¬d ,
    check if they are positive or negative unate. If both of them
    are not unate, classify d as binate.
    (Note. positive unate means f_d & f_off = 0, vise versa)
    */
    for (const auto d : candidateDivisor) {
        bool isPosUnate = true, isNegUnate = true;
        for (size_t i = 0; i < 64; ++i) {
            if (d->pattern >> i & 1) {
                if (fOffset.find(i) != fOffset.end()) isPosUnate = false;
                if (fOnset.find(i) != fOnset.end()) isNegUnate = false;
            }
        }
        d->type = isPosUnate ? divisorType::POS_UNATE : isNegUnate ? divisorType::NEG_UNATE
                                                                   : divisorType::BI_NATE;
    }

    // [ S3 ]
    /*
    [0-resub.] Check the collected lists of positive and negative
    unate literals for an 0-resub and return it if found.
    */
    for (size_t i = 0; i < candidateDivisor.size(); i = i + 2) {
        if (candidateDivisor[i]->type == divisorType::POS_UNATE && candidateDivisor[i + 1]->type == divisorType::NEG_UNATE) {
            collectList.emplace_back(candidateDivisor[i]);
            return true;
        }
        if (candidateDivisor[i]->type == divisorType::NEG_UNATE && candidateDivisor[i + 1]->type == divisorType::POS_UNATE) {
            collectList.emplace_back(candidateDivisor[i + 1]);
            return true;
        }
    }

    // [ S4 ]
    /*
    [Sort unate literals.] Sort the positive unate literals by
    the number of on-set minterms O NES (f l ∧ f on ) , and sort the
    negative unate literals by the number of off-set minterms
    O NES (f l ∧ f off ).
    */
    vector<Divisor*> posUnateList, negUnateList, binateList;
    // count ONEs
    for (const auto& d : candidateDivisor) {
        if (d->type == divisorType::POS_UNATE) {
            d->Ones = countOnes(d->pattern, fOnset);
            posUnateList.emplace_back(d);
        } else if (d->type == divisorType::NEG_UNATE) {
            d->Ones = countOnes(d->pattern, fOffset);
            negUnateList.emplace_back(d);
        } else if (d->type == divisorType::BI_NATE) {
            binateList.emplace_back(d);
        }
    }

    // sort (descending)
    sort(posUnateList.begin(), posUnateList.end(), [](const Divisor* a, const Divisor* b) { return a->Ones > b->Ones; });
    sort(negUnateList.begin(), negUnateList.end(), [](const Divisor* a, const Divisor* b) { return a->Ones > b->Ones; });

    // [ S5 ]
    /*
    [1-resub.] Enumerate pairs (l1 , l2 ) of positive unate literals
    to find an OR-type 1-resub. With the order sorted in S4, the
    enumeration can be terminated earlier if we know
    O NES (f_l1 ∧ f_on ) + O NES (f_l2 ∧ f_on ) < O NES (f_on )
    for the rest of the list. Similarly, enumerate pairs of negative
    unate literals to find an AND-type 1-resub and return it if
    found.
    */
    // OR-type 1-resub (posUnate)
    for (size_t i = 0; i < posUnateList.size() - 1; ++i) {
        if (posUnateList.size() == 0) break;
        for (size_t j = i + 1; j < posUnateList.size(); ++j) {
            Divisor *d1 = posUnateList[i], *d2 = posUnateList[j];
            if (d1->Ones + d2->Ones >= fOnset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOnset) == fOnset.size() && countOnes(~pattern, fOnset) == 0) {
                    collectList.emplace_back(d1);
                    collectList.emplace_back(d2);
                    cerr << "find 1-resub, posUnate" << endl;
                    return true;
                }
            }
        }
    }

    // AND-type 1-resub (negUnate)
    for (size_t i = 0; i < negUnateList.size() - 1; ++i) {
        if (negUnateList.size() == 0) break;
        for (size_t j = i + 1; j < negUnateList.size(); ++j) {
            Divisor *d1 = negUnateList[i], *d2 = negUnateList[j];
            if (d1->Ones + d2->Ones >= fOffset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOffset) == fOffset.size() && countOnes(~pattern, fOffset) == 0) {
                    collectList.emplace_back(d1);
                    collectList.emplace_back(d2);
                    cerr << "find 1-resub, negUnate" << endl;
                    cerr << endl;
                    return true;
                }
            }
        }
    }

    // [ S6 ]
    /*
    [Collect and sort unate pairs.] For each pair of binate
    divisors d1 , d2 , test the unateness of combining them using an
    AND gate and with all the four possibilities of negations.
    Collect the positive and negative unate pairs and sort them using the same method
    as in S4.
    */
    vector<DivisorPair*> posUnatePairList, negUnatePairList;
    for (size_t i = 0; i < binateList.size() - 1; ++i) {
        if (binateList.size() == 0) break;
        for (size_t j = i + 1; j < binateList.size(); ++j) {
            Divisor *d1 = binateList[i], *d2 = binateList[j];
            // AND to check pos unate
            uint64_t pattern = d1->pattern & d2->pattern;
            if (pattern == 0) continue;
            bool isPosUnate = true, isNegUnate = true;
            for (size_t k = 0; k < 64; ++k) {
                if (pattern >> k & 1) {
                    if (fOffset.find(k) != fOffset.end()) isPosUnate = false;
                    if (fOnset.find(k) != fOnset.end()) isNegUnate = false;
                }
            }
            if (isPosUnate) {
                DivisorPair* d = new DivisorPair{d1, d2, divisorType::POS_UNATE, pattern};
                posUnatePairList.emplace_back(d);
            } else if (isNegUnate) {
                DivisorPair* d = new DivisorPair{d1, d2, divisorType::NEG_UNATE, pattern};
                negUnatePairList.emplace_back(d);
            }
        }
    }

    // count ONEs
    for (const auto& d : posUnatePairList) {
        for (size_t i = 0; i < 64; ++i) {
            d->Ones = countOnes(d->pattern, fOnset);
        }
    }
    for (const auto& d : negUnatePairList) {
        for (size_t i = 0; i < 64; ++i) {
            d->Ones = countOnes(d->pattern, fOffset);
        }
    }

    // sort (descending)
    sort(posUnatePairList.begin(), posUnatePairList.end(), [](const DivisorPair* a, const DivisorPair* b) { return a->Ones > b->Ones; });
    sort(negUnatePairList.begin(), negUnatePairList.end(), [](const DivisorPair* a, const DivisorPair* b) { return a->Ones > b->Ones; });

    // [ S7 ]
    /*
    [ 2 - and 3-resub] Similar to S5, try to find a 2-resub by
    combining a unate literal and a unate pair. Then, try to find a
    3-resub by combining two unate pairs.
    */
    // 2-resub
    // OR-type (posUnate)
    for (size_t i = 0; i < posUnateList.size(); ++i) {
        if (posUnateList.size() == 0) break;
        for (size_t j = 0; j < posUnatePairList.size(); ++j) {
            Divisor* d1 = posUnateList[i];
            DivisorPair* d2 = posUnatePairList[j];
            if (d1->Ones + d2->Ones >= fOnset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOnset) == fOnset.size() && countOnes(~pattern, fOnset) == 0) {
                    collectList.emplace_back(d1);
                    collectPairList.emplace_back(d2);
                    cerr << "find 2-resub, posUnate" << endl;
                    return true;
                }
            }
        }
    }

    // 2-resub
    // AND-type (negUnate)
    for (size_t i = 0; i < negUnateList.size(); ++i) {
        if (negUnateList.size() == 0) break;
        for (size_t j = 0; j < negUnatePairList.size(); ++j) {
            Divisor* d1 = negUnateList[i];
            DivisorPair* d2 = negUnatePairList[j];
            if (d1->Ones + d2->Ones >= fOffset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOffset) == fOffset.size()) {
                    collectList.emplace_back(d1);
                    collectPairList.emplace_back(d2);
                    cerr << "find 2-resub, negUnate" << endl;
                    return true;
                }
            }
        }
    }

    // 3-resub
    // OR-type (posUnate)
    for (size_t i = 0; i < posUnatePairList.size() - 1; ++i) {
        if (posUnatePairList.size() == 0) break;
        for (size_t j = i + 1; j < posUnatePairList.size(); ++j) {
            DivisorPair* d1 = posUnatePairList[i];
            DivisorPair* d2 = posUnatePairList[j];
            if (d1->Ones + d2->Ones >= fOnset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOnset) == fOnset.size()) {
                    collectPairList.emplace_back(d1);
                    collectPairList.emplace_back(d2);
                    cerr << "find 3-resub, posUnate" << endl;
                    return true;
                }
            }
        }
    }

    // 3-resub
    // AND-type (negUnate)
    for (size_t i = 0; i < negUnatePairList.size() - 1; ++i) {
        if (negUnatePairList.size() == 0) break;
        for (size_t j = i + 1; j < negUnatePairList.size(); ++j) {
            DivisorPair* d1 = negUnatePairList[i];
            DivisorPair* d2 = negUnatePairList[j];
            if (d1->Ones + d2->Ones >= fOffset.size()) {
                uint64_t pattern = d1->pattern | d2->pattern;
                if (countOnes(pattern, fOffset) == fOffset.size()) {
                    collectPairList.emplace_back(d1);
                    collectPairList.emplace_back(d2);
                    cerr << "find 3-resub, negUnate" << endl;
                    return true;
                }
            }
        }
    }
    return false;
}

/*
[ Algorithm W (Window construction) ]

Given a node p , called
pivot, in a GIG N and two integers k and l , called cut size and
distance, this algorithm computes a small-scale window in the
local neighborhood of p with at most k inputs and potentially
multiple outputs that reaches a reconvergence in at most l
steps. The window is characterized by a triple (I, O, G) , of
local inputs I, local outputs O, and inner nodes G.

*/
// distance(l), and cutSize(k)
bool CirMgr::getWindow(CirGate* n, size_t cutSize, size_t distance, vector<CirGate*>& Iset,
                       vector<CirGate*>& Oset, vector<CirGate*>& Gset) {
    updateFanout();
    ++visitMark;
    n->visited = visitMark;

    // [ W1 ]
    /*
    Use breadth-first
    search to identify a node m , called meet, reachable from two
    fanins of p in at most l steps and add all nodes on the two
    paths from p to m to G.
    */

    {
        queue<CirGate*> queue;
        CirGate *fanin0, *fanin1, *meet = nullptr, *otherPath;
        queue.push(n);
        unordered_map<size_t, size_t> gateId2Distance;
        gateId2Distance[n->id] = 0;

        while (!queue.empty()) {
            CirGate* g = queue.front();
            queue.pop();
            g->visited = visitMark;
            if (g->type == PI_GATE) break;
            if (g->type == CONST_GATE) break;
            if (gateId2Distance[g->id] > distance) break;

            // add fanin to queue
            fanin0 = getGate(g->fanin0id);
            if (fanin0->visited != visitMark) {
                queue.push(fanin0);
                fanin0->predecessorId = g->id;
                gateId2Distance[fanin0->id] = gateId2Distance[g->id] + 1;
            }
            // find meet!
            else {
                meet = fanin0;
                otherPath = g;
                break;
            }
            fanin1 = getGate(g->fanin1id);
            if (fanin1->visited != visitMark) {
                queue.push(fanin1);
                fanin1->predecessorId = g->id;
                gateId2Distance[fanin1->id] = gateId2Distance[g->id] + 1;
            } else {
                meet = fanin1;
                otherPath = g;
                break;
            }
        }
        if (meet == nullptr) {
            return false;
        }

        // find 2 path from meet to n
        vector<CirGate*> path1, path2;
        while (meet->id != n->id) {
            path1.push_back(meet);
            meet = getGate(meet->predecessorId);
        }
        while (otherPath->id != n->id) {
            path2.push_back(otherPath);
            otherPath = getGate(otherPath->predecessorId);
        }

        Gset.insert(Gset.end(), path1.begin(), path1.end());
        Gset.insert(Gset.end(), path2.begin(), path2.end());
        removeTheSame(Gset);

        // if there are PI in GSet, then return false
        for (size_t i = 0; i < Gset.size(); ++i)
            if (Gset[i]->type == PI_GATE) {
                assert(0);
                cerr << "PI in Gset" << endl;
                return false;
            }
    }
    // [ W2 ]
    /*
    Mark all nodes in G as
    visited. Iterate over all fanins of the nodes in G and add
    all nodes without marks to I . If |I| > k , terminate without
    returning a result.
    */
    visitMark++;

    for (size_t i = 0; i < Gset.size(); ++i) {
        Gset[i]->visited = visitMark;
    }
    for (size_t i = 0; i < Gset.size(); ++i) {
        if (getGate(Gset[i]->fanin0id)->visited != visitMark) {
            getGate(Gset[i]->fanin0id)->visited = visitMark;
            Iset.push_back(getGate(Gset[i]->fanin0id));
        }
        if (getGate(Gset[i]->fanin1id)->visited != visitMark) {
            getGate(Gset[i]->fanin1id)->visited = visitMark;
            Iset.push_back(getGate(Gset[i]->fanin1id));
        }
    }

    if (Iset.size() > cutSize) return false;

    // [ W3 ]
    /*
    As long as |I| ≤ k and not all
    nodes in I are primary inputs, repeat two steps: (a) first perform
    all cost-free expansions of I ; (b) then choose a fanin v from
    I such that |E XPAND (I, v)| ≤ k and v has a highest fanout
    count within G . Expand I with n and go to (a). Otherwise, if
    no further expansion is possible in (b), go to the next step.
    */
    {
        vector<CirGate*> costFreeSet;
        while (findAllCostFreeExpansion(Iset, costFreeSet) && (Iset.size() <= cutSize)) {
            // (a) all cost-free expansions of I
            for (auto& g : costFreeSet) {
                // expand I with v
                if (getGate(g->fanin0id)->visited != visitMark) {
                    Iset.push_back(getGate(g->fanin0id));
                    getGate(g->fanin0id)->visited = visitMark;
                }
                if (getGate(g->fanin1id)->visited != visitMark) {
                    Iset.push_back(getGate(g->fanin1id));
                    getGate(g->fanin1id)->visited = visitMark;
                }

                // remove v from I
                removeElement(Iset, g);
                Gset.push_back(g);
            }
            costFreeSet.clear();

            // (b) find v (max fanout count)
            int maxFanout = 0, currFanout = 0, IsetSpace = cutSize - Iset.size(), cost = 0;
            CirGate* v = nullptr;
            for (auto& g : Iset) {
                if (g->type == PI_GATE) continue;
                if (IsetSpace == 0) break;
                currFanout = 0, cost = 0;
                CirGate* fanin0 = getGate(g->fanin0id);
                CirGate* fanin1 = getGate(g->fanin1id);
                if (fanin0->visited != visitMark) ++cost;
                if (fanin1->visited != visitMark) ++cost;
                if (cost > IsetSpace) continue;
                for (size_t j = 0; j < g->fanoutid.size(); ++j) {
                    if (getGate(g->fanoutid[j].first)->visited == visitMark) currFanout++;
                }
                if (currFanout > maxFanout) {
                    maxFanout = currFanout;
                    v = g;
                }
            }

            // expand I with v
            if (v == nullptr) continue;
            if (getGate(v->fanin0id)->visited != visitMark) {
                Iset.push_back(getGate(v->fanin0id));
                getGate(v->fanin0id)->visited = visitMark;
            }
            if (getGate(v->fanin1id)->visited != visitMark) {
                Iset.push_back(getGate(v->fanin1id));
                getGate(v->fanin1id)->visited = visitMark;
            }

            // remove v from I
            removeElement(Iset, v);
            Gset.push_back(v);
        }
    }

    // [ W4 ]
    /*
    Define a (sorted) map L which
    assigns a (initially empty) set of nodes to each level of N .
    Iterate over all nodes in I ∪ G , mark them, and sort them
    into L at the corresponding levels. Iterate over the nodes at
    each level in L from lowest to highest level. For each node,
    systematically explore its fanouts. If a fanout v is an inner
    node and not marked, but all its fanins are marked, then add
    v to G, mark it, and sort it into L
    */
    {
        CirGate* g;

        visitMark++;

        for (size_t i = 0; i < Gset.size(); ++i) {
            Gset[i]->visited = visitMark;
        }

        for (size_t i = 0; i < Iset.size(); ++i) {
            Iset[i]->visited = visitMark;
        }

        unordered_map<int, int> isDoneMap;
        for (size_t i = 0; i < Gset.size(); ++i) {
            isDoneMap[Gset[i]->id] = -1;
        }
        for (size_t i = 0; i < Iset.size(); ++i) {
            isDoneMap[Iset[i]->id] = -1;
        }
        vector<CirGate*> currentLevel;
        vector<CirGate*> nextLevel;
        // the 0-level = I
        currentLevel.insert(currentLevel.end(), Iset.begin(), Iset.end());

        while (!currentLevel.empty()) {
            for (size_t i = 0; i < currentLevel.size(); ++i) {
                for (size_t j = 0; j < currentLevel[i]->fanoutid.size(); ++j) {
                    g = getGate(currentLevel[i]->fanoutid[j].first);
                    if (g->type == PO_GATE) continue;
                    if (g->visited == visitMark && isDoneMap[g->id] == -1) {
                        nextLevel.push_back(g);
                    }
                    if (g->visited != visitMark && (getGate(g->fanin0id)->visited == visitMark) && (getGate(g->fanin1id)->visited == visitMark)) {
                        nextLevel.push_back(g);
                        Gset.push_back(g);
                        g->visited = visitMark;
                        isDoneMap[g->id] = -1;
                    }
                }
                isDoneMap[currentLevel[i]->id] = true;
            }
            removeTheSame(nextLevel);
            currentLevel = nextLevel;
            nextLevel.clear();
        }

        for (auto g : Gset) {
            if (getGate(g->fanin0id)->visited != visitMark)
                assert(0);
            if (getGate(g->fanin1id)->visited != visitMark)
                assert(0);
        }
    }

    // [ W5 ]
    /*
    Define a zero-initialized reference counter for each node in N .
    Iterate over all nodes in G and increment the counters of all fanins of
    g ∈ G . Iterate again over all nodes in G , observe their reference
    counters, and mark a node as output if its reference counter is
    lower than its fanout count
    */

    {
        unordered_map<size_t, size_t> refCount;
        for (size_t i = 0; i < Gset.size(); ++i) {
            refCount[Gset[i]->id] = 0;
        }
        for (size_t i = 0; i < Gset.size(); ++i) {
            refCount[Gset[i]->fanin0id]++;
            refCount[Gset[i]->fanin1id]++;
        }
        for (size_t i = 0; i < Gset.size(); ++i) {
            if (refCount[Gset[i]->id] < Gset[i]->fanoutid.size()) {  // whose  fanout not all in G
                Oset.push_back(Gset[i]);
                Gset.erase(Gset.begin() + i);
                --i;
            }
        }
    }
    // [ W6 ]
    /*
    Sort the nodes in G in topological
    order with respect to the structure of N and return the
    triple (I, O, G).
    */
    {
        CirGate* g;
        // sort Gset
        updateTrace();
        vector<CirGate*> Gset_temp;
        for (size_t i = 0; i < trace.size(); i++) {
            g = getGate(trace[i]);
            if (g->type != AIG_GATE) continue;
            for (size_t j = 0; j < Gset.size(); ++j) {
                if (g->id == Gset[j]->id) {
                    Gset_temp.push_back(g);
                    Gset.erase(Gset.begin() + j);
                    break;
                }
            }
            if (Gset.empty()) break;
        }
        Gset = Gset_temp;
        for (auto g : Gset) {
            for (size_t i = 0; i < g->fanoutid.size(); ++i) {
                if (getGate(g->fanoutid[i].first)->visited != visitMark) {
                    assert(0);
                }
            }
        }
    }
    return true;
}

/*
[ Algorithm M (Multi-output Resubstitution) ]

Given a multi-
output window (I, O, G) with input nodes I , output nodes O ,
and inner nodes G , this algorithm resynthesizes the output
functions f_oi , 1 ≤ i ≤ m to obtain a replacement (I, O ′ , G′ )
with functionally equivalent output functions.

*/

bool CirMgr::windowResubstitution(vector<CirGate*>& Iset, vector<CirGate*>& Oset, vector<CirGate*>& Gset) {
    updateCircuit();

    // [ M1 ]
    /*
    Initialize G ′ with G . Let T = ∅ be the set of
    already-tried nodes. Associate each input nodes i j ∈ I with the
    j th projection function. Simulate the window in a topological
    order to obtain the functions of all inner nodes and output
    nodes in terms of the input nodes.
    */
    vector<int> T;

    // Given a  changeLevel, return a Input pattern(010101, 00110011,, ...)
    auto genInputPattern = [](int changeLevel) {
        assert(changeLevel > 0);
        // gen input pattern
        uint64_t pattern = 0;
        int counter = 0;
        bool isOne = false;
        for (size_t i = 0; i < 64; ++i) {
            pattern += isOne ? 1 : 0;
            if (i == 63) break;
            pattern <<= 1;
            counter++;
            if (counter == changeLevel) {
                isOne = !isOne;
                counter = 0;
            }
        }
        return pattern;
    };

    // gen input pattern
    for (size_t i = 0; i < Iset.size(); i++) {
        Iset[i]->pattern = genInputPattern(pow(2, i));
    }

    // gen sim order (care about Gset and Oset order)
    updateTrace();
    vector<CirGate*> simOrder, simOrderTemp;
    simOrderTemp.insert(simOrderTemp.end(), Gset.begin(), Gset.end());
    simOrderTemp.insert(simOrderTemp.end(), Oset.begin(), Oset.end());
    for (size_t i = 0; i < trace.size(); i++) {
        CirGate* g = getGate(trace[i]);
        if (g->type != AIG_GATE) continue;
        for (size_t j = 0; j < simOrderTemp.size(); ++j) {
            if (g->id == simOrderTemp[j]->id) {
                simOrder.push_back(g);
                simOrderTemp.erase(simOrderTemp.begin() + j);
                break;
            }
        }
        if (simOrderTemp.empty()) break;
    }

    // simulate
    for (auto& g : simOrder) {
        g->pattern = (g->fanin0cp ? ~(getGate(g->fanin0id)->pattern) : (getGate(g->fanin0id)->pattern)) & (g->fanin1cp ? ~(getGate(g->fanin1id)->pattern) : (getGate(g->fanin1id)->pattern));
    }

    // [ M2 , M3 ]
    /*
    Select a node r ∈ G, r /∈ T in a reversed topological order,
    Temporarily complement the function
    of r , i.e., let f_r' = ¬f_r , and re-simulate TFO(r) to obtain f'o_i
    for each output node o i . Compute the ODC
    */
    vector<uint64_t> oriOutputPattern;
    for (size_t i = 0; i < Oset.size(); i++) {
        oriOutputPattern.push_back(Oset[i]->pattern);
    }
    CirGate* r;
    for (size_t i = Gset.size() - 1; i != SIZE_MAX; --i) {
        r = Gset[i];
        uint64_t oriNodePattern = r->pattern;
        r->pattern = ~r->pattern;

        // re-simulate TFO(r)
        size_t rIndex = simOrder.size();
        for (size_t j = 0; j < simOrder.size(); j++) {
            if (simOrder[j]->id == r->id) {
                rIndex = j;
                break;
            }
        }
        assert(rIndex != simOrder.size());
        for (size_t j = rIndex + 1; j < simOrder.size(); j++) {
            simOrder[j]->pattern = (simOrder[j]->fanin0cp ? ~(getGate(simOrder[j]->fanin0id)->pattern) : (getGate(simOrder[j]->fanin0id)->pattern)) & (simOrder[j]->fanin1cp ? ~(getGate(simOrder[j]->fanin1id)->pattern) : (getGate(simOrder[j]->fanin1id)->pattern));
        }

        // find ODC
        set<size_t> ODC;
        for (size_t j = 0; j < 64; j++) ODC.insert(j);
        for (size_t j = 0; j < Oset.size(); ++j) {
            for (size_t k = 0; k < 64; k++) {
                if (((Oset[j]->pattern >> k) & 1) != ((oriOutputPattern[j] >> k) & 1)) {
                    ODC.erase(k);
                }
            }
        }

        // [ M4 ]
        /*
        Mark the nodes in the maximum fanout-
        free cone (MFFC) [ 6] of r . A node v is in the MFFC of r
        if v ∈ TFI(r) and all paths from v to any output node pass
        through r.
        */

        // find MFFC
        vector<CirGate*> MFFC;
        updateFanout();
        // if no MFFC, then go to next node in Gset
        if (!findMFFC(r, MFFC)) {
            continue;
        }
        // [ M5 ]
        /*
        Collect the divisors D = I ∪ G ′ −
        MFFC(r) − TFO(r).
        */
        vector<CirGate*> D;
        vector<CirGate*> TFO;
        D.insert(D.end(), Iset.begin(), Iset.end());
        D.insert(D.end(), Gset.begin(), Gset.end());
        ++visitMark;
        getFanoutCone(r, TFO);

        unordered_map<int, bool> TFOMap;

        for (size_t j = 0; j < TFO.size(); ++j) {
            TFOMap[TFO[j]->id] = true;
        }
        for (size_t j = 0; j < D.size(); ++j) {
            if (TFOMap[D[j]->id]) {
                D.erase(D.begin() + j);
                --j;
            }
        }

        for (size_t j = 0; j < MFFC.size(); ++j) {
            for (size_t k = 0; k < D.size(); ++k) {
                if (MFFC[j]->id == D[k]->id) {
                    D.erase(D.begin() + k);
                    --k;
                }
            }
        }

        // [ M6 ]
        /*
        Resynthesize f r using the collected
        divisors D by calling Algorithm S
        */
        set<size_t> fOnset, fOffset;
        // oriNodePattern = f_r
        // r->pattern = ~ f_r
        for (size_t j = 0; j < 64; j++) {
            if ((((oriNodePattern >> j) & 1) == 1) && (ODC.find(j) == ODC.end())) {
                fOnset.insert(j);
            }
        }
        for (size_t j = 0; j < 64; j++) {
            if ((((r->pattern >> j) & 1) == 1) && (ODC.find(j) == ODC.end())) {
                fOffset.insert(j);
            }
        }

        vector<Divisor*> collectList;
        vector<DivisorPair*> collectPairList;
        if (!resyn(fOnset, fOffset, D, collectList, collectPairList)) continue;

        size_t Gs = (collectPairList.size() == 0) ? ::max((int)collectList.size() - 1, 0) : (1 + collectPairList.size());
        if (Gs > MFFC.size()) cerr << "skip: Gs > MFFC" << endl;
        if (Gs > MFFC.size()) continue;

        // recover all for debug
        r->pattern = oriNodePattern;
        for (size_t j = rIndex + 1; j < simOrder.size(); j++) {
            simOrder[j]->pattern = (simOrder[j]->fanin0cp ? ~(getGate(simOrder[j]->fanin0id)->pattern) : (getGate(simOrder[j]->fanin0id)->pattern)) & (simOrder[j]->fanin1cp ? ~(getGate(simOrder[j]->fanin1id)->pattern) : (getGate(simOrder[j]->fanin1id)->pattern));
        }

        // update the graph if Gs < MFFC
        // number of the new nodes

        // create new node Gs -> use the existed MFFC node
        size_t unusedMFFCIndex = 0;
        divisorType type = (collectList.size() > 0) ? collectList[0]->type : collectPairList[0]->type;
        auto useMFFCNode = [&unusedMFFCIndex, &MFFC](CirGate* c1, bool c1IsNeg, CirGate* c2, bool c2IsNeg) {
            CirGate* newGate = MFFC[unusedMFFCIndex];
            newGate->fanin0id = c1->id;
            newGate->fanin1id = c2->id;
            newGate->fanin0cp = c1IsNeg;
            newGate->fanin1cp = c2IsNeg;
            newGate->pattern = (newGate->fanin0cp ? (~c1->pattern) : c1->pattern) & (newGate->fanin1cp ? (~c2->pattern) : c2->pattern);
            unusedMFFCIndex++;
        };

        auto replaceRnode = [=](CirGate* c1, bool c1IsNeg, CirGate* c2, bool c2IsNeg) {
            CirGate* newGate = r;
            newGate->fanin0id = c1->id;
            newGate->fanin1id = c2->id;
            newGate->fanin0cp = c1IsNeg;
            newGate->fanin1cp = c2IsNeg;
            newGate->pattern = (newGate->fanin0cp ? (~c1->pattern) : c1->pattern) & (newGate->fanin1cp ? (~c2->pattern) : c2->pattern);
        };

        // refresh the connection for the fanout of R with new R
        auto refreshFanout = [=, this](CirGate* R, CirGate* newR, bool addInv) {
            for (auto fanoutpair : R->fanoutid) {
                CirGate* fanout = getGate(fanoutpair.first);
                uint64_t fanin0Pattern = (fanout->fanin0cp ? (~getGate(fanout->fanin0id)->pattern) : (getGate(fanout->fanin0id)->pattern));
                uint64_t fanin1Pattern = (fanout->fanin1cp ? (~getGate(fanout->fanin1id)->pattern) : (getGate(fanout->fanin1id)->pattern));
                if (fanout->fanin0id == R->id) {
                    fanout->fanin0id = newR->id;
                    fanout->fanin0cp = addInv ? (!fanout->fanin0cp) : fanout->fanin0cp;
                } else if (fanout->fanin1id == R->id) {
                    fanout->fanin1id = newR->id;
                    fanout->fanin1cp = addInv ? (!fanout->fanin1cp) : fanout->fanin1cp;
                }
                uint64_t fanin0PatternA = (fanout->fanin0cp ? (~getGate(fanout->fanin0id)->pattern) : (getGate(fanout->fanin0id)->pattern));
                uint64_t fanin1PatternA = (fanout->fanin1cp ? (~getGate(fanout->fanin1id)->pattern) : (getGate(fanout->fanin1id)->pattern));
                assert((fanin0Pattern && fanin1Pattern) == (fanin0PatternA && fanin1PatternA));
            }
        };

        switch (type) {
            // replace r with const 0
            case divisorType::CONST0:
                refreshFanout(r, ConstGates[0], false);
                break;
            case divisorType::CONST1:
                refreshFanout(r, ConstGates[0], true);
                break;
            case divisorType::POS_UNATE:
            case divisorType::NEG_UNATE: {
                // (1) connect the 2 binate divisor to pos unate by AND + new R output INV
                for (size_t i = 0; i < collectPairList.size(); ++i) {
                    useMFFCNode(collectPairList[i]->divisor1->gate, collectPairList[i]->divisor1->isNeg,
                                collectPairList[i]->divisor2->gate, collectPairList[i]->divisor2->isNeg);
                }

                // (2) connect the 2 divisor by OR
                if (unusedMFFCIndex == 2)  //  3-resub
                    replaceRnode(MFFC[0], true, MFFC[1], true);
                else if (unusedMFFCIndex == 1)  // 2-resub
                    replaceRnode(MFFC[0], true, collectList[0]->gate, !collectList[0]->isNeg);
                else if (unusedMFFCIndex == 0) {
                    // 1 - resub (Note that 0-resub is not things to do)
                    if (collectList.size() == 2) {
                        replaceRnode(collectList[0]->gate, !collectList[0]->isNeg,
                                     collectList[1]->gate, !collectList[1]->isNeg);
                    }
                }

                // (3) refresh the fanout of r
                if (collectList.size() == 1 && collectPairList.empty())
                    (type == divisorType::POS_UNATE)
                        ? refreshFanout(r, collectList[0]->gate, collectList[0]->isNeg)
                        : refreshFanout(r, collectList[0]->gate, !collectList[0]->isNeg);
                else
                    (type == divisorType::POS_UNATE) ? refreshFanout(r, r, true)
                                                     : refreshFanout(r, r, false);
            } break;

            default:
                break;
        }

        // update Circuit
        updateCircuit();
        sweep();
        optimize();
        // if one of Gset node resyn sucess, then we return and find new window of next node
        return true;
    }
    return false;
}

// Given a node, return the maximun fanout free cone of the node
bool CirMgr::findMFFC(CirGate* r, vector<CirGate*>& MFFC) {
    // the MFFC of r is in fanin cone of r
    vector<int> candidateMFFC;
    CirGate* node;
    ++visitMark;
    DFS(r, candidateMFFC);

    // keep a map to store the info of the MFFC candidate
    unordered_map<int, bool> candidateMap;
    for (size_t i = 0; i < candidateMFFC.size(); ++i) {
        candidateMap[candidateMFFC[i]] = true;
    }
    candidateMFFC.pop_back();  // last one is r

    for (size_t i = candidateMFFC.size() - 1; i != SIZE_MAX; --i) {
        node = getGate(candidateMFFC[i]);
        if (node->type == PO_GATE) assert(0);
        if (node->type == PI_GATE) {
            candidateMFFC.erase(candidateMFFC.begin() + i);
            candidateMap[node->id] = false;
            continue;
        }
        if (node->type != AIG_GATE) continue;
        for (size_t j = 0; j < node->fanoutid.size(); j++) {
            if (candidateMap.find(node->fanoutid[j].first) == candidateMap.end()) {
                candidateMFFC.erase(candidateMFFC.begin() + i);
                candidateMap[node->id] = false;
                break;
            } else if (candidateMap[node->fanoutid[j].first] == false) {
                candidateMFFC.erase(candidateMFFC.begin() + i);
                candidateMap[node->id] = false;
                break;
            }
        }
    }
    // fill the answer in real MFFC
    for (size_t i = 0; i < candidateMFFC.size(); ++i)
        MFFC.push_back(getGate(candidateMFFC[i]));

    return MFFC.size() > 0;
}

void CirMgr::getFanoutCone(CirGate* r, vector<CirGate*>& TFO) {
    if (r->type == PO_GATE) return;
    if (r->visited == visitMark) return;
    r->visited = visitMark;
    TFO.push_back(r);
    for (size_t i = 0; i < r->fanoutid.size(); i++) {
        getFanoutCone(getGate(r->fanoutid[i].first), TFO);
    }
}
