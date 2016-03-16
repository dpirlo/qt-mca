#ifndef APTIMEOUTREADMCAE_H
#define APTIMEOUTREADMCAE_H
#include "apMCAE.hpp"


namespace ap {

    class TimeOutReadMCAE : public MCAE
    {
    public:
        TimeOutReadMCAE(serial_port_ptr port_ptr, size_t timeout);
        void ReadComplete(const boost::system::error_code& error, size_t bytes_transferred);
        void TimeOut(const boost::system::error_code& error);
        bool ReadOneChar(char& val);
        void ReadString(string *msg);
        ~TimeOutReadMCAE();


    public:        
        size_t timeout;
        serial_port_ptr port_read;        
        deadline_timer timer;
        bool read_error;    
    };
}

#endif // APTIMEOUTREADMCAE_H
