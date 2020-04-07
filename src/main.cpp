#include "why_rwrlogic.h"

parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("input", 'i', "Original Circuit file", true);
    option.parse_check(argc, argv);
    return option;
}


void RenameNtk(Abc_Ntk_t * pNtk, string & fileName)
{
    uint32_t pos0 = fileName.find(".blif");
    DASSERT(pos0 != fileName.npos);
    uint32_t pos1 = fileName.rfind("/");
    if (pos1 == fileName.npos)
        pos1 = -1;
    Ckt_NtkRename(pNtk, fileName.substr(pos1 + 1, pos0 - pos1 - 1).c_str());
}


int main(int argc, char * argv[])
{
    // command line parser
    parser option = Cmdline_Parser(argc, argv);
    string input = option.get <string> ("input");

    // initilize ABC
    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    ostringstream command("");
    command << "read_blif " << input;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));

    Abc_Ntk_t * pNtk = Abc_NtkDup(Abc_FrameReadNtk(pAbc));
    Abc_Ntk_t * pNtkNew;
    DASSERT(pNtkNew = Abc_NtkStrash(pNtk, 0, 1, 0));
    Abc_NtkDelete(pNtk);
    DASSERT(Abc_NtkIsStrash(pNtkNew));
    DASSERT(!Abc_NtkGetChoiceNum(pNtkNew));

    // cout << Abc_NtkNodeNum(pNtkNew) << "," << Abc_NtkLevel(pNtkNew) << ",";

    SA * sa = new SA( pNtkNew, 10.0, 1000 );
    sa->Rewrite( SEQUENTIAL );
    delete sa;

    // cout << Abc_NtkNodeNum(pNtkNew) << "," << Abc_NtkLevel(pNtkNew) << ",";
    Abc_NtkDelete(pNtkNew);

    // recycle memory
    Abc_Stop();

    return 0;
}
