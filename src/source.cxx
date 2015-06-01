#include <gry/source.h>
#include <gry/utils.h>
#include <boost/thread/lock_guard.hpp>
#include <pion/tcp/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace gry;
using namespace boost::filesystem;
using namespace boost::chrono;

Source::Source(const path & _directory)
    : LIVE_VALUES(1000),
      m_directory(_directory),
      m_liveValues(LIVE_VALUES),
      m_bySecond(DEFAULT_BY_SECOND_VALUES, boost::chrono::seconds(1)),
      m_byMinute(DEFAULT_BY_MINUTE_VALUES, boost::chrono::minutes(1)),
      m_byHour(DEFAULT_BY_HOUR_VALUES, boost::chrono::hours(1)),
      m_byDay(DEFAULT_BY_DAY_VALUES, boost::chrono::hours(24))
{
    assert ( !is_regular_file(_directory) );

    m_bySecond.setNext ( &m_byMinute );
    m_byMinute.setNext ( &m_byHour );
    m_byHour.setNext ( &m_byDay );

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
        m_name = read_text_file ( namePath );
    }
    else
    {
        m_name = m_directory.filename().native();
    }
}

void Source::setName ( const std::string & _name )
{
    m_name = _name;

    write_text_file ( m_directory / "name.txt", _name );
}

int Source::numberOfBySecondValues()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_bySecond.size();
}

int Source::numberOfByMinuteValues()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_byMinute.size();
}

int Source::numberOfByHourValues()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_byHour.size();
}

int Source::numberOfByDayValues()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_byDay.size();
}

int Source::numberOfSamples()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_liveValues.size();
}

Source::Value Source::oldest()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    if ( m_liveValues.empty() )
    {
        return Source::Value(TimeSource::now(), NAN);
    }
    else
    {
        return m_liveValues.front();
    }
}

Source::Value Source::newest()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    if ( m_liveValues.empty() )
    {
        return Source::Value(TimeSource::now(), NAN);
    }
    else
    {
        return m_liveValues.back();
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
    static boost::chrono::seconds one_second (1);
    static boost::chrono::minutes one_minute (1);

    Timestamp now = TimeSource::now();
    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

        m_liveValues.push_back ( Value(now, _value) );
        m_bySecond.add ( now, _value );
    }

    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_listenersLock );

        for ( std::vector<pion::tcp::connection_ptr>::iterator it = m_listeners.begin(); it != m_listeners.end(); )
        {
            std::string valueAsString ( boost::lexical_cast<std::string>(_value) );
            pion::tcp::stream_buffer stream ( *it );
            stream << "data: ";
            stream << valueAsString;
            stream << "\n\n";

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
    return now;
}

void Source::writeValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    bool first = true;
    _writer->write ( "[\n" );
    for ( boost::circular_buffer<Source::Value>::const_iterator it = m_liveValues.begin();
          it != m_liveValues.end();
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

    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_listenersLock );

        m_listeners.push_back ( _conn );
    }
}
