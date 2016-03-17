#include "apMCAE.hpp"

using namespace ap;

MCAE::MCAE(size_t timeout)
    :port(serial_port_ptr(new serial_port(io))),
     timeout(timeout),
     read_error(true),
     timer(port->get_io_service()),
     PortBaudRate(921600),
     /*Funciones trama MCAE*/
     FunCHead("01"),
     FunCSP3("02"),
     FunCHV("03"),
     Head_MCAE("#C"),
     End_MCA("\r"),
     End_HV("\r\n"),
     HV_OFF("$SET,STA,OFF"),
     HV_ON("$SET,STA,ON")
{

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

void MCAE::ReadComplete(const boost::system::error_code& error,
                        size_t bytes_transferred)
{
    read_error = (error || bytes_transferred == 0);
    timer.cancel();
}


void MCAE::TimeOut(const boost::system::error_code& error)
{
    if (error) { return; }
    port->cancel();
}


bool MCAE::ReadOneChar(char& val)
{
   char c;
   val = c = '\0';

   port->get_io_service().reset();
   port->async_read_some(boost::asio::buffer(&c, 1),
                                          boost::bind(&MCAE::ReadComplete, this,
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred));
   timer.expires_from_now(boost::posix_time::milliseconds(timeout));
   timer.async_wait(boost::bind(&MCAE::TimeOut,
                    this, boost::asio::placeholders::error));

   port->get_io_service().run();

   if (!read_error)
       val = c;

   return !read_error;
}

void MCAE::ReadString(string *msg, char delimeter)
{
    char c;
    while (ReadOneChar(c) && c != delimeter) {
        msg->push_back(c);
        }

    if (c != delimeter) {
        Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
        throw exception_timeout;
    }
}


