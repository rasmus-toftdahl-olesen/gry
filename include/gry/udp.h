#include <boost/asio/ip/udp.hpp>
#include <pion/scheduler.hpp>
#include <log4cpp/Category.hh>

namespace gry
{
    class UdpServer
    {
    private:

        log4cpp::Category & m_logger;
        boost::asio::ip::udp::socket m_socket;
        boost::array<char, 1024> m_buffer;
        boost::asio::ip::udp::endpoint m_client;

        void handleReceive( const boost::system::error_code& error,
                            std::size_t _bytesTransfered );

    public:
        UdpServer ( pion::scheduler & _scheduler, ushort _portNumber );
        void start();
    };
}
