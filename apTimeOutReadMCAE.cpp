#include "apTimeOutReadMCAE.hpp"


using namespace ap;

TimeOutReadMCAE::TimeOutReadMCAE(serial_port_ptr port_ptr,  size_t timeout) :
            timeout(timeout),
            port_read(port_ptr),
            timer(port_read->get_io_service()),
            read_error(true)
{

}

TimeOutReadMCAE::~TimeOutReadMCAE()
{

}

void TimeOutReadMCAE::ReadComplete(const boost::system::error_code& error,
                        size_t bytes_transferred)
{
    read_error = (error || bytes_transferred == 0);    
    timer.cancel();
}


void TimeOutReadMCAE::TimeOut(const boost::system::error_code& error)
{
    if (error) { return; }
    port_read->cancel();    
}


bool TimeOutReadMCAE::ReadOneChar(char& val)
{
   char c;
   val = c = '\0';

   port_read->get_io_service().reset();
   port_read->async_read_some(boost::asio::buffer(&c, 1),
                                          boost::bind(&TimeOutReadMCAE::ReadComplete, this,
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred));
   timer.expires_from_now(boost::posix_time::milliseconds(timeout));   
   timer.async_wait(boost::bind(&TimeOutReadMCAE::TimeOut,
                    this, boost::asio::placeholders::error));

   port_read->get_io_service().run();

   if (!read_error)
       val = c;

   return !read_error;
}

void TimeOutReadMCAE::ReadString(string *msg)
{

    char c;
    while (ReadOneChar(c) && c != '\r') {
        msg->push_back(c);
        }

    if (c != '\r') {
        Exceptions exception_timeout("Error de tiempo de lectura. TimeOut!");
        throw exception_timeout;
    }
}
