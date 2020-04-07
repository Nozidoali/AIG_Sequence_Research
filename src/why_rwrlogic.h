#ifndef WHY_REWRITE_LOGIC_H
#define WHY_REWRITE_LOGIC_H

#include "why_rwrcore.h"

enum RWR_METHOD {
    SEQUENTIAL,
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
    double  T0, temperature;
    int     iteration;
    int     runtime;
    bool    isAccepted  ( int score );
    void    Anneal      ( double step );

//==========================Rewrite======================//
    int     NodeRewrite ( Abc_Obj_t * pNode );
    void    NodeUpdate  ( Abc_Obj_t * pNode );

public:
    SA( Abc_Ntk_t * pNtk, int _T0, int _runtime );
    ~SA();
    void    Rewrite     ( RWR_METHOD method );

};

#endif