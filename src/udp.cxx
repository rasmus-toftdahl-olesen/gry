#include <gry/udp.h>
#include <gry/repository.h>

using namespace gry;
using namespace boost::asio::ip;

UdpServer::UdpServer( pion::scheduler & _scheduler, ushort _portNumber )
    : m_logger ( log4cpp::Category::getInstance("gry.udp") ),
      m_socket(_scheduler.get_io_service(), udp::endpoint(udp::v4(), _portNumber))
{
}

void UdpServer::start()
{
    //m_logger << log4cpp::Priority::DEBUG << "Starting UDP listener on "; // << m_socket.endpoint().portNumber();
    m_socket.async_receive_from( boost::asio::buffer(m_buffer),
                                 m_client,
                                 boost::bind(&UdpServer::handleReceive, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
}

void UdpServer::handleReceive( const boost::system::error_code& error,
                               std::size_t _bytesTransfered )
{
    if (!error )
    {
        std::string input;
        std::copy( m_buffer.begin(), m_buffer.begin() + _bytesTransfered, std::back_inserter(input) );

        if (input.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_ .-\r\n") != std::string::npos)
        {
            m_logger << log4cpp::Priority::WARN << "Ignoring bad input data (" << input.length() << " characters): non-ascii data detected.";
        }
        else
        {
            m_logger << log4cpp::Priority::DEBUG << "Received data: " << input;
            std::string::size_type index = input.find(' ');
            if ( index != std::string::npos )
            {
                std::string::size_type endOfValue = input.find_first_not_of("0123456789.-", index + 1);
                if ( endOfValue == std::string::npos )
                {
                    endOfValue = input.length();
                }
                Repository & repo = Repository::instance();
                std::string sourceName ( input.substr(0, index) );
                std::string valueString ( input.substr(index + 1, endOfValue - index - 1) );
                m_logger << log4cpp::Priority::DEBUG << "Source name = '" << sourceName
                         << "', value = '" << valueString << "'";

                double value = boost::lexical_cast<double> ( valueString );
                SourcePtr source = repo.getOrCreateSourceByName(sourceName);
                Source::Timestamp timestamp = source->add ( value );

                m_logger << log4cpp::Priority::INFO << "Added data for " << source->name()
                         << " " << timestamp << " " << value;
            }
            else
            {
                m_logger << log4cpp::Priority::WARN << "Badly formatted message: " << input;
            }
        }
    }
    else
    {
        m_logger << log4cpp::Priority::ERROR << "Error receiving UDP data: " << error;
    }

    start();
}
