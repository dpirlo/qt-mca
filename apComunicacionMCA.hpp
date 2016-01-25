#ifndef COMUNICACIONMCA_H
#define COMUNICACIONMCA_H

#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

using namespace std;
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
        size_t portWrite(string *msg);
        size_t portRead(string *msg, int buffer_size);
        bool isPortOpen();
        ~ComunicacionMCA();

    private:
        io_service io;
        serial_port_ptr port;
    };

}

#endif // COMUNICACIONMCA_H
