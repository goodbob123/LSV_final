#include <cassert>
#include <iomanip>
#include <iostream>

#include "aigCmd.h"
#include "util.h"

using namespace std;

extern "C" {
#include "aiger.h"
}
extern int effLimit;

bool initAigCmd() {
    if (!(cmdMgr->regCmd("AIG2aag", 3, new Aig2aagCmd))) {
        cerr << "Registering \"cir\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
//    CIRCec <(string fileName)> <(string fileName)>
//----------------------------------------------------------------------
CmdExecStatus
Aig2aagCmd::exec(const string& option) {
    // check option
    vector<string> options;
    if (!CmdExec::lexOptions(option, options))
        return CMD_EXEC_ERROR;
    if (options.empty())
        return CmdExec::errorOption(CMD_OPT_MISSING, "");

    string fileName1, fileName2;
    fileName1 = options[0];
    fileName2 = options[1];

    // check if the file exists
    ifstream ifs1(fileName1.c_str());
    if (!ifs1) {
        cerr << "Error: \"" << fileName1 << "\" does not exist!!" << endl;
        return CMD_EXEC_ERROR;
    }

    aigtoaig(fileName1.c_str(), fileName2.c_str());
    return CMD_EXEC_DONE;
}

void Aig2aagCmd::usage(ostream& os) const {
    os << "Usage: AIG2aagCmd <(aig fileName)> <(aag fileName)>" << endl;
}

void Aig2aagCmd::help() const {
    cout << setw(15) << left << "AIG2aagCmd: "
         << "read in a circuit and construct the netlist" << endl;
}
