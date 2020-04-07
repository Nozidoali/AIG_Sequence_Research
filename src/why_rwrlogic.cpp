#include "why_rwrlogic.h"
#include <cmath>
#include <random>
#include <ctime>

/**
 * Default Constructor
 * 
 * 1. Create a Rwr_Man_t
 * 2. Create a Cut_Man_t
 * 
 * @_pNtk: abc network
 * @_T0: initial temperature (maximum loss tolerance)
 * @_runtime: number of iterations
 */
SA :: SA( Abc_Ntk_t * _pNtk, int _T0, int _runtime ) {

    // assignments
    pNtk = _pNtk; 
    T0 = temperature = _T0;
    iteration = ( runtime = _runtime ) , 0;

    assert( Abc_NtkIsStrash(pNtk) );
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);

    // start the rewriting manager
    pManRwr = Rwr_ManStart( 0 );
    assert ( pManRwr != NULL );
    pManRwr->nNodesBeg = Abc_NtkNodeNum(pNtk);

    // compute the reverse levels if level update is requested
    Abc_NtkStartReverseLevels( pNtk, 0 );

    // start the cut manager
    pManCut = WHY_NtkStartCutManForRewrite( pNtk );
    pNtk->pManCut = pManCut;

    // random seed
    srand( time( NULL ) );

}

/**
 * Destructor
 * 
 * 1. Delete Rwr_Man_t
 * 2. Delete Cut_Man_t
 */
SA :: ~SA() {

    pManRwr->nNodesEnd = Abc_NtkNodeNum(pNtk);

    // release the manager
    Rwr_ManStop( pManRwr );
    Cut_ManStop( pManCut );
    pNtk->pManCut = NULL;

    // fix the levels
    Abc_NtkStopReverseLevels( pNtk );

    // check
    assert( Abc_NtkCheck( pNtk ) );
}

/**
 * WHY_NodeRewrite
 * 
 * @pNode: target node to perform rewrite
 * @return: the gain of rewriting
 */
int SA :: NodeRewrite( Abc_Obj_t * pNode ) {

    // skip invalid nodes
    if (pNode == nullptr)
        return 0;

    // skip persistant nodes
    if ( Abc_NodeIsPersistant(pNode) )
        return 0;

    // skip the nodes with many fanouts
    if ( Abc_ObjFanoutNum(pNode) > 1000 )
        return 0;

    return WHY_NodeRewrite( pManRwr, pManCut, pNode, 1, 0, 0 );

}


/**
 * WHY_NodeUpdate
 * 
 * update and replace pNode by the new network in rwr manager
 */
extern "C" Abc_Obj_t * Dec_GraphToNetwork( Abc_Ntk_t * pNtk, Dec_Graph_t * pGraph );

void SA :: NodeUpdate( Abc_Obj_t * pNode ) {

    Dec_Graph_t * pGraph = (Dec_Graph_t *)Rwr_ManReadDecs(pManRwr);
    int fCompl = Rwr_ManReadCompl( pManRwr );
    if ( fCompl ) Dec_GraphComplement( pGraph );
    Abc_Obj_t * pNodeNew = Dec_GraphToNetwork( pNtk, pGraph );
    Abc_AigReplace( (Abc_Aig_t *)pNtk->pManFunc, pNode, pNodeNew, 1 );
    if ( fCompl ) Dec_GraphComplement( pGraph );

}

/**
 * WHY_isAccepted
 * 
 * Decide if the result is acceptable
 */
bool SA :: isAccepted ( int score ) {

    double rand_num = rand() / ( RAND_MAX + 1.0 ); 
    double threshold = score > 0 ? 1.0 : exp( - (double)score / temperature );
    return rand_num <= threshold;

}

/**
 * WHY_Anneal
 * 
 * Decrease the temperature and increase iteration
 */
void SA :: Anneal( double step ) {
    
    temperature = T0 / ( 1 + (++iteration) );

}

/**
 * WHY_Rewrite
 * 
 * 
 */
void SA :: Rewrite( RWR_METHOD method ) {

    vector<int> count;
    int number = 10;
    while( number -- ) count.push_back( 0 );

    Abc_Obj_t * pObj;
    int i;
    Abc_NtkForEachNode( pNtk, pObj, i ) {
        int gain = NodeRewrite( pObj );
        // if ( gain > 0 ) NodeUpdate( pObj );

        gain = gain < 0 ? 0 : gain;
        if ( gain < 10 ) count[gain]++;
    }

    cout << endl;
    for( auto & bin : count ) cout << (int)bin <<  " | ";
    cout << endl;
}