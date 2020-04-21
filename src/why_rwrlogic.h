#ifndef WHY_REWRITE_LOGIC_H
#define WHY_REWRITE_LOGIC_H

#include "why_rwrcore.h"
#include <map>

#define REWRITED    1
#define NOSOL       2

enum RWR_METHOD {
    SEQUENTIAL,
    SIMUANNEAL,
    RANDNEG,
    RANDOM,
    ALTERSEQ,
    NONE
};

class SA {

private:


//======================ABC Attributes===================//
    Rwr_Man_t * pManRwr;
    Cut_Man_t * pManCut;
    Abc_Ntk_t * pNtk;

//======================SA Attributes===================//
    double  T0, temperature, ratio;
    int     iteration;
    int     runtime;
    bool    isAccepted  ( Solution solution );
    void    Anneal      ();

//====================Node Attributes===================//
    map<int, int> history;

//==========================Rewrite======================//
    Solution    NodeRewrite ( Abc_Obj_t * pNode );
    Abc_Obj_t * NodeUpdate  ( Abc_Obj_t * pNode );

public:
    SA( Abc_Ntk_t * pNtk, double _T0, int _runtime, double _ratio );
    ~SA();
    void    Rewrite     ( RWR_METHOD method );

};

#endif