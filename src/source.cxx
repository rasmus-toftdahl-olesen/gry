#include <gry/source.h>
#include <gry/utils.h>
#include <boost/thread/locks.hpp>
#include <pion/tcp/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace gry;
using namespace boost::filesystem;
using namespace boost::chrono;

Source::Source(const path & _directory)
    : m_directory(_directory),
      m_bySecond(DEFAULT_BY_SECOND_VALUES, boost::chrono::seconds(1)),
      m_byMinute(DEFAULT_BY_MINUTE_VALUES, boost::chrono::minutes(1)),
      m_byHour(DEFAULT_BY_HOUR_VALUES, boost::chrono::hours(1)),
      m_byDay(DEFAULT_BY_DAY_VALUES, boost::chrono::hours(24))
{
    assert ( !is_regular_file(_directory) );

    m_bySecond.setNext ( &m_byMinute );
    m_byMinute.setNext ( &m_byHour );
    m_byHour.setNext ( &m_byDay );

    refresh( true );
}

void Source::refresh( bool _initial )
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

    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

        if ( _initial )
        {
            loadValues ( "by_second.mem", m_bySecond );
            loadValues ( "by_minute.mem", m_byMinute );
            loadValues ( "by_hour.mem", m_byHour );
            loadValues ( "by_day.mem", m_byDay );
        }
        else
        {
            saveValues ( "by_second.mem", m_bySecond );
            saveValues ( "by_minute.mem", m_byMinute );
            saveValues ( "by_hour.mem", m_byHour );
            saveValues ( "by_day.mem", m_byDay );
        }
    }
}

void Source::loadValues ( const std::string & _filename, ValueBuffer & _values ) const
{
    path fullPath = m_directory / _filename;
    if ( exists(fullPath) )
    {
        std::ifstream fp ( fullPath.native().c_str(), std::ifstream::binary );
        _values.load ( fp );
    }
}

void Source::saveValues ( const std::string & _filename, const ValueBuffer & _values ) const
{
    path fullPath = m_directory / _filename;
    std::ofstream fp ( fullPath.native().c_str(), std::ofstream::binary );
    _values.save ( fp );
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

void operator<< ( pion::tcp::stream_buffer & _stream, const std::string & _value )
{
    for ( std::string::size_type i = 0; i < _value.length(); ++i )
    {
        _stream.sputc ( _value[i] );
    }
}

void notifyListeners ( boost::recursive_mutex & _listenersLock, std::vector<pion::tcp::connection_ptr> & _listeners, double _value, int _number )
{
    boost::lock_guard<boost::recursive_mutex> guard ( _listenersLock );

    for ( std::vector<pion::tcp::connection_ptr>::iterator it = _listeners.begin(); it != _listeners.end(); )
    {
        std::string valueAsString ( boost::lexical_cast<std::string>(_value) );
        std::string numberAsString ( boost::lexical_cast<std::string>(_number) );
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
            it = _listeners.erase(it);
        }
    }
}

Source::Timestamp Source::add ( double _value )
{
    static boost::chrono::seconds one_second (1);
    static boost::chrono::minutes one_minute (1);

    int added = 0;
    Timestamp now = TimeSource::now();
    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

        added = m_bySecond.add ( now, _value );
    }

    notifyListeners(m_listenersLock, m_secondListeners, _value, added);

    return now;
}

void writeValues ( pion::http::response_writer_ptr _writer, const ValueBuffer & _buffer )
{
    bool first = true;
    _writer->write ( "[" );
    for ( ValueBuffer::ConstIterator it = _buffer.begin(); it != _buffer.end(); ++it )
    {
        if ( first )
        {
            first = false;
        }
        else
        {
            _writer->write ( "," );
        }
        _writer->write ( *it );
    }
    _writer->write ( "]\n" );
}

void Source::writeBySecondValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    writeValues ( _writer, m_bySecond );
}

void Source::writeByMinuteValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    writeValues ( _writer, m_byMinute );
}

void Source::writeByHourValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    writeValues ( _writer, m_byHour );
}

void Source::writeByDayValues ( pion::http::response_writer_ptr _writer )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    writeValues ( _writer, m_byDay );
}


void subscribe ( pion::tcp::connection_ptr & _conn, boost::recursive_mutex & _listenersLock, std::vector<pion::tcp::connection_ptr> & _listeners )
{
    _conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_KEEPALIVE);
    pion::tcp::stream_buffer stream ( _conn );
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-Type: text/event-stream\r\n";
    stream << "Connection: keep-alive\r\n";
    stream << "\r\n";
    {
        boost::lock_guard<boost::recursive_mutex> guard ( _listenersLock );

        _listeners.push_back ( _conn );
    }
}

void Source::subscribeSecond ( pion::tcp::connection_ptr & _conn )
{
    subscribe(_conn, m_listenersLock, m_secondListeners );
}

void Source::subscribeMinute ( pion::tcp::connection_ptr & _conn )
{
    subscribe(_conn, m_listenersLock, m_minuteListeners );
}

void Source::subscribeHour ( pion::tcp::connection_ptr & _conn )
{
    subscribe(_conn, m_listenersLock, m_hourListeners );
}

void Source::subscribeDay ( pion::tcp::connection_ptr & _conn )
{
    subscribe(_conn, m_listenersLock, m_dayListeners );
}
