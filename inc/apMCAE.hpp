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

#define DS1820_FACTOR 0.0625
#define CHANNELS 1024
#define SERIAL_PORT_READ_BUF_SIZE 1
#define PMTs 48
#define MAX_HV_VALUE 4095
#define MAX_HIGH_HV_VOLTAGE 1390
#define MIN_HIGH_HV_VOLTAGE 700
#define HV_BUFFER_SIZE 3
#define PMT_BUFFER_SIZE 2
#define RECEIVED_BUFFER_SIZE 4
#define SENDED_BUFFER_SIZE 2
#define CS_BUFFER_SIZE 2
#define CS_CALIB_BUFFER_SIZE 3
#define CRLF_SIZE 2

typedef shared_ptr<serial_port> serial_port_ptr;

namespace ap {

    class MCAE
    {
    private:
        enum string_code {a,b,c,d,e,f,no_value};

    public:
        MCAE(size_t timeout);
        void portReadString(string *msg, char delimeter);
        void portReadBufferString(string *msg, int buffer_size);
        size_t portWrite(string *msg);
        error_code portFlush();
        error_code portConnect(const char *tty_port_name);
        error_code portDisconnect();
        void getMCASplitData(string msg_data, int channels);
        void setMCAEStream(string pmt_dec, int size_stream, string function, string channel_dec="");
        void setPSOCEStream(string function, string psoc_value_dec="");
        double getPMTTemperature(string temp_stream);
        bool isPortOpen();
        bool verifyMCAEStream(string data_received, string data_to_compare);
        ~MCAE();

        // TESTING
        string getCalibTableFormat(QVector<double> table);

    private:
        size_t portRead(string *msg, int buffer_size);
        size_t portRead(char *c_msg);
        string portReadMCAELine();
        string portReadPSOCLine();
        void portReadComplete(const boost::system::error_code& error, size_t bytes_transferred);
        void portTimeOut(const boost::system::error_code& error);
        bool portReadOneChar(char& val);
        void setMCAStream(string pmt, string function, string channel="");
        void setPSOCStream(string function, string psoc_value="");
        int getMCACheckSum(string data);
        string getMCAFormatStream(string data);
        string convertToMCAFormatStream(string data_with_cs);
        string convertFromMCAFormatStream(string data_with_cs);
        MCAE::string_code getMCAStringValues(string const& in_string);
        MCAE::string_code setMCAStringValues(string const& in_string);
        void getMCAHitsData(QByteArray data_mca);
        string getHVValueCode(int channel_dec);
        string getPMTCode(int pmt_dec);
        bool portReadCharArray(int nbytes); /* TODO: Verificar */
        bool verifyStream(string data_received, string data_to_compare);
        string formatMCAEStreamSize(int expected_size, string data_stream);
        bool verifyCheckSum(string data_mca);
        int convertHexToDec(string hex_number);
        string convertDecToHex(int dec_number);
        QByteArray getReverse(QByteArray seq);

    protected:
        io_service io;
        serial_port_ptr port;

    public:
        static const int OK=0000;
        static const int FAILED=0001;
        static const int FILE_NOT_FOUND=0002;
        
    private:
        string FunCHead, FunCSP3, FunCPSOC, BrCst;
        string Init_MCA, Data_MCA, SetHV_MCA, Temp_MCA;
        string Head_Calib, Head_MCAE, End_MCA, End_PSOC;
        string Header_MCAE, Trama_MCAE, Trama_MCA, Trama_PSOC;
        string PSOC_OFF, PSOC_ON, PSOC_SET, PSOC_STA, PSOC_ANS, PSOC_SIZE_SENDED, PSOC_SIZE_RECEIVED;
        string Energy_Calib_Table, X_Calib_Table, Y_Calib_Table, Window_Limits_Table;
        double PSOC_ADC;
        string init_MCA,MCA, HV;
        string AnsMultiInit, AnsHeadInit;
        string AP_ON, AP_OFF;
        string AnsAP_ON, AnsAP_OFF;
        size_t timeout;
        char * data; /* TODO: Verificar */
        bool read_error;
        deadline_timer timer;
        int PortBaudRate;
        string Head_MCA;
        long long time_mca;
        int frame, HV_pmt, offset, var, temp;
        QVector<double> channels_id;
        QVector<double> hits_mca;


    public:
        string getFunCHead() const { return FunCHead; }
        string getFunCSP3() const { return FunCSP3; }
        string getFunCPSOC() const { return FunCPSOC; }
        string getBrCst() const { return BrCst; }
        string getHead_MCAE() const { return Head_MCAE; }
        string getHead_Calib() const { return Head_Calib; }
        string getEnd_MCA() const { return End_MCA; }
        string getEnd_PSOC() const { return End_PSOC; }
        string getHead_MCA() const { return Head_MCA; }
        string getTrama_MCAE() const { return Trama_MCAE; }
        string getTrama_MCA() const { return Trama_MCA; }
        string getTrama_PSOC() const { return Trama_PSOC; }
        string getHeader_MCAE() const { return Header_MCAE; }
        string getEnergy_Calib_Table() const { return Energy_Calib_Table; }
        string getX_Calib_Table() const { return X_Calib_Table; }
        string getY_Calib_Table() const { return Y_Calib_Table; }
        string getWindow_Limits_Table() const { return Window_Limits_Table; }
        string getPSOC_OFF() const { return PSOC_OFF; }
        string getPSOC_ON() const { return PSOC_ON; }
        string getPSOC_SET() const { return PSOC_SET; }
        string getPSOC_STA() const { return PSOC_STA; }
        string getPSOC_ANS() const { return PSOC_ANS; }
        string getPSOC_SIZE_SENDED() const { return PSOC_SIZE_SENDED; }
        string getPSOC_SIZE_RECEIVED() const { return PSOC_SIZE_RECEIVED; }
        double getPSOC_ADC() const { return PSOC_ADC; }
        string getInit_MCA() const { return Init_MCA; }
        string getData_MCA() const { return Data_MCA; }
        string getSetHV_MCA() const { return SetHV_MCA; }
        string getTemp_MCA() const { return Temp_MCA; }        
        string getAnsMultiInit() const { return AnsMultiInit; }
        string getAnsHeadInit() const { return AnsHeadInit; }
        string getAP_ON() const { return AP_ON; }
        string getAP_OFF() const { return AP_OFF; }
        string getAnsAP_ON() const { return AnsAP_ON; }
        string getAnsAP_OFF() const { return AnsAP_OFF; }
        void setHeader_MCAE(string data) { Header_MCAE=data; }
        void setTrama_MCAE(string data){ Trama_MCAE=data; }        
        void setTrama_PSOC(string data){ Trama_PSOC=data; }
        void setTrama_MCA(string data){ Trama_MCA=data; }
        serial_port_ptr getPort() const { return port; }
        int getFrameMCA() const { return frame; }
        long getTimeMCA() const { return time_mca; }
        int getHVMCA() const { return HV_pmt; }
        int getOffSetMCA() const { return offset; }
        int getVarMCA() const { return var; }
        int getTempValueMCA() const { return temp; }        
        QVector<double> getChannels() const { return channels_id; }
        QVector<double> getHitsMCA() const { return hits_mca; }
    };

}

#endif // APMCAE_H
