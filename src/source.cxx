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
            loadLastValueAt ( "by_second.last_value_at", m_bySecond );
            loadValues ( "by_minute.mem", m_byMinute );
            loadLastValueAt ( "by_minute.last_value_at", m_byMinute );
            loadValues ( "by_hour.mem", m_byHour );
            loadLastValueAt ( "by_hour.last_value_at", m_byHour );
            loadValues ( "by_day.mem", m_byDay );
            loadLastValueAt ( "by_day.last_value_at", m_byDay );
        }
        else
        {
            saveValues ( "by_second.mem", m_bySecond );
            saveLastValueAt ( "by_second.last_value_at", m_bySecond );
            saveValues ( "by_minute.mem", m_byMinute );
            saveLastValueAt ( "by_minute.last_value_at", m_byMinute );
            saveValues ( "by_hour.mem", m_byHour );
            saveLastValueAt ( "by_hour.last_value_at", m_byHour );
            saveValues ( "by_day.mem", m_byDay );
            saveLastValueAt ( "by_day.last_value_at", m_byDay );
        }

        path lastValuePath = m_directory / "name.txt";
        if ( exists(namePath) )
        {
            m_name = read_text_file ( namePath );
        }
        else
        {
            m_name = m_directory.filename().native();
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

void Source::loadLastValueAt ( const std::string & _filename, ValueBuffer & _values ) const
{
    path fullPath = m_directory / _filename;
    if ( exists(fullPath) )
    {
        std::ifstream fp ( fullPath.native().c_str() );
        Timestamp lastValueAt;
        fp >> lastValueAt;

        _values.setLastValueAt(lastValueAt);
    }
}

void Source::saveLastValueAt ( const std::string & _filename, const ValueBuffer & _values ) const
{
    path fullPath = m_directory / _filename;
    std::ofstream fp ( fullPath.native().c_str() );
    fp << _values.lastValueAt();
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

Source::Duration Source::timeSinceLastValue()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

    return m_bySecond.timeSinceLastValue();
}

Source::Timestamp Source::add ( double _value )
{
    static boost::chrono::seconds one_second (1);
    static boost::chrono::minutes one_minute (1);

    Timestamp now = TimeSource::now();
    {
        boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );

        m_bySecond.add ( now, _value );
    }
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
        if ( isnan(*it) )
        {
            _writer->write ( "null" );
        }
        else
        {
            _writer->write ( *it );
        }
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


void Source::subscribeSecond ( pion::tcp::connection_ptr & _conn )
{
    m_bySecond.subscribe ( _conn );
}

void Source::subscribeMinute ( pion::tcp::connection_ptr & _conn )
{
    m_byMinute.subscribe ( _conn );
}

void Source::subscribeHour ( pion::tcp::connection_ptr & _conn )
{
    m_byHour.subscribe ( _conn );
}

void Source::subscribeDay ( pion::tcp::connection_ptr & _conn )
{
    m_byDay.subscribe ( _conn );
}
