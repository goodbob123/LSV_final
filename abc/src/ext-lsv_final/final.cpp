#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "bdd/cudd/cudd.h"
#include "sat/cnf/cnf.h"

using namespace std;
using namespace std;

extern "C" {
Aig_Man_t* Abc_NtkToDar(Abc_Ntk_t* pNtk, int fExors, int fRegisters);
}
static int Lsv_CommandPrintMessage(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandBddReorder(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
    Cmd_CommandAdd(pAbc, "LSV", "lsv_print_message", Lsv_CommandPrintMessage, 0);
    Cmd_CommandAdd(pAbc, "LSV", "lsv_bdd_reorder", Lsv_CommandBddReorder, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};

struct PackageRegistrationManager {
    PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;


// just for test info before print_state
int Lsv_CommandPrintMessage(Abc_Frame_t* pAbc, int argc, char** argv) {
    if (argc ==2)
        cout<<argv[1]<<endl;
    return 0;
}

// # TODO: some optimize command can be added here (call it after &read , before &write)
// int Lsv_CommandBobOptimizeSomething(Abc_Frame_t* pAbc, int argc, char** argv) {


int Lsv_CommandBddReorder(Abc_Frame_t* pAbc, int argc, char** argv) {

    Abc_Ntk_t* pNtk;
    pNtk = Abc_FrameReadNtk(pAbc);

    // ensure Ntk exist
    if (pNtk == NULL) {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }
    if (!Abc_NtkIsBddLogic(pNtk)) {
        Abc_Print(-1, "Simulating BDDs can only be done for BDD networks (run \"collapse\").\n");
        return 1;
    }

    DdManager * dd;
    dd = (DdManager *)pNtk->pManFunc;
    cout<<"Start reordering"<<endl;
    cout<<"Origin DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
    int option;
    if (argc == 2)
        option = atoi(argv[1]);
    else
        option = 1;
    
    switch (option)
    {
    case 1:
        //Option 1: Dynamic reordering by sifting
        Cudd_AutodynEnable(dd, CUDD_REORDER_SYMM_SIFT);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_SYMM_SIFT, 1);
        cout<<"(Option 1) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    case 2:
        //Option 2: Dynamic reordering by window permutation
        Cudd_AutodynEnable(dd, CUDD_REORDER_WINDOW2);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_WINDOW2, 1);
        cout<<"(Option 2) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    case 3:
        // //Option 3: Dynamic reordering by simulated annealing
        Cudd_AutodynEnable(dd, CUDD_REORDER_ANNEALING);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_ANNEALING, 100);
        cout<<"(Option 3) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    case 4:
         // //Option 4: Dynamic reordering by genetic algorithm
        Cudd_AutodynEnable(dd, CUDD_REORDER_GENETIC);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_GENETIC, 100);
        cout<<"(Option 4) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    case 5:
        // //Option 5: Dynamic reordering by swapping
        Cudd_AutodynEnable(dd, CUDD_REORDER_RANDOM);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_RANDOM, 100);
        cout<<"(Option 5) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    default:
        Cudd_AutodynEnable(dd, CUDD_REORDER_SYMM_SIFT);
        for(int i=0; i<100;++i)
        Cudd_ReduceHeap(dd, CUDD_REORDER_SYMM_SIFT, 1);
        cout<<"(Option 1) DdManager nodes: "<<Cudd_ReadNodeCount(dd)<<"  "<<endl;
        break;
    }
   
    Cudd_AutodynDisable (dd);


    return 0;


}