#include "apMCAE.hpp"

using namespace ap;

MCAE::MCAE()
{
   portInit();
   setStrings();
}

MCAE::~MCAE()
{
   portDisconnect();
}

bool MCAE::isPortOpen()
{
    return port->is_open();
}

void MCAE::setStrings()
{
   /*Funciones trama MCAE*/
   FunCHead="01";
   FunCSP3="02";
   FunCHV="03";
   Head_MCAE="#C";
   End_MCA="\r";
   End_HV="\r\n";
   HV_OFF="$SET,STA,OFF";
   HV_ON="$SET,STA,ON";
}

void MCAE::portInit()
{
    port=serial_port_ptr(new serial_port(io));
}

error_code MCAE::portConnect(const char *tty_port_name, int baud_rate)
{
    error_code error_code;    
    port->open(tty_port_name, error_code);
    port->set_option(serial_port_base::baud_rate(baud_rate));

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


