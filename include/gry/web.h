#include <pion/http/server.hpp>
#include <log4cpp/Category.hh>

namespace gry
{
    class WebServer
    {
    public:
        WebServer( pion::scheduler & _scheduler, ushort _portNumber );
        void start();

    private:
        log4cpp::Category & m_logger;
        pion::http::server m_server;

        void requestHandler(const pion::http::request_ptr & _request, const pion::tcp::connection_ptr & _conn);
        void requestHandlerLive(const pion::http::request_ptr & _request, const pion::tcp::connection_ptr & _conn);
        void requestHandlerStatic(const pion::http::request_ptr & _request, const pion::tcp::connection_ptr & _conn);
    };
}
