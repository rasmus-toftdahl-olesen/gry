#include <gry/udp.h>
#include <gry/web.h>
#include <gry/repository.h>
#include <log4cpp/PropertyConfigurator.hh>

using namespace gry;

int main ()
{
    boost::filesystem::path initFileName = Repository::instance().dataDirectory() / "log4cpp.properties";
    if ( exists(initFileName) )
    {
        log4cpp::PropertyConfigurator::configure(initFileName.native());
    }

    pion::single_service_scheduler scheduler;
    WebServer server ( scheduler, 8123 );
    server.start();

    UdpServer udpServer ( scheduler, 8124 );
    udpServer.start();

    while (1)
    {
        Repository::instance().discover();
        sleep(5);
    }
    return 0;
}
