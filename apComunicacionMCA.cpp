#include "apComunicacionMCA.hpp"

using namespace ap;

ComunicacionMCA::ComunicacionMCA()
{

}

ComunicacionMCA::~ComunicacionMCA()
{
   portDisconnect();
}

error_code ComunicacionMCA::portConnect(const char *tty_port_name, int baud_rate)
{
    error_code error_code;
    port=serial_port_ptr(new serial_port(io));
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

size_t ComunicacionMCA::portWrite(unsigned char msg, int buffer_size)
{
    return port->write_some(buffer(&msg,buffer_size));
}

size_t ComunicacionMCA::portRead(unsigned char *msg)
{
    return port->read_some(buffer(&msg,1));
}

bool ComunicacionMCA::isPortOpen()
{
    return port->is_open();
}
