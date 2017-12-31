#include <gry/webvaluebuffer.h>
#include <pion/tcp/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace gry;

WebValueBuffer::WebValueBuffer ( size_t _numberOfValues, Duration _valueDuration )
    : ValueBuffer(_numberOfValues, _valueDuration)
{
}

void operator<< ( pion::tcp::stream_buffer & _stream, const std::string & _value )
{
    for ( std::string::size_type i = 0; i < _value.length(); ++i )
    {
        _stream.sputc ( _value[i] );
    }
}

int WebValueBuffer::add ( Timestamp _timestamp, double _value )
{
    int numberAdded = ValueBuffer::add ( _timestamp, _value );
    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_listenersLock );
        std::string valueAsString ( boost::lexical_cast<std::string>(_value) );
        std::string numberAsString ( boost::lexical_cast<std::string>(numberAdded) );

        for ( std::vector<pion::tcp::connection_ptr>::iterator it = m_listeners.begin();
              it != m_listeners.end(); )
        {
            pion::tcp::stream_buffer stream ( *it );
            stream << "data: [";
            stream << valueAsString;
            stream << ", ";
            stream << numberAsString;
            stream << "]\n\n";

            if ( (*it)->is_open() )
            {
                ++it;
            }
            else
            {
                it = m_listeners.erase(it);
            }
        }
    }
    return numberAdded;
}

void WebValueBuffer::subscribe ( const pion::tcp::connection_ptr & _conn )
{
    _conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_KEEPALIVE);
    pion::tcp::stream_buffer stream ( _conn );
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-Type: text/event-stream\r\n";
    stream << "Connection: keep-alive\r\n";
    stream << "\r\n";
    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_listenersLock );

        m_listeners.push_back ( _conn );
    }
}

WebValueBuffer::~WebValueBuffer()
{
}
