#include <gry/web.h>
#include <gry/repository.h>
#include <boost/algorithm/string.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <pion/http/response_writer.hpp>

#include "config.h"

using namespace gry;
using namespace pion::http;
using namespace pion::tcp;
using namespace boost;

WebServer::WebServer( pion::scheduler & _scheduler, ushort _portNumber )
    : m_logger ( log4cpp::Category::getInstance("gry.web") ),
      m_server(_scheduler, _portNumber)
{
    m_server.add_resource("/data",
                          boost::bind(&WebServer::requestHandler, this, _1, _2));
    m_server.add_resource("/live",
                          boost::bind(&WebServer::requestHandlerLive, this, _1, _2));
    m_server.add_resource("/",
                          boost::bind(&WebServer::requestHandlerStatic, this, _1, _2));
}

void WebServer::start()
{
    m_server.start();
}

#ifdef GRY_INCLUDE_STATIC_WEB
#include "generated_web.cxx"

void WebServer::requestHandlerStatic(request_ptr & _request, connection_ptr & _conn)
{
    response_writer_ptr writer( response_writer::create(_conn, *_request,
                                                        boost::bind(&connection::finish, _conn)));
    if ( _request->get_resource() == "/" )
    {
        writer->write ( s_index_html );
    }
    else
    {

    }
    writer->send();
}
#else
#include <boost/filesystem/fstream.hpp>

void readFile ( response_writer_ptr & _writer, const std::string & _filename )
{
    std::stringstream buffer;
    boost::filesystem::ifstream fileStream ( _filename );
    if ( fileStream.is_open() )
    {
        buffer << fileStream.rdbuf();
        if ( fileStream.bad() )
        {
            _writer->write ( "Error reading file: " );
            _writer->write ( _filename );
        }
        _writer->write ( buffer.str() );
    }
    else
    {
        _writer->write ( "Could not open file: " );
        _writer->write ( _filename );
    }
}

void WebServer::requestHandlerStatic(request_ptr & _request, connection_ptr & _conn)
{
    response_writer_ptr writer( response_writer::create(_conn, *_request,
                                                        boost::bind(&connection::finish, _conn)));
    writer->get_response().change_header ( response::HEADER_CONTENT_ENCODING, "ascii" );
    if ( _request->get_resource() == "/" )
    {
        writer->get_response().set_content_type ( "text/html; charset=ascii" );
        readFile ( writer, GRY_STATIC_WEB_DIR "index.html" );
    }
    else if ( _request->get_resource() == "/jquery.js" )
    {
        writer->get_response().set_content_type ( "text/javascript; charset=ascii" );
        readFile ( writer, GRY_STATIC_WEB_DIR "jquery.js" );
    }
    else if ( _request->get_resource() == "/smoothie.js" )
    {
        writer->get_response().set_content_type ( "text/javascript; charset=ascii" );
        readFile ( writer, GRY_STATIC_WEB_DIR "smoothie.js" );
    }
    else
    {
        writer->write ( "ERROR: Unknown resource " );
        writer->write ( _request->get_resource() );
    }
    writer->send();
}
#endif

void WebServer::requestHandler(request_ptr & _request, connection_ptr & _conn)
{
    Repository & repo = Repository::instance();
    SourcePtr source;
    std::string sourceCmd;
    if ( starts_with(_request->get_resource(), "/data/source/") )
    {
        int endOfSourceName = _request->get_resource().find('/', 13);
        if ( endOfSourceName != -1 )
        {
            std::string sourceName = _request->get_resource().substr(13, endOfSourceName - 13);
            source = repo.findSourceByName(sourceName);
            sourceCmd = _request->get_resource().substr(endOfSourceName + 1);
        }
    }
    response_writer_ptr writer( response_writer::create(_conn, *_request,
                                                        boost::bind(&connection::finish, _conn)));

    writer->get_response().set_content_type ( "application/json; charset=ascii" );
    writer->get_response().change_header ( response::HEADER_CONTENT_ENCODING, "ascii" );

    if ( _request->get_resource() == "/data/sources" )
    {
        writer->write ( "{ \"sources\": [" );
        bool first  = true;
        std::vector<std::string> sources = repo.sourceNames();
        for ( std::vector<std::string>::iterator it = sources.begin(); it != sources.end(); ++it )
        {
            if ( first )
            {
                first = false;
            }
            else
            {
                writer->write ( "," );
            }
            writer->write ( '"' );
            writer->write ( *it );
            writer->write ( '"' );
        }
        writer->write ( "] }" );
    }
    else if ( source )
    {
        if ( sourceCmd == "info" )
        {
            writer->write ( "{ \"directory\": \"" );
            writer->write ( source->directory().native() );
            writer->write ( "\", \"samples\": " );
            writer->write ( source->numberOfSamples() );
            writer->write ( ", \"from\": \"" );
            writer->write ( source->oldest().get<0>() );
            writer->write ( "\", \"to\": \"" );
            writer->write ( source->newest().get<0>() );
            writer->write ( "\" }" );
        }
        else if ( sourceCmd == "add" )
        {
            double value = boost::lexical_cast<double> ( _request->get_query_string() );
            Source::Timestamp timestamp = source->add ( value );

            writer->write ( "{ \"timestamp\": \"" );
            writer->write ( timestamp );
            writer->write ( "\", value: " );
            writer->write ( value );
            writer->write ( " }" );
        }
        else if ( sourceCmd == "data" )
        {
            source->writeValues ( writer );
        }
        else
        {
            writer->get_response().set_status_code(types::RESPONSE_CODE_BAD_REQUEST);
            writer->get_response().set_status_message(types::RESPONSE_MESSAGE_BAD_REQUEST);
            writer->write ( "{ \"error\": \"Unknown source command: " );
            writer->write ( sourceCmd );
            writer->write ( "\" } " );
        }
    }
    else
    {
        writer->get_response().set_status_code(types::RESPONSE_CODE_BAD_REQUEST);
        writer->get_response().set_status_message(types::RESPONSE_MESSAGE_BAD_REQUEST);
        writer->write ( "{ \"error\": \"Unknown data request\", " );
        writer->write ( "  \"request\": \"" );
        writer->write ( _request->get_resource() );
        writer->write ( "\"} " );
    }
    writer->send();
}

void WebServer::requestHandlerLive(request_ptr & _request, connection_ptr & _conn)
{
    Repository & repo = Repository::instance();
    SourcePtr source;
    std::string sourceName = _request->get_resource().substr(6);
    source = repo.findSourceByName(sourceName);
    if ( source )
    {

        m_logger << log4cpp::Priority::INFO << "Subscription to " << sourceName << " from " << _conn->get_remote_ip();

        source->subscribe ( _conn );
    }
    else
    {
        m_logger << log4cpp::Priority::WARN << "Subscription to non-existing source " << sourceName << " from " << _conn->get_remote_ip();
    }
}
