#ifndef APMCAE_H
#define APMCAE_H

#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "apExceptions.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::system;

#define SERIAL_PORT_READ_BUF_SIZE 1

typedef shared_ptr<serial_port> serial_port_ptr;

namespace ap {

    class MCAE
    {
    public:
        MCAE();
        void setStrings();
        void portInit();
        error_code portConnect(const char *tty_port_name, int baud_rate);
        error_code portDisconnect();
        size_t portWrite(string *msg);
        size_t portRead(string *msg, int buffer_size);
        size_t portRead(char *c_msg);
        string portReadMCAELine();
        string portReadPSOCLine();
        bool isPortOpen();
        ~MCAE();
    private:
        /* Pruebas*/

    protected:
        io_service io;
        serial_port_ptr port;

    public:
        static const int OK=0000;
        static const int FAILED=0001;
        static const int FILE_NOT_FOUND=0002;
        
    private:
        string FunCHead, FunCSP3, FunCHV;
        string Head_MCAE, End_MCA, End_HV, Head_MCA;
        string Header_MCAE, Trama_MCAE;
        string HV_OFF, HV_ON;

    public:
        string getFunCHead() const { return FunCHead; }
        string getFunCSP3() const { return FunCSP3; }
        string getFunCHV() const { return FunCHV; }
        string getHead_MCAE() const { return Head_MCAE; }
        string getEnd_MCA() const { return End_MCA; }
        string getEnd_HV() const { return End_HV; }
        string getHead_MCA() const { return Head_MCA; }
        string getTrama_MCAE() const { return Trama_MCAE; }
        string getHeader_MCAE() const { return Header_MCAE; }
        string getHV_OFF() const { return HV_OFF; }
        string getHV_ON() const { return HV_ON; }
        void setHeader_MCAE(string data) { Header_MCAE=data; }
        void setTrama_MCAE(string data){ Trama_MCAE=data; }
        serial_port_ptr getPort() const { return port; }

    };

}

/*

string tramaRx_Init_Head = "@0064020<", tramaRx_Init_Slaves = "@0064310>", tramaRx_Set_HV = "@0086", tramaRx_Tabla_E = "&101", tramaRx_Tabla_X = "&102", tramaRx_Tabla_T = "&105" , tramaRx_Tabla_Y = "&103", tramaRx_Tabla_Est = "&104", tramaRx_Tabla_err = "&666";
string tramaHV_ON = "$SET,STA,ON", tramaHV_OFF = "$SET,S,OFF", tramaHV_SET = "$SET,VCO,", tramaHV_STATUS = "$TEMP", tramaRx_HV = "DEPENDE LA CANTIDAD DE SENSORES";
string largo_trama_set_HV = "0003", largo_trama_status_hv ="0061";

string tramaMCA;
string Encabezado_MCA = "#C";
string fin_de_Trama_MCA = Convert.ToString('\r'), fin_de_Trama_HV = Convert.ToString('\r') + Convert.ToString('\n');
int timeoutRx = 0;
const int TIMEOUTRX = 200;

*/

#endif // APMCAE_H
