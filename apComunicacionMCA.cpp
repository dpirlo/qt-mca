#include "apComunicacionMCA.hpp"

using namespace ap;

ComunicacionMCA::ComunicacionMCA()
{

}

ComunicacionMCA::~ComunicacionMCA()
{

}

int ComunicacionMCA::portConnect(const char *tty_port_name, int baud_rate)
{
    port=serial_port_ptr(new serial_port(io));
    port->open(tty_port_name);
    port->set_option(serial_port_base::baud_rate(baud_rate));
    return 0;
}

int ComunicacionMCA::portDisconnect()
{
    if (port->is_open())
        port->close();
    return 0;
}


bool ComunicacionMCA::portWrite(unsigned char msg)
{
    port->write_some(buffer(&msg,1));
    return true;
}

unsigned char ComunicacionMCA::portRead()
{
    unsigned char msg;
    port->read_some(buffer(&msg,1));
    return msg;
}
