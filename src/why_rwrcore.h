#ifndef WHY_REWRITE_CORE_H
#define WHY_REWRITE_CORE_H

#include "headers.h"
#include "cmdline.h"
#include "abcApi.h"
#include "debugAssert.h"
#include "cktUtil.h"

using namespace std;
using namespace cmdline;

struct Solution {
    int gain;
    int * leaves;   // 4 feasible cut
    Solution( int _gain, int * _leaves ) { gain = _gain; leaves = _leaves; }
};

extern "C" {
    void Dec_GraphUpdateNetwork( Abc_Obj_t * pRoot, Dec_Graph_t * pGraph, int fUpdateLevel, int nGain );
}

Cut_Man_t * WHY_NtkStartCutManForRewrite( Abc_Ntk_t * pNtk );

/**
 * WHY_NodeRewrite
 * 
 * Adjust the gain best macha
 */
int WHY_NodeRewrite( Rwr_Man_t * p, Cut_Man_t * pManCut, Abc_Obj_t * pNode, int fUpdateLevel, int fUseZeros, int fPlaceEnable );

#endif