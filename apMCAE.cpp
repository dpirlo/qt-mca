#include "apMCAE.hpp"

using namespace ap;

MCAE::MCAE(size_t timeout)
    :port(serial_port_ptr(new serial_port(io))),
     channels_id(CHANNELS),
     hits_mca(CHANNELS),
     timeout(timeout),
     read_error(true),
     timer(port->get_io_service()),     
     /*Funciones trama MCAE*/
     FunCHead("01"),
     FunCSP3("02"),
     FunCHV("03"),
     Head_MCAE("#C"),
     Head_MCA("@"),
     End_MCA("\r"),
     End_HV("\r\n"),
     HV_OFF("$SET,STA,OFF"),
     HV_ON("$SET,STA,ON"),
     PortBaudRate(921600),
     Init_MCA("64"),
     Data_MCA("65"),
     SetHV_MCA("68")
{
    /*PRUEBAS*/
}

MCAE::~MCAE()
{
   portDisconnect();
}

bool MCAE::isPortOpen()
{
    return port->is_open();
}

error_code MCAE::portConnect(const char *tty_port_name)
{
    error_code error_code;    
    port->open(tty_port_name, error_code);
    port->set_option(serial_port_base::baud_rate(PortBaudRate));

    return error_code;
}

error_code MCAE::portDisconnect()
{
    error_code error_code;
    if (port->is_open())
        port->close(error_code);

    return error_code;
}

size_t MCAE::portWrite(string *msg)
{
    char c_msg[msg->size()+1];
    strcpy(c_msg, msg->c_str());
    size_t bytes_transferred = port->write_some(boost::asio::buffer(c_msg,msg->size()));

    return bytes_transferred;
}

size_t MCAE::portRead(string *msg, int buffer_size)
{
    char c_msg[buffer_size];
    size_t bytes_transferred = port->read_some(boost::asio::buffer(c_msg,buffer_size));
    msg->assign(c_msg);

    return bytes_transferred;
}


size_t MCAE::portRead(char *c_msg)
{
    int buffer_size=1;
    size_t bytes_transferred = port->read_some(boost::asio::buffer(c_msg,buffer_size));

    return bytes_transferred;
}


string MCAE::portReadMCAELine() {
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

string MCAE::portReadPSOCLine() {
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

void MCAE::portReadComplete(const boost::system::error_code& error,
                        size_t bytes_transferred)
{
    read_error = (error || bytes_transferred == 0);
    timer.cancel();
}


void MCAE::portTimeOut(const boost::system::error_code& error)
{
    if (error) { return; }
    port->cancel();
}


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

void MCAE::portReadString(string *msg, char delimeter)
{
    char c;
    while (portReadOneChar(c) && c != delimeter) {
        msg->push_back(c);
        }

    if (c != delimeter) {
        Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
        throw exception_timeout;
    }
}

void MCAE::portReadBufferString(string *msg, int buffer_size)
{
    char c;
    int buffer=1;
    while (portReadOneChar(c) && buffer <= buffer_size) {
        msg->push_back(c);
        buffer++;
        }

    if (buffer <= buffer_size) {
        Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
        throw exception_timeout;
    }
}

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

error_code MCAE::portFlush()
{
    error_code ec;    

    const bool isFlushed =! ::tcflush(port->native(), TCIOFLUSH);
    if (!isFlushed)
        ec = error_code(errno,error::get_system_category());

    return ec;
}

int MCAE::convertHexToDec(string hex_number_s)
{
    bool ok;
    QString hex_number = QString::fromStdString(hex_number_s);
    int dec_number = hex_number.toInt(&ok,16);

    return dec_number;
}

string MCAE::convertDecToHex(int dec_number)
{
    QByteArray hex_number = QByteArray::number(dec_number,16);

    return QString(hex_number).toStdString();
}

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

void MCAE::getMCAHitsData(QByteArray data_mca)
{
    int channel;
    long hits;

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

int MCAE::getMCACheckSum(string data_function, string data_channel, string data_pmt, int data_length)
{
    int sum_of_elements = 0;
    for(unsigned int i = 0; i < data_function.length(); ++i)
    {
        string token(1, data_function.at(i));
        sum_of_elements = sum_of_elements + atoi(token.c_str());
    }

    if(data_length > 5)
    {
        string channel_1(1,data_channel.at(0));
        string channel_2(1,data_channel.at(1));
        string channel_3(1,data_channel.at(2));
        sum_of_elements = sum_of_elements + convertHexToDec(channel_1) + convertHexToDec(channel_2) + convertHexToDec(channel_3);
    }

    string pmt_1(1,data_pmt.at(0));
    string pmt_2(1,data_pmt.at(1));

    sum_of_elements = sum_of_elements + convertHexToDec(pmt_1) + convertHexToDec(pmt_2);

    return sum_of_elements;
}

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

string MCAE::convertMCAFormatStream(string data_with_cs)
{
    /* Formato de data con checksum:
     * @ddcc--...--ss
     */

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


string MCAE::getMCAFormatStream(string data)
{
    /* Formato de data sin checksum:
     * @ddcc--...--
     */

    string data_channel;
    string data_pmt=data.substr(1,2);
    string data_function=data.substr(3,2);
    if(data.length()>5) data_channel = data.substr(5,data.length());    
    string checksum=convertDecToHex(getMCACheckSum(data_function,data_channel,data_pmt,data.length()));
    if (checksum.length()==1) checksum="0" + checksum;
    string data_plus_checksum = data_pmt + data_function + data_channel + checksum;
    data_plus_checksum = Head_MCA + data_plus_checksum;
    string data_plus_checksum_mca_format=convertMCAFormatStream(data_plus_checksum);

    return data_plus_checksum_mca_format;
}

void MCAE::setMCAStream(string pmt, string function, string channel)
{
    string stream_wo_cs="@"+pmt+function+channel;
    setTrama_MCA(getMCAFormatStream(stream_wo_cs));
}

void MCAE::setMCAEStream(string pmt_dec, int size_stream, string function, string channel_dec)
{
    string channel;
    if (channel_dec.length()>=1) channel=getChannelCode(atoi(channel_dec.c_str()));
    string pmt=getPMTCode(atoi(pmt_dec.c_str()));
    setMCAStream(pmt, function, channel);
    int size_mca=(int)(getTrama_MCA().size());
    string size_sended=to_string(size_mca);
    if (size_sended.length()==1) size_sended="0" + size_sended;
    string size_received=QString::number(size_stream+size_mca).toStdString();
    string stream=getHeader_MCAE()+size_sended+size_received+getTrama_MCA();
    setTrama_MCAE(stream);
}

string MCAE::getChannelCode(int channel_dec)
{
    string channel= convertDecToHex(channel_dec);
    if (channel.length()==1) channel="00" + channel;
    if (channel.length()==2) channel="0" + channel;

    return channel;
}

string MCAE::getPMTCode(int pmt_dec)
{
    string pmt=convertDecToHex(pmt_dec);
    if (pmt.length()==1) pmt="0" + pmt;

    return pmt;
}
