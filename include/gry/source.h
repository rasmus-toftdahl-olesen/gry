#include <string>
#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <ostream>
#include <pion/http/response_writer.hpp>
#include <pion/tcp/connection.hpp>
#include <gry/valuebuffer.h>

namespace gry
{
    class Source
    {
    public:
        typedef boost::chrono::system_clock TimeSource;
        typedef TimeSource::time_point Timestamp;
        typedef TimeSource::duration Duration;
        typedef boost::tuple<Timestamp,double> Value;

        static const size_t DEFAULT_BY_SECOND_VALUES = 60 * 60; // One hour of by-second values;
        static const size_t DEFAULT_BY_MINUTE_VALUES = 24 * 60; // 24 hours of by-minute values
        static const size_t DEFAULT_BY_HOUR_VALUES = 24 * 60; // Two months of hourly values
        static const size_t DEFAULT_BY_DAY_VALUES = 365 * 2; // Two years of daily values

    protected:
        boost::filesystem::path m_directory;
        std::string m_name;
        boost::recursive_mutex m_valuesLock;

        ValueBuffer m_bySecond;
        ValueBuffer m_byMinute;
        ValueBuffer m_byHour;
        ValueBuffer m_byDay;

        boost::recursive_mutex m_listenersLock;
        std::vector<pion::tcp::connection_ptr> m_secondsListeners;

    public:
        Source ( const boost::filesystem::path & _directory );

        const std::string & name () const
        {
            return m_name;
        }

        void setName ( const std::string & _name );

        const boost::filesystem::path & directory () const
        {
            return m_directory;
        }

        int numberOfBySecondValues();
        int numberOfByMinuteValues();
        int numberOfByHourValues();
        int numberOfByDayValues();

        Timestamp add ( double _value );
        void refresh( bool _intial );

        void writeBySecondValues ( pion::http::response_writer_ptr _writer );
        void writeByMinuteValues ( pion::http::response_writer_ptr _writer );
        void writeByHourValues ( pion::http::response_writer_ptr _writer );
        void writeByDayValues ( pion::http::response_writer_ptr _writer );

        void subscribeSeconds ( pion::tcp::connection_ptr & _conn );

    protected:
        void saveValues ( const std::string & _filename, const ValueBuffer & _values ) const;
        void loadValues ( const std::string & _filename, ValueBuffer & _values ) const;
    };
}
