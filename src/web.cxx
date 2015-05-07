#include <gry/web.h>
#include <gry/repository.h>
#include <boost/algorithm/string.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <pion/http/response_writer.hpp>

using namespace gry;
using namespace pion::http;
using namespace pion::tcp;
using namespace boost;

WebServer::WebServer()
    : m_server(8123)
{
    m_server.add_resource("/data",
                         boost::bind(&WebServer::requestHandler, this, _1, _2));
}

void WebServer::start()
{
    m_server.start();
}

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

    response resp ( writer->get_response() );
    resp.set_content_type ( "text/json" );
    if ( _request->get_resource() == "/data/sources" )
    {
        writer->write ( "{ sources: [" );
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
            writer->write ( "'" );
            writer->write ( *it );
            writer->write ( "'" );
        }
        writer->write ( "] }" );
    }
    else if ( source )
    {
        if ( sourceCmd == "info" )
        {
            writer->write ( "{ directory: '" );
            writer->write ( source->directory().native() );
            writer->write ( "', samples: '" );
            writer->write ( source->numberOfSamples() );
            writer->write ( "', from: '" );
            writer->write ( source->oldest().get<0>() );
            writer->write ( "', to: '" );
            writer->write ( source->newest().get<0>() );
            writer->write ( "' }" );
        }
        else if ( sourceCmd == "add" )
        {
            double value = boost::lexical_cast<double> ( _request->get_query_string() );
            Source::Timestamp timestamp = source->add ( value );

            writer->write ( "{ timestamp: '" );
            writer->write ( timestamp );
            writer->write ( "'," );
            writer->write ( value );
            writer->write ( "' }" );
        }
        else if ( sourceCmd == "data" )
        {
            source->writeValues ( writer );
        }
        else
        {
            resp.set_status_code(types::RESPONSE_CODE_BAD_REQUEST);
            resp.set_status_message(types::RESPONSE_MESSAGE_BAD_REQUEST);
            writer->write ( "{ error: 'Unknown source command: " );
            writer->write ( sourceCmd );
            writer->write ( "' } " );
        }
    }
    else
    {
        resp.set_status_code(types::RESPONSE_CODE_BAD_REQUEST);
        resp.set_status_message(types::RESPONSE_MESSAGE_BAD_REQUEST);
        writer->write ( "{ error: 'Unknown data request', " );
        writer->write ( "  request: '" );
        writer->write ( _request->get_resource() );
        writer->write ( "'} " );
    }
    writer->send();
}
