#include <gry/source.h>
#include <boost/thread/lock_guard.hpp>

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

Source::Timestamp Source::add ( double _value )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_valuesLock );
    Timestamp now = TimeSource::now();

    m_values.push_back ( Value(now, _value) );

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
