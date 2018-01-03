#include <boost/circular_buffer.hpp>
#include <boost/chrono.hpp>
#include <boost/exception/all.hpp>
#include <ostream>

namespace gry
{
    class ValueBuffer
    {
    public:
        typedef boost::chrono::system_clock TimeSource;
        typedef TimeSource::time_point Timestamp;
        typedef TimeSource::duration Duration;
        typedef boost::circular_buffer<double>::iterator Iterator;
        typedef boost::circular_buffer<double>::const_iterator ConstIterator;
        typedef boost::circular_buffer<double>::size_type SizeType;

        ValueBuffer ( size_t _numberOfValues, Duration _valueDuration );
        virtual ~ValueBuffer();

        virtual int add ( Timestamp _timestamp, double _value );
        void setNext ( ValueBuffer * _nextBuffer );
        void dump ( std::ostream & _stream ) const;
        void save ( std::ostream & _stream ) const;
        void load ( std::istream & _stream );
        void setLastValueAt ( Timestamp _lastValueAt );
        void reset();
        inline Iterator begin() { return m_values.begin(); }
        inline Iterator end() { return m_values.end(); }
        inline ConstIterator begin() const { return m_values.begin(); }
        inline ConstIterator end() const { return m_values.end(); }
        inline SizeType size() const { return m_values.size(); }
        inline double operator[] ( SizeType _index ) const { return m_values[_index]; }
        inline double front() const { return m_values.front(); }
        inline double back() const { return m_values.back(); }
        inline Duration bufferDuration() const { return m_values.size() * m_valueDuration; }
        inline Timestamp lastValueAt() const { return m_lastValue; }
        Duration timeSinceLastValue() const;

    protected:
        Duration m_valueDuration;
        Timestamp m_lastValue;
        boost::circular_buffer<double> m_values;
        ValueBuffer * m_next;
    };

    struct TimestampIsOlderThanLastValueException : virtual std::exception, virtual boost::exception
    {
    };

    struct NextValueBufferIsTooLarge : virtual std::exception, virtual boost::exception
    {
    };
}
