#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

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
static int Lsv_CommandSimulatedAnnealing(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
    Cmd_CommandAdd(pAbc, "LSV", "lsv_print_message", Lsv_CommandPrintMessage, 0);
    Cmd_CommandAdd(pAbc, "LSV", "lsv_sa", Lsv_CommandSimulatedAnnealing, 0);
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
int Lsv_CommandSimulatedAnnealing(Abc_Frame_t* pAbc, int argc, char** argv) {

    Abc_Ntk_t* pNtk;
    pNtk = Abc_FrameReadNtk(pAbc);
    srand(time(0));

    // ensure Ntk exist
    if (pNtk == NULL) {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }
    if (!Abc_NtkIsStrash(pNtk)) {
        Abc_Print(-1, "Simulating BDDs can only be done for BDD networks (run \"collapse\").\n");
        return 1;
    }

    //init annealing condition
    cout<<"Simulated Annealing"<<endl;
    double start_heat = 1000, end_heat = 0.01, cooling_rate = 0.99;

    //preserve old aig
    cout<<"Origin and: "<<Abc_NtkNodeNum(pNtk)<<"  "<<endl;
    double best_eval = Abc_NtkNodeNum(pNtk);
    char * Command = "write_aiger annealing.aig;";
    if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
        Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
        return 1;
    }

    for(double heat = start_heat; heat > end_heat; heat *= cooling_rate) {
        int option = (rand()%7)+1;
        switch (option){
            case 1:
                Command = "rewrite; refactor; resub;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 2:
                Command = "&get; &deepsyn -I 10 -J 50 -T 30 -S 11 -t; &put;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 3:
                Command = "&get; &deepsyn -I 5 -J 50 -T 15 -S 22 -t; &put;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 4:
                Command = "&get; &deepsyn -I 7 -J 50 -T 20 -S 33 -t; &put;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 5:
                Command = "&get; &transduction -T 4 -I 111 -R 117; &dc2; &put;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 6:
                Command = "&get; &transduction -I 123 -R 17; &dc2; &put;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            case 7:
                Command = "orchestrate;";
                if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                    Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                    return 1;
                }
                break;
            default:
                cout<<"SA_option_error"<<endl;
                return 1;
                break;
        }

        double change = Abc_NtkNodeNum(pNtk) - best_eval;
        if (change < 0 || (exp((-change-0.1) / heat) > (rand() / (double) RAND_MAX))) {
            Command = "write_aiger annealing.aig;";
            if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                return 1;
            }
            best_eval = Abc_NtkNodeNum(pNtk);
        }
        else{
            Command = "read annealing.aig";
            if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
                Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
                return 1;
            }
        }
    }

    Command = "read_aiger annealing.aig";
    if ( Cmd_CommandExecute(Abc_FrameGetGlobalFrame(), Command) ){
        Abc_Print( 1, "Something did not work out with the command \"%s\".\n", Command );
        return 1;
    }

    remove("annealing.aig");
    return 0;

}

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