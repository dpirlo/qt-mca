#include "apComunicacionMCA.hpp"

using namespace ap;

ComunicacionMCA::ComunicacionMCA()
{
   portInit();
}

ComunicacionMCA::~ComunicacionMCA()
{
   portDisconnect();
}

void ComunicacionMCA::portInit()
{
    port=serial_port_ptr(new serial_port(io));
}

error_code ComunicacionMCA::portConnect(const char *tty_port_name, int baud_rate)
{
    error_code error_code;    
    port->open(tty_port_name, error_code);
    port->set_option(serial_port_base::baud_rate(baud_rate));
    return error_code;
}

error_code ComunicacionMCA::portDisconnect()
{
    error_code error_code;
    if (port->is_open())
        port->close(error_code);

    return error_code;
}

size_t ComunicacionMCA::portWrite(string *msg)
{
    char c_msg[msg->size()+1];
    strcpy(c_msg, msg->c_str());
    size_t bytes_transferred = port->write_some(boost::asio::buffer(c_msg,msg->size()));
    return bytes_transferred;
}

size_t ComunicacionMCA::portRead(string *msg, int buffer_size)
{
    char c_msg[buffer_size];
    size_t bytes_transferred = port->read_some(boost::asio::buffer(c_msg,buffer_size));
    msg->assign(c_msg);

    return bytes_transferred;
}

bool ComunicacionMCA::isPortOpen()
{
    return port->is_open();
}
