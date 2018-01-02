#include <pion/http/server.hpp>
#include <log4cpp/Category.hh>

#ifndef GRY_WEB_H
#define GRY_WEB_H

namespace gry
{
#ifdef GRY_NON_CONST_PION_PTRS
    typedef pion::tcp::connection_ptr & ConnectionPtr;
    typedef pion::http::request_ptr & RequestPtr;
#else
    typedef const pion::tcp::connection_ptr & ConnectionPtr;
    typedef const pion::http::request_ptr & RequestPtr;
#endif

    class WebServer
    {
    public:
        WebServer( pion::scheduler & _scheduler, ushort _portNumber );
        void start();

    private:
        log4cpp::Category & m_logger;
        pion::http::server m_server;

        void requestHandler(RequestPtr _request, ConnectionPtr _conn);
        void requestHandlerLive(RequestPtr _request, ConnectionPtr _conn);
        void requestHandlerStatic(RequestPtr _request, ConnectionPtr _conn);
    };
}

#endif
