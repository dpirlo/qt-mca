#ifndef COMUNICACIONMCA_H
#define COMUNICACIONMCA_H

#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::system;

#define SERIAL_PORT_READ_BUF_SIZE 1

typedef shared_ptr<serial_port> serial_port_ptr;

namespace ap {

    class ComunicacionMCA
    {
    public:
        ComunicacionMCA();
        error_code portConnect(const char *tty_port_name, int baud_rate);
        error_code portDisconnect();
        size_t portWrite(unsigned char msg, int buffer_size);
        size_t portRead(unsigned char *msg);
        bool isPortOpen();
        ~ComunicacionMCA();

    private:
        io_service io;
        serial_port_ptr port;
    };

}

#endif // COMUNICACIONMCA_H
