#include <gry/source.h>
#include <boost/thread/lock_guard.hpp>
#include <pion/tcp/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace gry;
using namespace boost::filesystem;

Source::Source(const path & _directory)
    : m_directory(_directory),
      m_values(1000)
{
    assert ( !is_regular_file(_directory) );

    refresh();
}

void Source::refresh()
{
    if ( !exists(m_directory) )
    {
        create_directory(m_directory);
    }

    path namePath = m_directory / "name.txt";
    if ( exists(namePath) )
    {

    }
    else
    {
        m_name = m_directory.filename().native();
    }
}

int Source::numberOfSamples()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_values.size();
}

Source::Value Source::oldest()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    if ( m_values.empty() )
    {
        return Source::Value(TimeSource::now(), NAN);
    }
    else
    {
        return m_values.front();
    }
}

Source::Value Source::newest()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    if ( m_values.empty() )
    {
        return Source::Value(TimeSource::now(), NAN);
    }
    else
    {
        return m_values.back();
    }
}

void operator<< ( pion::tcp::stream_buffer & _stream, const std::string & _value )
{
    for ( std::string::size_type i = 0; i < _value.length(); ++i )
    {
        _stream.sputc ( _value[i] );
    }
}

Source::Timestamp Source::add ( double _value )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );
    Timestamp now = TimeSource::now();

    m_values.push_back ( Value(now, _value) );

    for ( std::vector<pion::tcp::connection_ptr>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it )
    {
        std::string valueAsString ( boost::lexical_cast<std::string>(_value) );
        pion::tcp::stream_buffer stream ( *it );
        stream << "data: ";
        stream << valueAsString;
        stream << "\n\n";
    }
    return now;
}

void Source::writeValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    bool first = true;
    _writer->write ( "[\n" );
    for ( boost::circular_buffer<Source::Value>::const_iterator it = m_values.begin();
          it != m_values.end();
          ++it )
    {
        if ( first )
        {
            first = false;
        }
        else
        {
            _writer->write ( ", " );
        }
        _writer->write ( " { timestamp: '" );
        _writer->write ( it->get<0>() );
        _writer->write ( "', value: " );
        _writer->write ( it->get<1>() );
        _writer->write ( " }\n" );
    }
    _writer->write ( "]\n" );
}


void Source::subscribe ( pion::tcp::connection_ptr & _conn )
{
    _conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_KEEPALIVE);
    pion::tcp::stream_buffer stream ( _conn );
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-Type: text/event-stream\r\n";
    stream << "Connection: keep-alive\r\n";
    stream << "\r\n";

    m_listeners.push_back ( _conn );
}
