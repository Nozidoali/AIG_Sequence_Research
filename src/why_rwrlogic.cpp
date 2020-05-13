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
SA :: SA( Abc_Ntk_t * _pNtk, double _T0, int _runtime, double _ratio ) {

    // assignments
    pNtk = _pNtk; 
    T0 = temperature = _T0;
    iteration = ( runtime = _runtime ) , 0;
    ratio = _ratio;

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
Solution SA :: NodeRewrite( Abc_Obj_t * pNode, MODE mode ) {

    // skip invalid nodes
    if (pNode == nullptr)
        return Solution( -1, NULL );

    // skip persistant nodes
    if ( Abc_NodeIsPersistant(pNode) )
        return Solution( -1, NULL );

    // skip the nodes with many fanouts
    if ( Abc_ObjFanoutNum(pNode) > 1000 )
        return Solution( -1, NULL );

    return WHY_NodeRewrite( pManRwr, pManCut, pNode, 1, 0, 0, mode==TEMP? temperature:minGain );

}


/**
 * WHY_NodeUpdate
 * 
 * update and replace pNode by the new network in rwr manager
 */
extern "C" Abc_Obj_t * Dec_GraphToNetwork( Abc_Ntk_t * pNtk, Dec_Graph_t * pGraph );

Abc_Obj_t * SA :: NodeUpdate( Abc_Obj_t * pNode ) {

    Dec_Graph_t * pGraph = (Dec_Graph_t *)Rwr_ManReadDecs(pManRwr);
    int fCompl = Rwr_ManReadCompl( pManRwr );
    if ( fCompl ) Dec_GraphComplement( pGraph );
    Abc_Obj_t * pNodeNew = Dec_GraphToNetwork( pNtk, pGraph );
    Abc_AigReplace( (Abc_Aig_t *)pNtk->pManFunc, pNode, pNodeNew, 1 );
    if ( fCompl ) Dec_GraphComplement( pGraph );
    return pNodeNew;

}

/**
 * WHY_isAccepted
 * 
 * Decide if the result is acceptable
 */
bool SA :: isAccepted ( Solution solution ) {

    if ( solution.leaves == NULL )
        return false;
    int score = solution.gain;
    double rand_num = rand() / ( RAND_MAX + 1.0 ); 
    double threshold = score > 0 ? 1.0 : exp( (double)score / temperature );
    return rand_num < threshold;

}

/**
 * WHY_Anneal
 * 
 * Decrease the temperature and increase iteration
 */
void SA :: Anneal() {
    
    temperature *= ratio;

}

/**
 * WHY_Rewrite
 * 
 * 
 */
void SA :: Rewrite( RWR_METHOD method ) {
    int numOld = Abc_NtkNodeNum( pNtk );
    int totGain = 0;
    int attemps = 0;
    Abc_Obj_t * pObj;
    int i;
    switch ( method )
    {
    case SEQUENTIAL:
        temperature = -1;
        Abc_NtkForEachNode( pNtk, pObj, i ) {
            minGain = 1;
            Solution solution = NodeRewrite( pObj, MINGAIN );
            int gain = solution.gain;
            // if ( gain > 0 ) {
            //     cout << "Gain = " << gain << "\t id = "<< Abc_ObjId( pObj ) << endl;
            //     for (unsigned int j=0;j<4;j++) 
            //         cout << "\t Leaf #" << j << ":" << solution.leaves[j] << endl;
            // }
            if ( gain > 0 ) NodeUpdate( pObj );

        }
        break;

    case SIMUANNEAL:
        // cerr << "Iteration,Temperature,TotalGain" << endl;
        for( iteration = 0, attemps = 0; attemps < runtime && iteration < runtime; attemps ++) {
            Abc_Obj_t * pObj = WHY_RandNode( pNtk );
            Solution solution = NodeRewrite( pObj, TEMP );
            if ( solution.leaves != NULL ) {
                iteration ++;
                totGain += solution.gain;
                // cerr << iteration << ",";
                // cerr << (double)temperature << ",";
                // cerr << totGain << endl;
                Abc_Obj_t * pNew = NodeUpdate( pObj );
                Anneal();
                continue;
                // try immediately fix all the neighbours
                minGain = 1;
                if ( !pNew ) {
                    continue;
                }
                Vec_Ptr_t * vTarget = WHY_FindNeighbours( pNew );
                int ii;
                Abc_Obj_t * pObjj;
                Vec_PtrForEachEntry( Abc_Obj_t *, vTarget, pObjj, ii ) {
                    if ( Abc_ObjIsCi( pObjj ) || Abc_ObjIsCo( pObjj ) ) {
                        continue;
                    }
                    if ( pObjj && Abc_AigNodeIsAnd( pObjj ) ) {
                        Solution nb_solution = NodeRewrite( pObjj, MINGAIN );
                        if ( solution.leaves != NULL ) {
                            NodeUpdate( pObjj );
                        }
                    }
                }
                Vec_PtrFree( vTarget );
            }
        }
        // SEQ at last to fix the obvious nodes
        Rewrite( SEQUENTIAL );
        cerr << endl;
        break;   
    
    case ALTERSEQ:

        // cerr << Abc_NtkNodeNum( pNtk ) << endl;

        // SEQ+
        temperature = 0;
        Abc_AigForEachAnd( pNtk, pObj, i ) {
            history[ Abc_ObjId( pObj ) ] = 0;
            minGain = 1;
            Solution solution = NodeRewrite( pObj, MINGAIN );
            if ( solution.leaves != NULL ) {
                Abc_Obj_t * pObjNew = NodeUpdate( pObj );
                history[ Abc_ObjId( pObjNew ) ] = 1;
            }
        }

        // cerr << Abc_NtkNodeNum( pNtk ) << endl;

        // loop for <runtime> period
        for ( iteration=0;iteration<runtime;iteration++) {

            // SEQ-
            temperature = -1;
            int flex = 1;
            while ( flex -- ) {
                Abc_Obj_t * pTarget = WHY_RandNode( pNtk );
                minGain = -1;
                Solution solution = NodeRewrite( pTarget, MINGAIN );
                if ( solution.leaves != NULL ) {
                    NodeUpdate( pTarget );
                }
            }

        // cerr << Abc_NtkNodeNum( pNtk ) << endl;

            // SEQ+
            temperature = 0;
            Abc_AigForEachAnd( pNtk, pObj, i ) {
                history[ Abc_ObjId( pObj ) ] = 0;
                minGain = 1;
                Solution solution = NodeRewrite( pObj, MINGAIN );
                if ( solution.leaves != NULL ) {
                    Abc_Obj_t * pObjNew = NodeUpdate( pObj );
                    history[ Abc_ObjId( pObj ) ] = 1;
                }
            }

        // cerr << Abc_NtkNodeNum( pNtk ) << endl;


        }
                
        break;
    

    case QUICKSEQ:
        cerr << Abc_NtkNodeNum( pNtk ) << endl;

        // SEQ+
        minGain = 1;
        Abc_AigForEachAnd( pNtk, pObj, i ) {
            history[ Abc_ObjId( pObj ) ] = 0;
            minGain = 1;
            Solution solution = NodeRewrite( pObj, MINGAIN );
            if ( solution.leaves != NULL ) {
                Abc_Obj_t * pObjNew = NodeUpdate( pObj );
                history[ Abc_ObjId( pObjNew ) ] = 1;
            }
        }

        cerr << Abc_NtkNodeNum( pNtk ) << endl;

        // loop for <runtime> period
        for ( iteration=0;iteration<runtime;iteration++) {

            // initialize a vector to store the objs
            Vec_Ptr_t * vObj = Vec_PtrAlloc( 6 );

            // SEQ-
            minGain = -4;
            int flex = 1;
            while ( flex -- ) {
                Abc_Obj_t * pTarget = WHY_RandNode( pNtk );
                Solution solution = NodeRewrite( pTarget, MINGAIN );
                if ( solution.leaves != NULL ) {
                    Abc_Obj_t * pNew = NodeUpdate( pTarget );
                    if ( Abc_ObjIsCi( pNew ) || Abc_ObjIsCo( pNew ) ) {
                        continue;
                    }
                    if ( !Abc_ObjIsNode( pNew ) ) {
                        continue;
                    }
                    // add the fanin and the fanin of fanin to the working vector
                    Abc_ObjForEachFanin( pNew, pObj, i ) {
                        int ii;
                        Abc_Obj_t * pObjj;
                        if ( Abc_ObjIsCo( pObj ) || Abc_ObjIsCi( pObj ) ) {
                            continue;
                        }
                        if ( Abc_AigNodeIsAnd( pObj ) ) {
                            Vec_PtrPush( vObj, pObj );
                            Abc_ObjForEachFanin( pObj, pObjj, ii ) {
                                if ( Abc_ObjIsCo( pObjj ) || Abc_ObjIsCi( pObjj ) ) {
                                    continue;
                                }
                                if ( Abc_AigNodeIsAnd( pObjj ) ) {
                                    Vec_PtrPush( vObj, pObjj );
                                }
                            }
                        }                     
                    }
                    // add the fanout nodes to the working vector
                    Abc_ObjForEachFanout( pNew, pObj, i ) {
                        if ( Abc_ObjIsCo( pObj ) || Abc_ObjIsCi( pObj ) ) {
                            continue;
                        }
                        if ( Abc_AigNodeIsAnd( pObj ) ) {
                            Vec_PtrPush( vObj, pObj );
                        }
                    }
                    Vec_PtrPush( vObj, pNew );
                }
            }


        cerr << Abc_NtkNodeNum( pNtk ) << endl;

            // SEQ+
            minGain = 0;
            Vec_PtrForEachEntry( Abc_Obj_t *, vObj, pObj, i ) {
                if ( !Abc_ObjIsNode( pObj ) ) {
                    continue;
                }
                if ( !Abc_AigNodeIsAnd( pObj ) ) {
                    continue;
                }
                history[ Abc_ObjId( pObj ) ] = 0;
                minGain = 1;
                Solution solution = NodeRewrite( pObj, MINGAIN );
                if ( solution.leaves != NULL ) {
                    Abc_Obj_t * pObjNew = NodeUpdate( pObj );
                    history[ Abc_ObjId( pObj ) ] = 1;
                }
            }

            Vec_PtrFree( vObj );

        cerr << Abc_NtkNodeNum( pNtk ) << endl;

        }
                
        break;

    case RANDNEG:
        temperature = -1;
        for( iteration = 0, attemps = 0; attemps < runtime && iteration < runtime; attemps ++) {
            Abc_Obj_t * pTarget = WHY_RandNode( pNtk );
            minGain = -1;
            Solution solution = NodeRewrite( pTarget, MINGAIN );
            if ( solution.leaves != NULL ) {

                // we find a valid solution
                iteration ++;
                Abc_Obj_t * pObjNew = NodeUpdate( pTarget );
                Abc_ObjForEachFanin( pObjNew, pObj, i ) {
                    if ( !Abc_AigNodeIsAnd( pObj ) ) {
                        continue;
                    }
                    minGain = 1;
                    Solution leaf = NodeRewrite( pObj, MINGAIN );
                    if ( leaf.leaves != NULL && leaf.gain > 0 ) {
                        NodeUpdate( pObj );
                    }
                }

                minGain = 1;
                Solution root = NodeRewrite( pObjNew, MINGAIN );
                if ( root.leaves != NULL && root.gain > 0 ) {
                    NodeUpdate( pObjNew );
                }

            }
        }
        break;
    default:
        break;
    }

}
