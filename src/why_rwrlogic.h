#ifndef WHY_REWRITE_LOGIC_H
#define WHY_REWRITE_LOGIC_H

#include "why_rwrcore.h"

enum RWR_METHOD {
    SEQUENTIAL,
    SIMUANNEAL,
    RANDOM,
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

//==========================Rewrite======================//
    Solution    NodeRewrite ( Abc_Obj_t * pNode );
    void        NodeUpdate  ( Abc_Obj_t * pNode );

public:
    SA( Abc_Ntk_t * pNtk, double _T0, int _runtime, double _ratio );
    ~SA();
    void    Rewrite     ( RWR_METHOD method );

};

#endif