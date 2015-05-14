#include <pion/http/server.hpp>

namespace gry
{
    class WebServer
    {
    public:
        WebServer( pion::scheduler & _scheduler, ushort _portNumber );
        void start();

    private:
        pion::http::server m_server;

        void requestHandler(pion::http::request_ptr & _request, pion::tcp::connection_ptr & _conn);
        void requestHandlerStatic(pion::http::request_ptr & _request, pion::tcp::connection_ptr & _conn);
    };
}
