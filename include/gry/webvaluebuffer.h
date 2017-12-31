#include <gry/valuebuffer.h>
#include <boost/thread/recursive_mutex.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/tcp/connection.hpp>
#include <vector>

namespace gry
{
    class WebValueBuffer : public ValueBuffer
    {
    protected:
        boost::recursive_mutex m_listenersLock;
        std::vector<pion::tcp::connection_ptr> m_listeners;

    public:
        WebValueBuffer ( size_t _numberOfValues, Duration _valueDuration );
        virtual ~WebValueBuffer();

        virtual int add ( Timestamp _timestamp, double _value );
        void subscribe ( const pion::tcp::connection_ptr & _conn );
    };
}
