#include <gry/valuebuffer.h>
#include <boost/throw_exception.hpp>

using namespace gry;

ValueBuffer::ValueBuffer ( size_t _numberOfValues, Duration _valueDuration )
    : m_valueDuration(_valueDuration),
      m_lastValue(Timestamp::min()),
      m_values(_numberOfValues),
      m_next(0)
{
    for ( size_t i = 0; i < _numberOfValues; i++ )
    {
        m_values.push_back(0);
    }
}

void ValueBuffer::setNext ( ValueBuffer * _nextBuffer )
{
    //size_t neededSamples = _nextBuffer->m_valueDuration / m_valueDuration;

    m_next = _nextBuffer;
}

void ValueBuffer::add ( Timestamp _timestamp, double _value )
{
    if ( _timestamp < m_lastValue )
    {
        BOOST_THROW_EXCEPTION( TimestampIsOlderThanLastValueException() );
    }

    if ( m_lastValue == Timestamp::min() )
    {
        m_values.push_back( _value );
        m_lastValue = _timestamp;
    }
    else
    {
        Duration sinceLastSample = _timestamp - m_lastValue;
        if ( sinceLastSample < m_valueDuration )
        {
            m_values.back() = _value;
        }
        else
        {
            while ( sinceLastSample >= m_valueDuration )
            {
                m_values.push_back(_value);

                sinceLastSample -= m_valueDuration;
            }
            m_lastValue = _timestamp;

            if ( m_next )
            {
                if ( m_next->m_lastValue == Timestamp::min() )
                {
                    m_next->add ( _timestamp, _value );
                }
                else
                {
                    Duration sinceNext = _timestamp - m_next->m_lastValue;
                    if ( sinceNext > m_next->m_valueDuration )
                    {
                        size_t neededSamples = m_next->m_valueDuration / m_valueDuration;
                        double total = 0;
                        size_t start = 0;
                        size_t samples = m_values.size();
                        if ( neededSamples < m_values.size() )
                        {
                            start = m_values.size() - neededSamples;
                            samples = neededSamples;
                        }
                        for ( size_t i = start; i < samples; ++i )
                        {
                            total += m_values[i];
                        }
                        double average = total / ((double) samples);

                        m_next->add ( _timestamp, average );
                    }
                }
            }
        }
    }
}

void ValueBuffer::dump ( std::ostream & _stream )
{
    _stream << "[";
    for ( Iterator it = begin(); it != end(); ++it )
    {
        _stream << *it << ", ";
    }
    _stream << "]";
    _stream << std::endl;
}
