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

namespace gry
{
    class Source
    {
    public:
        typedef boost::chrono::system_clock TimeSource;
        typedef TimeSource::time_point Timestamp;
        typedef boost::tuple<Timestamp,double> Value;

    private:
        boost::filesystem::path m_directory;
        std::string m_name;
        boost::recursive_mutex m_valuesLock;
        boost::circular_buffer<Value> m_values;
        std::vector<pion::tcp::connection_ptr> m_listeners;

    public:
        Source ( const boost::filesystem::path & _directory );

        const std::string & name () const
        {
            return m_name;
        }

        const boost::filesystem::path & directory () const
        {
            return m_directory;
        }

        int numberOfSamples();
        Value oldest();
        Value newest();
        Timestamp add ( double _value );
        void refresh();

        void writeValues ( pion::http::response_writer_ptr _writer );
        void subscribe ( pion::tcp::connection_ptr & _conn );
    };
}
