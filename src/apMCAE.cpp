#include "inc/apMCAE.hpp"

using namespace ap;

/**
 * @brief MCAE::MCAE
 *
 * Constructor de la clase
 *
 * Se inicializan todas las propiedades respecto a las funciones de envío de datos serie con los protocolos: MCA, PSOC, Coincidencias y Calibración.
 * Al conjunto que engloba todos estos protocolos se lo denomina MCA Extendido. Esta clase contiene todos los métodos y propiedades para acceder, administrar,
 * configurar y monitoriar el tomógrafo por emisión de positrones ARPET.
 *
 * Solo admite como parámetro el tiempo que debe esperar el servicio de comunicación antes de emitir una excepción por _time out_. El tiempo por defecto es de 1 segundo.
 *
 * @note Se documentan las propiedades más importantes.
 *
 * @brief PortBaudRate:
 * Velocidad de funcionamiento de la comunicación con el equipo
 *
 * @brief PSOC_ADC:
 * Tensión por cada unidade de ADC
 * 1 Unidad de ADC = 5.8823 V
 *
 * @param timeout
 */
MCAE::MCAE(size_t timeout)
  :port(serial_port_ptr(new serial_port(io))),
    timeout(timeout),
    read_error(true),
    timer(port->get_io_service()),
    PortBaudRate(921600),
    AnsAP_ON("ON"),
    AnsAP_OFF("OFF"),
    AP_ON("ARPETON"),
    AP_OFF("ARPETOFF"),
    AP_STATUS("ARPETO?"),
    Head_MCAE("#C"),
    Head_MCA("@"),
    End_MCA("\r"),
    End_PSOC("\r\n"),

    /*Funciones trama MCAE*/
    FunCHead("01"),
    FunCSP3("02"),
    FunCPSOC("03"),

    /*Funciones de Tablas*/
    Head_Calib_Coin("&"),
    Energy_Calib_Table("001"),
    X_Calib_Table("002"),
    Y_Calib_Table("003"),
    Window_Limits_Table("004"),
    AnsEnergy_Calib_Table("&101"),
    AnsX_Calib_Table("&102"),
    AnsY_Calib_Table("&103"),
    AnsWindow_Limits_Table("&104"),
    Generic_Received_Size("0001"),
    Generic_Sended_Size("01"),

    /*Funciones trama Coincidencia*/
    Init_Coin("701"),
    Window_Time_Coin("702"),
    Select_Mode_Coin("703"),
    Normal_Coin_Mode("333333333"),
    Auto_Coin_Mode("020102121"),
    Head_Coin("7"),
    Calib_Mode("6411"),

    /*Funciones trama PSOC*/
    PSOC_OFF("$SET,STA,OFF"),
    PSOC_ON("$SET,STA,ON"),
    PSOC_SET("$SET,VCON,"),
    PSOC_STA("$TEMP"),
    PSOC_ANS("$OK"),
    PSOC_ADC(5.8823),
    PSOC_SIZE_SENDED("14"),
    PSOC_SIZE_RECEIVED("0031"),
    PSOC_SIZE_RECEIVED_ALL("0003"),

    /*Funciones trama MCA*/
    AnsMultiInit("@0064310>"),
    AnsHeadInit("@0064020<"),
    BrCst("00"),
    Init_Calib_MCAE("64"),
    Init_MCA("6401"),
    Data_MCA("65"),
    SetHV_MCA("68"),
    Temp_MCA("74000"),
    Set_Time_MCA("80"),
    Rate_MCA("0060")
{
  /* Testing */
}
/**
 * @brief MCAE::~MCAE
 *
 * Destructor de la clase MCAE
 *
 */
MCAE::~MCAE()
{
  portDisconnect();
}
/**
 * @brief MCAE::isPortOpen
 * @return Si está abierto responde _true_
 */
bool MCAE::isPortOpen()
{
  return port->is_open();
}
/**
 * @brief MCAE::portConnect
 *
 * Conexión del objeto puerto serie _port_
 *
 * @param tty_port_name
 * @return Código de error
 */
error_code MCAE::portConnect(const char *tty_port_name)
{
  error_code error_code;
  port->open(tty_port_name, error_code);
  port->set_option(serial_port_base::baud_rate(PortBaudRate));

  return error_code;
}
/**
 * @brief MCAE::portDisconnect
 *
 * Desconexión del objeto puerto serie _port_
 *
 * @return Código de error
 */
error_code MCAE::portDisconnect()
{
  error_code error_code;
  if (port->is_open())
    port->close(error_code);

  return error_code;
}
/**
 * @brief MCAE::portWrite
 *
 * Escritura sobre el objeto puerto serie _port_
 *
 * @param msg
 * @param tty_port_name
 * @return Tamaño de la trama en bytes
 */
size_t MCAE::portWrite(string *msg, const char *tty_port_name)
{
  char c_msg[msg->size()+1];
  strcpy(c_msg, msg->c_str());
  size_t bytes_transferred = port->write_some(boost::asio::buffer(c_msg,msg->size()));

  return bytes_transferred;
}
/**
 * @brief MCAE::portRead
 *
 * Lectura sobre el objeto puerto serie _port_ y el resultado se guarda en un puntero a _string_
 *
 * @param msg
 * @param buffer_size
 * @return Tamaño de la trama en bytes
 */
size_t MCAE::portRead(string *msg, int buffer_size)
{
  char c_msg[buffer_size];
  size_t bytes_transferred = port->read_some(boost::asio::buffer(c_msg,buffer_size));
  msg->assign(c_msg);

  return bytes_transferred;
}
/**
 * @brief MCAE::portRead
 * @overload
 *
 * Lectura sobre el objeto puerto serie _port_ y el resultado se guarda en un puntero a _char_
 *
 * @param c_msg
 * @return Tamaño de la trama en bytes
 */
size_t MCAE::portRead(char *c_msg)
{
  int buffer_size=1;
  size_t bytes_transferred = port->read_some(boost::asio::buffer(c_msg,buffer_size));

  return bytes_transferred;
}
/**
 * @brief MCAE::portReadMCAELine
 *
 * Lectura por línea sobre el objeto puerto serie _port_ y el resultado es un _string_
 *
 * @return La línea leída en _string_
 */
string MCAE::portReadMCAELine()
{
  char c;
  string msg;
  while(true) {
      portRead(&c);
      switch(c)
        {
        case '\r':
          msg+=c;
          return msg;
        default:
          msg+=c;
        }
    }
  return msg;
}
/**
 * @brief MCAE::portReadPSOCLine
 *
 * Lectura por línea sobre el objeto puerto serie _port_ y el resultado es un _string_
 *
 * @return La línea leída en _string_
 */
string MCAE::portReadPSOCLine()
{
  char c;
  string msg;
  while(true) {
      portRead(&c);
      switch(c)
        {
        case '\r':
          msg+=c;
          break;
        case '\n':
          msg+=c;
          return msg;
        default:
          msg+=c;
        }
    }
  return msg;
}
/**
 * @brief MCAE::portReadComplete
 *
 * Lectura completa sobre el objeto puerto serie _port_ hasta agotar el temporizador
 *
 * @param error
 * @param bytes_transferred
 */
void MCAE::portReadComplete(const boost::system::error_code& error,
                            size_t bytes_transferred)
{
  read_error = (error || bytes_transferred == 0);
  timer.cancel();
}
/**
 * @brief MCAE::portTimeOut
 *
 * Cuando se llama a este método cancela la actividad sobre el objeto _port_
 *
 * @param error
 */
void MCAE::portTimeOut(const boost::system::error_code& error)
{
  if (error) { return; }
  port->cancel();
}
/**
 * @brief MCAE::portReadOneChar
 *
 * Lectura de a un _char_ sobre el objeto puerto serie _port_ y el resultado se guarda en un puntero a _char_
 *
 * @param val
 * @return El estado de la lectura, _true_ si no hubo inconvenientes.
 */
bool MCAE::portReadOneChar(char& val)
{
  char c;
  val = c = '\0';

  port->get_io_service().reset();
  port->async_read_some(boost::asio::buffer(&c, 1),
                        boost::bind(&MCAE::portReadComplete, this,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred));
  timer.expires_from_now(boost::posix_time::milliseconds(timeout));
  timer.async_wait(boost::bind(&MCAE::portTimeOut,
                               this, boost::asio::placeholders::error));

  port->get_io_service().run();

  if (!read_error)
    val = c;

  return !read_error;
}
/**
 * @brief MCAE::portReadString
 *
 * Lectura sobre el objeto puerto serie _port_ y el resultado va a un puntero _string_
 *
 * @param msg
 * @param delimeter
 * @param tty_port_name
 */
void MCAE::portReadString(string *msg, char delimeter, const char *tty_port_name)
{
  char c;
  while (portReadOneChar(c) && c != delimeter)
    {
      msg->push_back(c);
    }

  if (c != delimeter)
    {
      Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
      throw exception_timeout;
    }
}
/**
 * @brief MCAE::portReadBufferString
 *
 * Lectura de un determinado _buffer_ sobre el objeto puerto serie _port_ y el resultado va a un puntero _string_
 *
 * @param msg
 * @param buffer_size
 * @param tty_port_name
 */
void MCAE::portReadBufferString(string *msg, int buffer_size, const char *tty_port_name)
{
  char c;
  int buffer=1;
  while (portReadOneChar(c) && buffer <= buffer_size)
    {
      msg->push_back(c);
      buffer++;
    }

  if (buffer <= buffer_size)
    {
      Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
      throw exception_timeout;
    }
}
/**
 * @brief MCAE::portReadCharArray
 * @deprecated
 *
 * Lectura de un determinado _array_ sobre el objeto puerto serie _port_ y el resultado va a un puntero _char_
 *
 * @param nbytes
 * @return _true_ si se logra leer satisfactoriamente
 */
bool MCAE::portReadCharArray(int nbytes)
{
  try
  {
    port->read_some(boost::asio::buffer(data, nbytes));
  }
  catch (const boost::system::system_error &ex)
  {
    std::cout << "Error: " << ex.what() << "\n";
    return false;
  }
  return true;
}
/**
 * @brief MCAE::portFlush
 *
 * Limpieza del _buffer_ de lectura del objeto _port_
 *
 * @return Código de error
 */
error_code MCAE::portFlush()
{
  error_code ec;

  const bool isFlushed =! ::tcflush(port->native(), TCIOFLUSH);
  if (!isFlushed)
    ec = error_code(errno,error::get_system_category());

  return ec;
}
/**
 * @brief MCAE::sendString
 *
 * Envío de la trama 'msg' al puerto serie
 *
 * @param msg
 * @param end
 * @param port_name
 * @return
 */
size_t MCAE::sendString(string msg, string end, string port_name)
{
  portFlush();
  size_t bytes_transfered = 0;

  try{
    string sended= msg + end;
    bytes_transfered = portWrite(&sended, port_name.c_str());
  }
  catch(boost::system::system_error e){
    Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
    throw exception_serial_port;
  }

  return bytes_transfered;
}
/**
 * @brief MCAE::readString
 *
 * Lectura del 'buffer' serie hasta recibir el _delimeter_
 *
 * @param delimeter
 * @param port_name
 * @return
 */
string MCAE::readString(char delimeter, string port_name)
{
  string msg;
  try{
    portReadString(&msg,delimeter, port_name.c_str());
  }
  catch( Exceptions & ex ){
    Exceptions exception_stop(ex.excdesc);
    throw exception_stop;
  }
  return msg;
}
/**
 * @brief MCAE::readBufferString
 *
 * Lectura de un buffer de tamaño 'buffer_size'
 *
 * @param buffer_size
 * @param port_name
 * @return
 */
string MCAE::readBufferString(int buffer_size, string port_name)
{
  string msg;
  try{
    portReadBufferString(&msg,buffer_size, port_name.c_str());
  }
  catch( Exceptions & ex ){
    Exceptions exception_stop(ex.excdesc);
    throw exception_stop;
  }
  return msg;
}
/**
 * @brief MCAE::convertHexToDec
 *
 * Método de conversión de Hexadecimal (_string_) a Decimal (_int_)
 *
 * @param hex_number_s
 * @return dec_number
 */
int MCAE::convertHexToDec(string hex_number_s)
{
  bool ok;
  QString hex_number = QString::fromStdString(hex_number_s);
  int dec_number = hex_number.toInt(&ok,16);

  return dec_number;
}
/**
 * @brief MCAE::convertDecToHex
 *
 * Método de conversión de Decimal (_int_) a Hexadecimal (_string_) en minúsculas (abcdef)
 *
 * @param dec_number
 * @return hex_number
 */
string MCAE::convertDecToHex(int dec_number)
{
  QByteArray hex_number = QByteArray::number(dec_number,16);

  return QString(hex_number).toStdString();
}
/**
 * @brief MCAE::convertDecToHexUpper
 *
 * Método de conversión de Decimal (_int_) a Hexadecimal (_string_) en mayúsculas (ABCDEF)
 *
 * @param dec_number
 * @return hex_number
 */
string MCAE::convertDecToHexUpper(int dec_number)
{
  QByteArray hex_number = QByteArray::number(dec_number,16).toUpper();

  return QString(hex_number).toStdString();
}
/**
 * @brief MCAE::getReverse
 *
 * Método de inversión de _bytes_
 *
 * @param seq
 * @return Secuencia _seq_ en sentido inverso
 */
QByteArray MCAE::getReverse(QByteArray seq)
{
  QByteArray reverse;
  for( QByteArray::const_iterator i = seq.constEnd(); i !=seq.constBegin(); )
    {
      --i;
      reverse += *i;
    }

  return reverse;
}
/**
 * @brief MCAE::getMCASplitData
 *
 * Método que realiza la segmentación de la trama MCA en los siguientes campos:
 * > Sincronismo
 * > tiempo de adquisición
 * > HV del fotomultiplicador
 * > nivel de offset del ADC
 * > potencia de ruido (varianza)
 * > temperatura del fotomultiplicador
 * > Cuentas MCA por canal
 *
 * @param msg_data
 * @param channels
 */
void MCAE::getMCASplitData(string msg_data, int channels)
{
  int size_block=6*channels;
  QByteArray q_msg_data(msg_data.c_str(), msg_data.length());

  /* Adquisición de los bytes en raw data */
  frame=convertHexToDec(getReverse(q_msg_data.left(4)).toHex().toStdString());
  time_mca=convertHexToDec(getReverse(q_msg_data.mid(5, 5)).toHex().toStdString());
  HV_pmt=convertHexToDec(getReverse(q_msg_data.mid(9, 2)).toHex().toStdString());
  offset=convertHexToDec(q_msg_data.mid(11, 1).toHex().toStdString());
  var=convertHexToDec(getReverse(q_msg_data.mid(12, 2)).toHex().toStdString());
  temp=convertHexToDec(getReverse(q_msg_data.mid(14, 2)).toHex().toStdString())*DS1820_FACTOR;
  QByteArray data_mca_bytes = q_msg_data.right(size_block);

  /* Parseo de datos de la trama que contiene las cuentas por canal */
  getMCAHitsData(data_mca_bytes);
}
/**
 * @brief MCAE::getMCAHitsData
 *
 * Método que realiza la segmentación de la trama de cuentas de MCA por canal y los guarda en los vectores *channels_id* y *hits_mca*
 *
 * @param data_mca
 */
void MCAE::getMCAHitsData(QByteArray data_mca)
{
  int channel;
  long hits;

  channels_id.resize(data_mca.length());
  hits_mca.resize(data_mca.length());

  channels_id.fill(0);
  hits_mca.fill(0);

  for(int i = 0; i < data_mca.length(); i+=6)
    {
      channel=convertHexToDec(getReverse(data_mca.mid(i,2)).toHex().toStdString());
      hits=(long)convertHexToDec(getReverse(data_mca.mid(i+2,4)).toHex().toStdString());
      channels_id[channel]=channel;
      hits_mca[channel]=hits;
    }
}
/**
 * @brief MCAE::getMCACheckSum
 *
 * Cálculo del _checksum_ en la trama de datos
 *
 * @param data
 * @return El valor del _checksum_
 */
int MCAE::getMCACheckSum(string data)
{
  int sum_of_elements = 0;
  for(unsigned int i = 0; i < data.length(); ++i)
    {
      string token(1, data.at(i));
      sum_of_elements = sum_of_elements + convertHexToDec(token);
    }

  return sum_of_elements;
}
/**
 * @brief MCAE::getMCAStringValues
 *
 * Se obtiene el dato en _string_code_ a partir de un _string_
 *
 * @param in_string
 * @return Código _string_code_
 */
MCAE::string_code MCAE::getMCAStringValues(string const& in_string)
{
  if (in_string == "a") return a;
  if (in_string == "b") return b;
  if (in_string == "c") return c;
  if (in_string == "d") return d;
  if (in_string == "e") return e;
  if (in_string == "f") return f;
  else return no_value;
}
/**
 * @brief MCAE::setMCAStringValues
 *
 * Se configura el dato en _string_code_ a partir de un dato _ASCII_ en la trama de datos MCA
 *
 * @param in_string
 * @return Código _string_code_
 */
MCAE::string_code MCAE::setMCAStringValues(string const& in_string)
{
  if (in_string == ":") return a;
  if (in_string == ";") return b;
  if (in_string == "<") return c;
  if (in_string == "=") return d;
  if (in_string == ">") return e;
  if (in_string == "?") return f;
  else return no_value;
}
/**
 * @brief MCAE::convertToMCAFormatStream
 *
 * Conversión de la trama en formato de envío MCA.
 * Formato de data con checksum: @ddcc--...--ss
 *
 * @param data_with_cs
 * @return data_with_cs con formato MCA
 */
string MCAE::convertToMCAFormatStream(string data_with_cs)
{
  size_t pos = 0;

  while (pos < data_with_cs.length())
    {
      string token = data_with_cs.substr(pos, 1);
      switch (getMCAStringValues(token)) {
        case a:
          data_with_cs.replace(pos,token.length(),":");
          break;
        case b:
          data_with_cs.replace(pos,token.length(),";");
          break;
        case c:
          data_with_cs.replace(pos,token.length(),"<");
          break;
        case d:
          data_with_cs.replace(pos,token.length(),"=");
          break;
        case e:
          data_with_cs.replace(pos,token.length(),">");
          break;
        case f:
          data_with_cs.replace(pos,token.length(),"?");
          break;
        default:
          break;
        }
      pos++;
    }

  return data_with_cs;
}
/**
 * @brief MCAE::convertFromMCAFormatStream
 *
 * Conversión de la trama MCA en formato _string_ para lectura de datos
 * Formato de data con checksum: @ddcc--...--ss
 *
 * @param data_with_cs
 * @return data_with_cs con formato _string_
 */
string MCAE::convertFromMCAFormatStream(string data_with_cs)
{
  /* Formato de data con checksum:
     * @ddcc--...--ss
     */

  size_t pos = 0;

  while (pos < data_with_cs.length())
    {
      string token = data_with_cs.substr(pos, 1);
      switch (setMCAStringValues(token)) {
        case a:
          data_with_cs.replace(pos,token.length(),"a");
          break;
        case b:
          data_with_cs.replace(pos,token.length(),"b");
          break;
        case c:
          data_with_cs.replace(pos,token.length(),"c");
          break;
        case d:
          data_with_cs.replace(pos,token.length(),"d");
          break;
        case e:
          data_with_cs.replace(pos,token.length(),"e");
          break;
        case f:
          data_with_cs.replace(pos,token.length(),"f");
          break;
        default:
          break;
        }
      pos++;
    }

  return data_with_cs;
}

/**
 * @brief MCAE::getMCAFormatStream
 *
 * Conversión de la trama en formato MCA para el envío de datos
 * Formato de _data_ sin checksum: @ddcc--...--
 *
 * @param data
 * @return data con _checksum_ en formato MCA
 */
string MCAE::getMCAFormatStream(string data)
{

  string checksum=formatMCAEStreamSize(CS_BUFFER_SIZE, convertDecToHex(getMCACheckSum(data)));
  string data_plus_checksum = data + checksum;
  data_plus_checksum = Head_MCA + data_plus_checksum;
  string data_plus_checksum_mca_format=convertToMCAFormatStream(data_plus_checksum);

  return data_plus_checksum_mca_format;
}
/**
 * @brief MCAE::setMCAStream
 *
 * Configuración de la trama MCA
 *
 * @param pmt
 * @param function
 * @param channel
 */
void MCAE::setMCAStream(string pmt, string function, string channel)
{
  string stream_wo_cs=pmt+function+channel;
  setTrama_MCA(getMCAFormatStream(stream_wo_cs));
}
/**
 * @brief MCAE::setMCAStream
 * @overload
 *
 * Configuración de la trama MCA
 *
 * @param pmt
 * @param function
 * @param time
 */
void MCAE::setMCAStream(string pmt, string function, double time)
{
  string time_str=QString::number(time).toStdString();
  string stream_wo_cs=pmt+function+time_str;
  setTrama_MCA(getMCAFormatStream(stream_wo_cs));
}
/**
 * @brief MCAE::setPSOCStream
 *
 * Configuración de la trama PSOC
 *
 * @param function
 * @param psoc_value
 */
void MCAE::setPSOCStream(string function, string psoc_value)
{
  string stream_psoc;
  stream_psoc=function+psoc_value;
  setTrama_PSOC(stream_psoc);
}
/**
 * @brief MCAE::convertDoubleToInt
 *
 * Conversión de un valor en _double_ a _int_
 * int=(int)round(double)
 *
 * @param value
 * @return value_int
 */
int MCAE::convertDoubleToInt(double value)
{
  int value_int;
  /** @note: Se elimina esta línea: value=value*1000; Modificado en el firmware de FPGA*/

  return value_int=(int)round(value);
}
/**
 * @brief MCAE::convertToTwoComplement
 *
 * Conversión de un valor _double_ negativo a complemento a 2
 *
 * @param value
 * @param two_complement_bits
 * @return Valor en complemento a 2 en Hexadecimal (upper)
 */
string MCAE::convertToTwoComplement(double value, int two_complement_bits)
{
  int value_int = (1 << two_complement_bits) + convertDoubleToInt(value);

  return convertDecToHexUpper(value_int);
}
/**
 * @brief MCAE::convertToTwoComplement
 * @overload
 *
 * Conversión de un valor _int_ negativo a complemento a 2
 *
 * @param value
 * @param two_complement_bits
 * @return Valor en complemento a 2 en Hexadecimal (lower)
 */
string MCAE::convertToTwoComplement(int value, int two_complement_bits)
{
  int value_int = (1 << two_complement_bits) + value;

  return convertDecToHex(value_int);
}
/**
 * @brief MCAE::getCalibTableFormat
 *
 * Se obtiene la trama a partir de la tabla configurada en la etapa de calibración
 *
 * @param function
 * @param table
 * @return Trama para calibración
 */
string MCAE::getCalibTableFormat(string function, QVector<double> table)
{
  string calib_stream, temp_calib_stream;
  int file=QString::fromStdString(function).toInt();

  switch (file) {
    case 1:
      for (int index=0; index < PMTs; index++) calib_stream = calib_stream + formatMCAEStreamSize(CS_CALIB_BUFFER_SIZE, convertDecToHexUpper(convertDoubleToInt(table[index])));
      break;
    case 2 ... 3:
      for (int index=0; index < PMTs; index++)
        {
          if(table[index]>=0)
            temp_calib_stream = formatMCAEStreamSize(CS_CALIB_BUFFER_SIZE, convertDecToHexUpper(convertDoubleToInt(table[index])));
          else
            temp_calib_stream = formatMCAEStreamSize(CS_CALIB_BUFFER_SIZE, convertToTwoComplement(table[index]));

          calib_stream = calib_stream + temp_calib_stream;
        }
      break;
    case 4:
      for (int index = 0; index < table.length(); index++) calib_stream = calib_stream + formatMCAEStreamSize(CS_CALIB_BUFFER_SIZE, convertDecToHexUpper(QString::number(table[index]).toInt()));
      break;
    default:
      break;
    }

  return calib_stream;
}
/**
 * @brief MCAE::setCoinStream
 *
 * Configuración de la trama para el modo coincidencia
 *
 * @param function
 * @param data_one
 * @param data_two
 * @param time
 */
void MCAE::setCoinStream(string function, string data_one, string data_two, bool time)
{
  string stream;
  if (time)
    {
      stream = function + formatMCAEStreamSize(COIN_BUFFER_SIZE, convertDecToHex(QString::fromStdString(data_two).toInt())) + formatMCAEStreamSize(COIN_BUFFER_SIZE, convertToTwoComplement(QString::fromStdString(data_one).toInt()));
    }
  else
    {
      stream = function + data_one;
    }

  string cs_stream = formatMCAEStreamSize(CS_BUFFER_SIZE,convertDecToHex(getMCACheckSum(stream)));
  setTrama_Coin(getHead_Calib_Coin() + stream + cs_stream);
}
/**
 * @brief MCAE::setCalibStream
 *
 * Configuración de la trama de envío para calibración
 *
 * @param function
 * @param table
 */
void MCAE::setCalibStream(string function, QVector<double> table)
{
  string stream_pmts = getCalibTableFormat(function,table);
  string cs_stream = formatMCAEStreamSize(CS_CALIB_BUFFER_SIZE, convertDecToHexUpper(getMCACheckSum(function + stream_pmts)));
  setTrama_Calib(getHead_Calib_Coin()+function+stream_pmts+cs_stream);
}
/**
 * @brief MCAE::setMCAEStream
 *
 * Configuración de la trama general de envío para el protocolo MCAE
 *
 * Se configura la trama general de MCAE para el envío de MCA. Este método recibe como parámetros el valor de 'pmt',
 * el tamaño de la trama de recepción 'size_stream', la función MCA ('function') y en el caso que se realice la
 * confifuración de HV se debe incorporar el valor de HV ('channel_dec'), caso contrario dejar este campo en blanco.
 *
 * @param pmt_dec
 * @param size_stream
 * @param function
 * @param channel_dec (opcional)
 */
void MCAE::setMCAEStream(string pmt_dec, int size_stream, string function, string channel_dec)
{
  string channel_value;
  if (channel_dec.length()>=1) channel_value=getHVValueCode(atoi(channel_dec.c_str()));
  string pmt=getPMTCode(atoi(pmt_dec.c_str()));
  setMCAStream(pmt, function, channel_value);
  int size_mca=(int)(getTrama_MCA().size());
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_mca));
  string size_received=formatMCAEStreamSize(RECEIVED_BUFFER_SIZE,to_string(size_stream+size_mca));/** @todo:Verificar QString::number(size_stream+size_mca).toStdString()->to_string(size_stream+size_mca)*/
  string stream=getHeader_MCAE()+size_sended+size_received+getTrama_MCA();
  setTrama_MCAE(stream);
}
/**
 * @brief MCAE::setMCAEStream
 * @overload
 *
 * Configuración de la trama general de envío para el protocolo MCAE
 *
 * Se configura la trama general de MCAE para la configuración de calibración de Tiempos en el Cabezal. Este método
 * recibe como parámetros el valor de 'pmt_dec' (en decimal), la función MCA ('function') y el tiempo en double.
 * Este método se utiliza para la configuración de las tablas de Tiempos en el Cabezal.
 *
 * @param pmt_dec
 * @param function
 * @param time
 */
void MCAE::setMCAEStream(string pmt_dec, string function, double time)
{
  string pmt=getPMTCode(atoi(pmt_dec.c_str()));
  setMCAStream(pmt, function, time);
  int size_mca=(int)(getTrama_MCA().size());
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_mca));
  string size_received=formatMCAEStreamSize(RECEIVED_BUFFER_SIZE,to_string(size_mca));/** @todo:Verificar QString::number(size_mca).toStdString()->to_string(size_mca)*/
  string stream=getHeader_MCAE()+size_sended+size_received+getTrama_MCA();
  setTrama_MCAE(stream);
}
/**
 * @brief MCAE::setMCAEStream
 * @overload
 *
 * Configuración de la trama general de envío para el protocolo MCAE
 *
 * Se configura la trama general de MCAE para la configuración de las tablas de calibración. Este método recibe como
 * parámetros 'function' correspondiente a la función de calibración y 'table' que corresponde a la tabla con los
 * valores de calibración correspondiente.
 *
 * @param function
 * @param table
 */
void MCAE::setMCAEStream(string function, QVector<double> table)
{
  setCalibStream(function, table);
  int size_calib=(int)(getTrama_Calib().size());
  if (size_calib > 99) size_calib = 99;
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_calib));
  string size_received=getGeneric_Received_Size();
  string stream=getHeader_MCAE()+size_sended+size_received+getTrama_Calib();
  setTrama_MCAE(stream);
}
/**
 * @brief MCAE::setMCAEStream
 * @overload
 *
 * Configuración de la trama general de envío para el protocolo MCAE
 *
 * Se configura la trama general de MCAE para la configuración de los modos de autocoincidencia. Este método recibe
 * como parámetros la función de coincidencia ('function'), las tramas 'data_one' y 'data_two', y el valor
 * booleano 'time'. Cuando se define la ventana temporal (en _subclocks_) se utilizan las tramas 'data_one' y 'data_two'
 * (como el valor de ventana inferior y superior respectivamente), y la variable booleana 'time' se configura en _true_.
 * Para los otros modos solo se configura la trama 'data_one', la trama 'data_two' queda en blanco y la variable
 * booleana 'time' se configura en _false_.
 *
 * @param function
 * @param data_one
 * @param data_two
 * @param time
 */
void MCAE::setMCAEStream(string function, string data_one, string data_two, bool time)
{
  setCoinStream(function, data_one, data_two, time);
  int size_coin=(int)(getTrama_Coin().size());
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_coin));
  string size_received=formatMCAEStreamSize(RECEIVED_BUFFER_SIZE,to_string(size_coin));
  string stream = getHeader_MCAE()+size_sended+size_received+convertToMCAFormatStream(getTrama_Coin());
  setTrama_MCAE(stream);
}
/**
 * @brief MCAE::setPSOCEStream
 *
 * Configuración de la trama general de envío para el protocolo MCAE (para PSOC)
 *
 * @param function
 * @param psoc_value_dec
 */
void MCAE::setPSOCEStream(string function, string size_received, string psoc_value_dec)
{
  string psoc_value;
  if (psoc_value_dec.length()>=1) psoc_value=QString::number(round(QString::fromStdString(psoc_value_dec).toInt()/getPSOC_ADC())).toStdString();
  setPSOCStream(function, psoc_value);
  int size_psoc=(int)(getTrama_PSOC().size())+CRLF_SIZE;
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_psoc));
  string stream=getHeader_MCAE()+size_sended+size_received+getTrama_PSOC();
  setTrama_MCAE(stream);
}
/**
 * @brief MCAE::getHVValueCode
 *
 * Valor de HV en Hexadecimal con su formato
 *
 * @param hv_value_dec
 * @return hv_value_dec en Hexadecimal (lower)
 */
string MCAE::getHVValueCode(int hv_value_dec)
{
  string hv_value= convertDecToHex(hv_value_dec);
  hv_value=formatMCAEStreamSize(HV_BUFFER_SIZE,hv_value);

  return hv_value;
}
/**
 * @brief MCAE::formatMCAEStreamSize
 *
 * Formato de la trama para envío
 *
 * @param expected_size
 * @param data_stream
 * @return data_stream en formato de envío
 */
string MCAE::formatMCAEStreamSize(int expected_size, string data_stream)
{
  switch (expected_size) {
    case 2:
      if (data_stream.length()==1) data_stream="0" + data_stream;
      break;
    case 3:
      if (data_stream.length()==1) data_stream="00" + data_stream;
      if (data_stream.length()==2) data_stream="0" + data_stream;
      break;
    case 4:
      if (data_stream.length()==1) data_stream="000" + data_stream;
      if (data_stream.length()==2) data_stream="00" + data_stream;
      if (data_stream.length()==3) data_stream="0" + data_stream;
      break;
    default:
      break;
    }

  return data_stream;
}
/**
 * @brief MCAE::getPMTCode
 *
 * Valor de PMT en Hexadecimal con su formato
 *
 * @param pmt_dec
 * @return Valor de PMT
 */
string MCAE::getPMTCode(int pmt_dec)
{
  string pmt=formatMCAEStreamSize(PMT_BUFFER_SIZE,convertDecToHex(pmt_dec));

  return pmt;
}
/**
 * @brief MCAE::getPMTTemperature
 *
 * Analizador de la trama de temperatura
 *
 * @param temp_stream
 * @return Valor de temperatura en decimal
 */
double MCAE::getPMTTemperature(string temp_stream)
{
  QByteArray q_temp_stream(temp_stream.c_str(), temp_stream.length());
  /** string temp_stream_mca_format=convertFromMCAFormatStream(getReverse(q_temp_stream.mid(5,3)).toStdString()); @note : Cambio de lógica de envío de temperatura, verificar su funcionamiento. */
  string temp_stream_mca_format=convertFromMCAFormatStream(q_temp_stream.mid(5,3).toStdString());

  return convertHexToDec(temp_stream_mca_format)*DS1820_FACTOR;
}
/**
 * @brief MCAE::verifyCheckSum
 *
 * Analizador de _checksum_
 *
 * @param data_mca
 * @return _true_ si el _checksum_ es válido
 */
bool MCAE::verifyCheckSum(string data_mca)
{
  string data=convertFromMCAFormatStream(data_mca);
  QByteArray q_data(data.c_str(), data.length());
  QByteArray q_data_wo_cs=q_data.mid(1,data.length()-2);
  string data_cs=q_data.mid(data.length()-2,2).toStdString();
  string checksum_received=formatMCAEStreamSize(CS_BUFFER_SIZE, convertDecToHex(getMCACheckSum(q_data_wo_cs.toStdString())));
  bool checked=verifyStream(data_cs, checksum_received);

  return checked;
}
/**
 * @brief MCAE::verifyMCAEStream
 *
 * Analizador de la trama MCAE
 *
 * @param data_received
 * @param data_to_compare
 * @return _true_ si la trama es válida
 */
bool MCAE::verifyMCAEStream(string data_received, string data_to_compare)
{
  return verifyStream(data_received,data_to_compare);
}
/**
 * @brief MCAE::verifyStream
 *
 * Analizador de trama de datos
 *
 * @param data_received
 * @param data_to_compare
 * @return _true_ si la trama es válida
 */
bool MCAE::verifyStream(string data_received, string data_to_compare)
{
  bool checked = false;
  if (strcmp(data_received.c_str(),data_to_compare.c_str())==0) checked=true;

  return checked;
}
/**
 * @brief MCAE::parserPSOCStream
 *
 * Analiza la trama de respuesta de la placa PSOC, identificando si está encendida o no y el valor de tensión configurado
 *
 * @param stream
 * @return Vector _line_ con los campos de la trama identificados
 */
QVector<QString> MCAE::parserPSOCStream(string stream)
{
  QVector<QString> line;
  string delimiter_1 = ",";
  string delimiter_2 = "/";
  size_t pos = 0, pos_line=0;
  int vecIndex=0;
  string token;

  while ((pos = stream.find(delimiter_1)) != string::npos)
    {
      token = stream.substr(0, pos);
      line.append(QString::fromStdString(token));
      stream.erase(0, pos + delimiter_1.length());
      vecIndex++;
    }

  pos_line = line.at(2).toStdString().find(delimiter_2);
  token = line.at(2).toStdString().substr(0,pos_line);
  line.replace(2,QString::fromStdString(token));

  return line;
}
/**
 * @brief MCAE::getMCA
 *
 * Adquisición de MCA
 *
 * @param pmt
 * @param function
 * @param head
 * @param channels
 * @param port_name
 * @return Mensaje de recepción en _string_
 */
string MCAE::getMCA(string pmt, string function, string head, int channels, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + function);
  setMCAEStream(pmt, channels*6+16, getData_MCA());
  char delimeter='\r';
  string msg, msg_data;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);
  msg_data = readBufferString(channels*6+16, port_name);

  getMCASplitData(msg_data, channels);

  return msg;
}
/**
 * @brief MCAE::setHV
 *
 * Configuración de la tensión de dinodo del PMT
 *
 * @param head
 * @param pmt
 * @param channel_dec
 * @param port_name
 * @return Mensaje de recepción en _string_
 */
string MCAE::setHV(string head, string pmt, string channel_dec, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + getFunCSP3());
  setMCAEStream(pmt, 0, getSetHV_MCA(), channel_dec);
  char delimeter='\r';
  string msg;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);

  return msg;
}
/**
 * @brief MCAE::setCalibTable
 *
 * Configuración de las tablas de calibración
 *
 * @param head
 * @param calib_function
 * @param table
 * @param port_name
 * @return Mensaje de recepción en _string_
 */
string MCAE::setCalibTable(string head, string calib_function, QVector<double> table, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + getFunCHead());
  setMCAEStream(calib_function, table);
  char delimeter='\r';
  string msg;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);

  return msg;
}
/**
 * @brief MCAE::setTime
 *
 * Configura el tiempo relativo para cada PMT (obtenido en la calibración)
 *
 * @param head
 * @param time_value
 * @param pmt
 * @param port_name
 * @return Mensaje de recepción en _string_
 */
string MCAE::setTime(string head, double time_value, string pmt, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + getFunCSP3());
  setMCAEStream(pmt, getSet_Time_MCA(), time_value);
  char delimeter='\r';
  string msg;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);

  return msg;
}
/**
 * @brief MCAE::getTemp
 *
 * Método que obtiene la temperatura del PMT
 *
 * @param head
 * @param pmt
 * @param port_name
 * @return Mensaje de recepción en _string_
 */
string MCAE::getTemp(string head, string pmt, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + getFunCSP3());
  setMCAEStream(pmt, 0, getTemp_MCA());

  char delimeter='\r';
  string msg;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);

  return msg;
}
/**
 * @brief MCAE::parserRateStream
 *
 * Método que parsea la trama de recepción de la tasa de adquisición del cabezal
 *
 * @param stream
 * @return _rate_ de adquisición en _vector<int>_
 */
vector<int> MCAE::parserRateStream(string stream)
{
    QByteArray q_stream(stream.c_str(), stream.length());
    vector<int> rates(3);
    rates.at(0)=convertHexToDec(convertFromMCAFormatStream(q_stream.mid(5, 6).toStdString())); // Ventana baja
    rates.at(1)=convertHexToDec(convertFromMCAFormatStream(q_stream.mid(11, 6).toStdString())); // Ventana media
    rates.at(2)=convertHexToDec(convertFromMCAFormatStream(q_stream.mid(17, 6).toStdString())); // Ventana alta
    return rates;
}
/**
 * @brief MCAE::getRate
 *
 * Método que obtiene la tasa de adquisición del cabezal
 *
 * @param head
 * @param port_name
 * @return _rate_ de adquisición en _vector<int>_
 */
vector<int> MCAE::getRate(string head, string port_name)
{
  setHeader_MCAE(getHead_MCAE() + head + getFunCHead());
  string rate_stream = getMCAFormatStream(getRate_MCA());
  int size_rate=(int)(rate_stream.size());
  string size_sended=formatMCAEStreamSize(SENDED_BUFFER_SIZE,to_string(size_rate));
  string size_received=formatMCAEStreamSize(RECEIVED_BUFFER_SIZE,to_string(size_rate+RECEIVED_RATE_BUFFER_SIZE));
  string stream = getHeader_MCAE() + size_sended + size_received + rate_stream;
  setTrama_MCAE(stream);

  char delimeter='\r';
  string msg;

  sendString(getTrama_MCAE(), getEnd_MCA(), port_name);
  msg = readString(delimeter, port_name);

  return parserRateStream(msg);
}
