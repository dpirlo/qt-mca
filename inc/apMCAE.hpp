/**
 * @class ap::MCAE
 *
 * @brief Clase MCA Extendido
 *
 * Esta clase provee métodos y propiedades para el manejo del protocolo de comunicación del
 * tomográfo por emisión de positrones AR-PET. A partir del uso de esta clase se puede realizar
 * el mantenimiento, configuración y monitoreo del equipo.
 * La aplicación *qt-mca* utiliza esta clase en conjunto con clases de entorno gráfico de Qt.
 *
 * @author Ariel Hernández
 *
 * @version $Version
 *
 * Contacto: ahernandez@cae.cnea.gov.ar
 *           ariel.h.estevenz@ieee.org
 *
 */
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
#define CHANNELS 256
#define CHANNELS_PMT 256
#define SERIAL_PORT_READ_BUF_SIZE 1
#define PMTs 48
#define MAX_HV_VALUE 4095
#define MAX_HIGH_HV_VOLTAGE 1390
#define MIN_HIGH_HV_VOLTAGE 700
#define HV_BUFFER_SIZE 3
#define PMT_BUFFER_SIZE 2
#define TIME_BUFFER_SIZE 2
#define RECEIVED_BUFFER_SIZE 4
#define RECEIVED_RATE_BUFFER_SIZE 20
#define SENDED_BUFFER_SIZE 2
#define CS_BUFFER_SIZE 2
#define CS_CALIB_BUFFER_SIZE 3
#define CRLF_SIZE 2
#define COIN_BUFFER_SIZE 2
#define COIN_BYTES_ADV 9
#define MIN_TEMPERATURE 20

/**
 * @brief serial_port_ptr
 *
 * Definición del tipo serial_port_ptr como un _shared pointer_ de la clase *serial_port*
 */
typedef shared_ptr<serial_port> serial_port_ptr;

/**
 * El Namespace ap contiene los métodos y propiedades relacionados con la comunicación del equipo ARPET.
 */
namespace ap {

  class MCAE
  {
  private:
    /**
         * @brief The string_code enum
         *
         * Definición de enum utilizado para el código de envío de datos serie.
         *
         */
    enum string_code {a,b,c,d,e,f,no_value};

  public:
    /** @note: Se encuentra configurado con un _timeout_ de 150ms por defecto*/
    MCAE(size_t timeout=150);
    void portReadString(string *msg, char delimeter, const char *tty_port_name);
    void portReadBufferString(string *msg, int buffer_size, const char *tty_port_name);
    size_t portWrite(string *msg, const char *tty_port_name);
    size_t sendString(string msg, string end, string port_name);
    string readString(char delimeter, string port_name);
    string readBufferString(int buffer_size, string port_name);
    error_code portFlush();
    error_code portConnect(const char *tty_port_name);
    error_code portDisconnect();
    void getMCASplitData(string msg_data, int channels);
    void setMCAEStream(string pmt_dec, int size_stream, string function, string channel_dec="");
    void setMCAEStream(string pmt_dec, string function, double time);
    void setMCAEStream(string function, QVector<double> table);
    void setMCAEStream(string function, string data_one, string data_two="", bool time=false);
    void setPSOCEStream(string function, string size_received, string psoc_value_dec="");
    bool verifyMCAEStream(string data_received, string data_to_compare);
    string getMCA(string pmt, string function, string head, int channels, string port_name);
    string setHV(string head, string pmt, string channel_dec, string port_name);
    string setCalibTable(string head, string calib_function, QVector<double> table, string port_name);
    string setTime(string head, double time_value, string pmt, string port_name);
    string getTemp(string head, string pmt, string port_name);
    vector<int> getRate(string head, string port_name);
    QVector<QString> parserPSOCStream(string stream);
    double getPMTTemperature(string temp_stream);
    bool isPortOpen();
    ~MCAE();

    /* Area de métodos en testing */


  private:
    size_t portRead(string *msg, int buffer_size);
    size_t portRead(char *c_msg);
    string portReadMCAELine();
    string portReadPSOCLine();
    void portReadComplete(const boost::system::error_code& error, size_t bytes_transferred);
    void portTimeOut(const boost::system::error_code& error);
    bool portReadOneChar(char& val);
    void setMCAStream(string pmt, string function, string channel="");
    void setMCAStream(string pmt, string function, double time);
    void setCalibStream(string function, QVector<double> table);
    void setCoinStream(string function, string data_one, string data_two="", bool time=false);
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
    /** @todo: Verificar el funcionamiento de este método. */
    bool portReadCharArray(int nbytes);
    bool verifyStream(string data_received, string data_to_compare);
    string formatMCAEStreamSize(int expected_size, string data_stream);
    bool verifyCheckSum(string data_mca);
    int convertHexToDec(string hex_number);
    string convertDecToHex(int dec_number);
    string convertDecToHexUpper(int dec_number);
    string getCalibTableFormat(string function, QVector<double> table);
    int convertDoubleToInt(double value);
    vector<int> parserRateStream(string stream);
    string convertToTwoComplement(double value, int two_complement_bits=10);
    string convertToTwoComplement(int value, int two_complement_bits=8);
    QByteArray getReverse(QByteArray seq);

    /* Area de métodos en testing */

  protected:
    /**
         * @brief io
         *
         * Servicio de Input/Output utilizado para la adquisición de datos serie
         */
    io_service io;
    /**
         * @brief port
         *
         * Objeto del tipo _serial_port_ptr_
         *
         * @see serial_port_ptr
         */
    serial_port_ptr port;

  public:
    /**
         * @brief OK
         */
    static const int OK=0000;
    /**
         * @brief FAILED
         */
    static const int FAILED=0001;
    /**
         * @brief FILE_NOT_FOUND
         */
    static const int FILE_NOT_FOUND=0002;

  private:
    string FunCHead, FunCSP3, FunCPSOC, BrCst;
    string Init_Calib_MCAE, Init_MCA, Data_MCA, SetHV_MCA, Temp_MCA, Set_Time_MCA, Rate_MCA;
    string Head_Calib_Coin, Head_MCAE, End_MCA, End_PSOC;
    string Header_MCAE, Trama_MCAE, Trama_MCA, Trama_PSOC, Trama_Calib, Trama_Coin;
    string PSOC_OFF, PSOC_ON, PSOC_SET, PSOC_STA, PSOC_ANS, PSOC_SIZE_SENDED, PSOC_SIZE_RECEIVED, PSOC_SIZE_RECEIVED_ALL;
    string Energy_Calib_Table, X_Calib_Table, Y_Calib_Table, Window_Limits_Table;
    string Init_Coin, Window_Time_Coin, Select_Mode_Coin, Head_Coin, Calib_Mode;
    string Auto_Coin_Mode, Normal_Coin_Mode;
    string Generic_Received_Size, Generic_Sended_Size;
    double PSOC_ADC;
    string init_MCA, MCA, HV;
    string AnsMultiInit, AnsHeadInit;
    string AnsEnergy_Calib_Table, AnsX_Calib_Table, AnsY_Calib_Table, AnsWindow_Limits_Table;
    string AP_ON, AP_OFF, AP_STATUS;
    string AnsAP_ON, AnsAP_OFF;
    size_t timeout;
    /** @todo: Verificar esta propiedad si no es _deprecated_*/
    char * data;
    bool read_error;
    deadline_timer timer;
    int PortBaudRate;
    string Head_MCA;
    long long time_mca;
    int frame, HV_pmt, offset, var, temp;
    QVector<double> channels_id;
    QVector<double> hits_mca;

    /* Area de propiedades en testing */

  public:
    /**
         * @brief getFunCHead
         * @return FunCHead
         */
    string getFunCHead() const { return FunCHead; }
    /**
         * @brief getFunCSP3
         * @return FunCSP3
         */
    string getFunCSP3() const { return FunCSP3; }
    /**
         * @brief getFunCPSOC
         * @return FunCPSOC
         */
    string getFunCPSOC() const { return FunCPSOC; }
    /**
         * @brief getBrCst
         * @return BrCst
         */
    string getBrCst() const { return BrCst; }
    /**
         * @brief getHead_MCAE
         * @return Head_MCAE
         */
    string getHead_MCAE() const { return Head_MCAE; }
    /**
         * @brief getHead_Calib_Coin
         * @return Head_Calib_Coin
         */
    string getHead_Calib_Coin() const { return Head_Calib_Coin; }
    /**
         * @brief getEnd_MCA
         * @return End_MCA
         */
    string getEnd_MCA() const { return End_MCA; }
    /**
         * @brief getEnd_PSOC
         * @return End_PSOC
         */
    string getEnd_PSOC() const { return End_PSOC; }
    /**
         * @brief getHead_MCA
         * @return Head_MCA
         */
    string getHead_MCA() const { return Head_MCA; }
    /**
         * @brief getTrama_MCAE
         * @return Trama_MCAE
         */
    string getTrama_MCAE() const { return Trama_MCAE; }
    /**
         * @brief getTrama_MCA
         * @return Trama_MCA
         */
    string getTrama_MCA() const { return Trama_MCA; }
    /**
         * @brief getTrama_PSOC
         * @return Trama_PSOC
         */
    string getTrama_PSOC() const { return Trama_PSOC; }
    /**
         * @brief getTrama_Calib
         * @return Trama_Calib
         */
    string getTrama_Calib() const { return Trama_Calib; }
    /**
         * @brief getTrama_Coin
         * @return Trama_Coin
         */
    string getTrama_Coin() const {return Trama_Coin; }
    /**
         * @brief getHeader_MCAE
         * @return Header_MCAE
         */
    string getHeader_MCAE() const { return Header_MCAE; }
    /**
         * @brief getEnergy_Calib_Table
         * @return Energy_Calib_Table
         */
    string getEnergy_Calib_Table() const { return Energy_Calib_Table; }
    /**
         * @brief getX_Calib_Table
         * @return X_Calib_Table
         */
    string getX_Calib_Table() const { return X_Calib_Table; }
    /**
         * @brief getY_Calib_Table
         * @return Y_Calib_Table
         */
    string getY_Calib_Table() const { return Y_Calib_Table; }
    /**
         * @brief getWindow_Limits_Table
         * @return Window_Limits_Table
         */
    string getWindow_Limits_Table() const { return Window_Limits_Table; }
    /**
         * @brief getInit_Coin
         * @return Init_Coin
         */
    string getInit_Coin() const { return Init_Coin; }
    /**
         * @brief getWindow_Time_Coin
         * @return Window_Time_Coin
         */
    string getWindow_Time_Coin() const { return Window_Time_Coin; }
    /**
         * @brief getSelect_Mode_Coin
         * @return Select_Mode_Coin
         */
    string getSelect_Mode_Coin() const { return Select_Mode_Coin; }
    /**
         * @brief getAuto_Coin_Mode
         * @return Auto_Coin_Mode
         */
    string getAuto_Coin_Mode() const { return Auto_Coin_Mode; }
    /**
         * @brief getNormal_Coin_Mode
         * @return Normal_Coin_Mode
         */
    string getNormal_Coin_Mode() const { return Normal_Coin_Mode; }
    /**
         * @brief getHead_Coin
         * @return Head_Coin
         */
    string getHead_Coin() const { return Head_Coin; }
    /**
         * @brief getCalib_Mode
         * @return Calib_Mode
         */
    string getCalib_Mode() const { return Calib_Mode; }
    /**
         * @brief getPSOC_OFF
         * @return PSOC_OFF
         */
    string getPSOC_OFF() const { return PSOC_OFF; }
    /**
         * @brief getPSOC_ON
         * @return PSOC_ON
         */
    string getPSOC_ON() const { return PSOC_ON; }
    /**
         * @brief getPSOC_SET
         * @return PSOC_SET
         */
    string getPSOC_SET() const { return PSOC_SET; }
    /**
         * @brief getPSOC_STA
         * @return PSOC_STA
         */
    string getPSOC_STA() const { return PSOC_STA; }
    /**
         * @brief getPSOC_ANS
         * @return PSOC_ANS
         */
    string getPSOC_ANS() const { return PSOC_ANS; }
    /**
         * @brief getPSOC_SIZE_SENDED
         * @return PSOC_SIZE_SENDED
         */
    string getPSOC_SIZE_SENDED() const { return PSOC_SIZE_SENDED; }
    /**
         * @brief getPSOC_SIZE_RECEIVED
         * @return PSOC_SIZE_RECEIVED
         */
    string getPSOC_SIZE_RECEIVED() const { return PSOC_SIZE_RECEIVED; }
    /**
         * @brief getPSOC_SIZE_RECEIVED_ALL
         * @return
         */
    string getPSOC_SIZE_RECEIVED_ALL() const { return PSOC_SIZE_RECEIVED_ALL; }
    /**
         * @brief getPSOC_ADC
         * @return PSOC_ADC
         */
    double getPSOC_ADC() const { return PSOC_ADC; }
    /**
         * @brief getInit_Calib_MCAE
         * @return Init_Calib_MCAE
         */
    string getInit_Calib_MCAE() const { return Init_Calib_MCAE; }
    /**
         * @brief getInit_MCA
         * @return Init_MCA
         */
    string getInit_MCA() const { return Init_MCA; }
    /**
         * @brief getRate_MCA
         * @return
         */
    string getRate_MCA() const { return Rate_MCA; }
    /**
         * @brief getData_MCA
         * @return Data_MCA
         */
    string getData_MCA() const { return Data_MCA; }
    /**
         * @brief getSetHV_MCA
         * @return SetHV_MCA
         */
    string getSetHV_MCA() const { return SetHV_MCA; }
    /**
         * @brief getTemp_MCA
         * @return Temp_MCA
         */
    string getTemp_MCA() const { return Temp_MCA; }
    /**
         * @brief getSet_Time_MCA
         * @return Set_Time_MCA
         */
    string getSet_Time_MCA() const { return Set_Time_MCA; }
    /**
         * @brief getAnsMultiInit
         * @return AnsMultiInit
         */
    string getAnsMultiInit() const { return AnsMultiInit; }
    /**
         * @brief getAnsHeadInit
         * @return AnsHeadInit
         */
    string getAnsHeadInit() const { return AnsHeadInit; }
    /**
         * @brief getAP_ON
         * @return AP_ON
         */
    string getAP_ON() const { return AP_ON; }
    /**
         * @brief getAP_OFF
         * @return AP_OFF
         */
    string getAP_OFF() const { return AP_OFF; }
    /**
         * @brief getAP_STATUS
         * @return AP_STATUS
         */
    string getAP_STATUS() const { return AP_STATUS; }
    /**
         * @brief getAnsAP_ON
         * @return AnsAP_ON
         */
    string getAnsAP_ON() const { return AnsAP_ON; }
    /**
         * @brief getAnsAP_OFF
         * @return AnsAP_OFF
         */
    string getAnsAP_OFF() const { return AnsAP_OFF; }
    /**
         * @brief getAnsEnergy_Calib_Table
         * @return AnsEnergy_Calib_Table
         */
    string getAnsEnergy_Calib_Table() const { return AnsEnergy_Calib_Table; }
    /**
         * @brief getAnsX_Calib_Table
         * @return AnsX_Calib_Table
         */
    string getAnsX_Calib_Table() const { return AnsX_Calib_Table; }
    /**
         * @brief getAnsY_Calib_Table
         * @return AnsY_Calib_Table
         */
    string getAnsY_Calib_Table() const { return AnsY_Calib_Table; }
    /**
         * @brief getAnsWindow_Limits_Table
         * @return AnsWindow_Limits_Table
         */
    string getAnsWindow_Limits_Table() const { return AnsWindow_Limits_Table; }
    /**
         * @brief getGeneric_Received_Size
         * @return Generic_Received_Size
         */
    string getGeneric_Received_Size() const { return Generic_Received_Size; }
    /**
         * @brief getGeneric_Sended_Size
         * @return Generic_Sended_Size
         */
    string getGeneric_Sended_Size() const { return Generic_Sended_Size; }
    /**
         * @brief setHeader_MCAE
         * @param data
         */
    void setHeader_MCAE(string data) { Header_MCAE=data; }
    /**
         * @brief setTrama_MCAE
         * @param data
         */
    void setTrama_MCAE(string data){ Trama_MCAE=data; }
    /**
         * @brief setTrama_PSOC
         * @param data
         */
    void setTrama_PSOC(string data){ Trama_PSOC=data; }
    /**
         * @brief setTrama_MCA
         * @param data
         */
    void setTrama_MCA(string data){ Trama_MCA=data; }
    /**
         * @brief setTrama_Calib
         * @param data
         */
    void setTrama_Calib(string data){ Trama_Calib=data; }
    /**
         * @brief setTrama_Coin
         * @param data
         */
    void setTrama_Coin(string data){ Trama_Coin=data; }
    /**
         * @brief getPort
         * @return port
         */
    serial_port_ptr getPort() const { return port; }
    /**
         * @brief getFrameMCA
         * @return frame
         */
    int getFrameMCA() const { return frame; }
    /**
         * @brief getTimeMCA
         * @return time_mca
         */
    long getTimeMCA() const { return time_mca; }
    /**
         * @brief getHVMCA
         * @return HV_pmt
         */
    int getHVMCA() const { return HV_pmt; }
    /**
         * @brief getOffSetMCA
         * @return offset
         */
    int getOffSetMCA() const { return offset; }
    /**
         * @brief getVarMCA
         * @return var
         */
    int getVarMCA() const { return var; }
    /**
         * @brief getTempValueMCA
         * @return temp
         */
    int getTempValueMCA() const { return temp; }
    /**
         * @brief getChannels
         * @return channels_id
         */
    QVector<double> getChannels() const { return channels_id; }
    /**
         * @brief getHitsMCA
         * @return hits_mca
         */
    QVector<double> getHitsMCA() const { return hits_mca; }
    /**
         * @brief resetHitsMCA
         */
    void resetHitsMCA() { hits_mca.clear(); }
  };

}

#endif // APMCAE_H
