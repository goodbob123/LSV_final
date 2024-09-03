#include "cirDef.h"
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

// simple way: simulated based
void CirMgr::doCec(CirMgr* compareMgr) {
    random_device rd;
    mt19937_64 generator(rd());
    uniform_int_distribution<uint64_t> distribution(0, numeric_limits<uint64_t>::max());

    if (inputId2Pattern.empty()) {
        inputId2Pattern = vector<int>(all.size());
        for (size_t i = 0; i < PIs.size(); ++i) {
            inputId2Pattern[PIs[i]->id] = i;
        }
    }
    compareMgr->inputId2Pattern = inputId2Pattern;

    inputPattern.clear();
    compareMgr->inputPattern.clear();
    for (size_t i = 0; i < PIs.size(); ++i) {
        uint64_t random = distribution(generator);
        inputPattern.emplace_back(random);
    }
    cerr << "start simulate CEC" << endl;
    compareMgr->inputPattern = inputPattern;
    simulate64times();
    compareMgr->simulate64times();
    cerr << "start check CEC" << endl;
    size_t diff = 0;
    // compare POs
    for (size_t i = 0; i < POs.size(); ++i) {
        if ((POs[i]->pattern ^ compareMgr->POs[i]->pattern) != 0) {
            cerr << "PO [" << setw(4) << i << " ] mismatch" << endl;
            cerr << "  in cir1: " << setw(70) << bitset<64>(POs[i]->pattern) << endl;
            cerr << "  in cir2: " << setw(70) << bitset<64>(compareMgr->POs[i]->pattern) << endl;
            diff++;
        } else
            cerr << "PO [" << setw(4) << i << " ] match" << endl;
    }
    if (diff == 0) {
        cerr << "-------------------[ Circuit Valid ]---------------------" << endl;
        return;
    } else {
        cerr << "Total PO: " << POs.size() << " , Total diff: " << diff << endl;
    }
}