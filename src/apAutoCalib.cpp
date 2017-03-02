#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib(QList<int> checked_PMTs, QList<int> checked_Cab, float Canal_Obj_par)
{

    this->PMTs_List = checked_PMTs;
    this->Cab_List = checked_Cab;
    this->Canal_Obj = Canal_Obj_par;

}

bool AutoCalib::calibrar_simple()
{
    for(int i=0; i<PMTs_List.length();i++) { cout<<PMTs_List[i]<<endl; }
    for(int i=0; i<Cab_List.length();i++) { cout<<Cab_List[i]<<endl; }
    cout<<"Canal Objetivo:"<<Canal_Obj<<endl;

    return 1;
}
