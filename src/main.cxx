#include <gry/udp.h>
#include <gry/web.h>
#include <gry/repository.h>
#include <log4cpp/PropertyConfigurator.hh>
#include "config.h"

using namespace gry;

int main ()
{
    boost::filesystem::path initFileName = Repository::instance().dataDirectory() / "log4cpp.properties";
    if ( exists(initFileName) )
    {
        log4cpp::PropertyConfigurator::configure(initFileName.native());
    }

    pion::single_service_scheduler scheduler;
    WebServer server ( scheduler, GRY_WEB_PORT );
    server.start();

    UdpServer udpServer ( scheduler, GRY_UDP_PORT );
    udpServer.start();

    while (1)
    {
        Repository::instance().discover();
        sleep(5);
    }
    return 0;
}
