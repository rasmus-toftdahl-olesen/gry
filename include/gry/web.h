#include <pion/http/server.hpp>

namespace gry
{
    class WebServer
    {
    public:
        WebServer();
        void start();

    private:
        pion::http::server m_server;

        void requestHandler(pion::http::request_ptr & _request, pion::tcp::connection_ptr & _conn);
    };
}
