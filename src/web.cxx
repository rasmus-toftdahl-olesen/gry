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
    else if ( _request->get_resource() == "/jquery.js" )
    {
        writer->get_response().set_content_type ( "text/javascript; charset=ascii" );
        writer->write ( s_jquery_js );
    }
    else if ( _request->get_resource() == "/jquery.flot.min.js" )
    {
        writer->get_response().set_content_type ( "text/javascript; charset=ascii" );
        writer->write ( s_jquery_flot_min_js );
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
    else if ( _request->get_resource() == "/jquery.flot.min.js" )
    {
        writer->get_response().set_content_type ( "text/javascript; charset=ascii" );
        readFile ( writer, GRY_STATIC_WEB_DIR "jquery.flot.min.js" );
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
        size_t endOfSourceName = _request->get_resource().find('/', 13);
        if ( endOfSourceName != std::string::npos )
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
            writer->write ( "\", \"by_second_values\": " );
            writer->write ( source->numberOfBySecondValues() );
            writer->write ( ", \"by_minute_values\": " );
            writer->write ( source->numberOfByMinuteValues() );
            writer->write ( ", \"by_hour_values\": " );
            writer->write ( source->numberOfByHourValues() );
            writer->write ( ", \"by_day_values\": " );
            writer->write ( source->numberOfByDayValues() );
            writer->write ( ", \"time_since_last_value\": " );
            boost::chrono::seconds inSeconds;
            inSeconds = boost::chrono::duration_cast<boost::chrono::seconds>(source->timeSinceLastValue());
            writer->write ( inSeconds.count() );
            writer->write ( " }" );
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
        else if ( sourceCmd == "seconds/data" )
        {
            source->writeBySecondValues ( writer );
        }
        else if ( sourceCmd == "minutes/data" )
        {
            source->writeByMinuteValues ( writer );
        }
        else if ( sourceCmd == "hours/data" )
        {
            source->writeByHourValues ( writer );
        }
        else if ( sourceCmd == "days/data" )
        {
            source->writeByDayValues ( writer );
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
    std::string sourceAndValues = _request->get_resource().substr(6);
    size_t i = sourceAndValues.find('/');
    if ( i != std::string::npos )
    {
        std::string sourceName = sourceAndValues.substr(0, i);
        std::string valuesName = sourceAndValues.substr(i + 1);
        source = repo.findSourceByName(sourceName);
        if ( source )
        {

            m_logger << log4cpp::Priority::INFO << "Subscription to " << sourceName << " " << valuesName << " from " << _conn->get_remote_ip();

            if ( valuesName == "seconds" )
            {
                source->subscribeSecond ( _conn );
            }
            else if ( valuesName == "minutes" )
            {
                source->subscribeMinute ( _conn );
            }
            else if ( valuesName == "hours" )
            {
                source->subscribeHour ( _conn );
            }
            else if ( valuesName == "days" )
            {
                source->subscribeDay ( _conn );
            }
            else
            {
                m_logger << log4cpp::Priority::WARN << "Subscription to non-existing values " << valuesName << " from " << _conn->get_remote_ip();
            }
        }
        else
        {
            m_logger << log4cpp::Priority::WARN << "Subscription to non-existing source " << sourceName << " from " << _conn->get_remote_ip();
        }
    }
}
