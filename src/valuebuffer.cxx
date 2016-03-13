#include <gry/valuebuffer.h>
#include <boost/throw_exception.hpp>

using namespace gry;

ValueBuffer::ValueBuffer ( size_t _numberOfValues, Duration _valueDuration )
    : m_valueDuration(_valueDuration),
      m_lastValue(Timestamp::min()),
      m_values(_numberOfValues),
      m_next(0)
{
    reset();
}

void ValueBuffer::reset()
{
    for ( SizeType i = 0; i < m_values.capacity(); i++ )
    {
        m_values.push_back(0);
    }
}

void ValueBuffer::setNext ( ValueBuffer * _nextBuffer )
{
    m_next = _nextBuffer;
}

int ValueBuffer::add ( Timestamp _timestamp, double _value )
{
    int valuesAdded = 0;
    if ( _timestamp < m_lastValue )
    {
        BOOST_THROW_EXCEPTION( TimestampIsOlderThanLastValueException() );
    }

    if ( m_lastValue == Timestamp::min() )
    {
        m_values.push_back( _value );
        m_lastValue = _timestamp;
        valuesAdded = 1;
    }
    else
    {
        Duration sinceLastSample = _timestamp - m_lastValue;
        if ( sinceLastSample < m_valueDuration )
        {
            m_values.back() = _value;
            valuesAdded = 0;
        }
        else
        {
            while ( sinceLastSample >= m_valueDuration )
            {
                m_values.push_back(_value);

                sinceLastSample -= m_valueDuration;

                valuesAdded += 1;
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
    return valuesAdded;
}

void ValueBuffer::dump ( std::ostream & _stream ) const
{
    _stream << "[";
    for ( ConstIterator it = begin(); it != end(); ++it )
    {
        _stream << *it << ", ";
    }
    _stream << "]";
    _stream << std::endl;
}

void ValueBuffer::save ( std::ostream & _stream ) const
{
    for ( ConstIterator it = begin(); it != end(); ++it )
    {
        _stream.write ( reinterpret_cast<const char*>(&*it), sizeof(double) );
    }
}

void ValueBuffer::load ( std::istream & _stream )
{
    reset();

    for ( Iterator it = begin(); it != end(); ++it )
    {
        _stream.read ( reinterpret_cast<char*>( &*it ), sizeof(double) );
    }
}
