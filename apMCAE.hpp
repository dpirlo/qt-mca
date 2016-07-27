#ifndef APMCAE_H
#define APMCAE_H

#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <stdlib.h>
#include <iostream>
#include <vector>
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
    private:
        enum string_code {a,b,c,d,e,f,no_value};

    public:
        MCAE(size_t timeout);
        void setStrings();        
        error_code portConnect(const char *tty_port_name);
        error_code portDisconnect();
        bool isPortOpen();
        size_t portWrite(string *msg);
        size_t portRead(string *msg, int buffer_size);
        size_t portRead(char *c_msg);
        string portReadMCAELine();
        string portReadPSOCLine();
        void portReadComplete(const boost::system::error_code& error, size_t bytes_transferred);
        void portTimeOut(const boost::system::error_code& error);
        bool portReadOneChar(char& val);
        void portReadString(string *msg, char delimeter);
        void portReadBufferString(string *msg, int buffer_size);
        int convertHexToDec(string hex_number);
        string convertDecToHex(int dec_number);
        QByteArray getReverse(QByteArray seq);
        void setMCAStream(string pmt, string function);
        void setMCAEStream(string pmt, string size_sended, string size_received, string function);
        error_code portFlush();
        int getMCACheckSum(string data_function, string data_pmt);
        string getMCAFormatStream(string data);
        string convertMCAFormatStream(string data_with_cs);
        MCAE::string_code getMCAStringValues(string const& in_string);
        void getMCASplitData(string msg_data, int channels);
        void getMCAHitsData(QByteArray data_mca);
        ~MCAE();

        /* Pruebas*/

        bool portReadCharArray(int nbytes);
        char * data;

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
        string Init_MCA, Data_MCA, SetHV_MCA;
        string Head_MCAE, End_MCA, End_HV;
        string Header_MCAE, Trama_MCAE, Trama_MCA;
        string HV_OFF, HV_ON;
        string init_MCA,MCA, HV;
        size_t timeout;       
        bool read_error;
        deadline_timer timer;
        int PortBaudRate;
        string Head_MCA;
        long long time_mca;
        int frame, HV_pmt, offset, var, temp;
        vector<int> channels_id;
        vector<long long> hits_mca;

    public:
        string getFunCHead() const { return FunCHead; }
        string getFunCSP3() const { return FunCSP3; }
        string getFunCHV() const { return FunCHV; }
        string getHead_MCAE() const { return Head_MCAE; }
        string getEnd_MCA() const { return End_MCA; }
        string getEnd_HV() const { return End_HV; }
        string getHead_MCA() const { return Head_MCA; }
        string getTrama_MCAE() const { return Trama_MCAE; }
        string getTrama_MCA() const { return Trama_MCA; }
        string getHeader_MCAE() const { return Header_MCAE; }
        string getHV_OFF() const { return HV_OFF; }
        string getHV_ON() const { return HV_ON; }
        string getInit_MCA() const { return Init_MCA; }
        string getData_MCA() const { return Data_MCA; }
        string getSetHV_MCA() const { return SetHV_MCA; }
        void setHeader_MCAE(string data) { Header_MCAE=data; }
        void setTrama_MCAE(string data){ Trama_MCAE=data; }
        void setTrama_MCA(string data){ Trama_MCA=data; }
        serial_port_ptr getPort() const { return port; }
        int getFrameMCA() const { return frame; }
        long long getTimeMCA() const { return time_mca; }
        int getHVMCA() const { return HV_pmt; }
        int getOffSetMCA() const { return offset; }
        int getVarMCA() const { return var; }
        int getTempMCA() const { return temp; }
        vector<int> getChannels() const { return channels_id; }
        vector<long long> getHitsMCA() const { return hits_mca; }
    };

}

/*

string tramaRx_Init_Head = "@0064020<", tramaRx_Init_Slaves = "@0064310>", tramaRx_Set_HV = "@0086", tramaRx_Tabla_E = "&101", tramaRx_Tabla_X = "&102", tramaRx_Tabla_T = "&105" , tramaRx_Tabla_Y = "&103", tramaRx_Tabla_Est = "&104", tramaRx_Tabla_err = "&666";
string tramaHV_ON = "$SET,STA,ON", tramaHV_OFF = "$SET,S,OFF", tramaHV_SET = "$SET,VCO,", tramaHV_STATUS = "$TEMP", tramaRx_HV = "DEPENDE LA CANTIDAD DE SENSORES";
string largo_trama_set_HV = "0003", largo_trama_status_hv ="0061";

int timeoutRx = 0;
const int TIMEOUTRX = 200;

*/

#endif // APMCAE_H
