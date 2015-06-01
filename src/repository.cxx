#include <gry/repository.h>
#include <boost/thread/lock_guard.hpp>

using namespace gry;
using namespace boost::filesystem;

Repository Repository::s_instance;

Repository::Repository()
    : m_logger ( log4cpp::Category::getInstance("gry.repository") )
{
    m_dataDirectory = current_path() / "data";

    m_logger << log4cpp::Priority::INFO << "Using data directory: " << m_dataDirectory;
    if ( !exists(m_dataDirectory) )
    {
        m_logger << log4cpp::Priority::INFO << "Creating data directory: " << m_dataDirectory;
        create_directory(m_dataDirectory);
    }
}

void Repository::discover()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_lock );

    m_logger << log4cpp::Priority::DEBUG << "Discovering";

    for ( directory_iterator it = directory_iterator(m_dataDirectory); it != directory_iterator(); ++it )
    {
        if ( is_directory(*it) )
        {
            SourcePtr sourceIt = findSourceByPath(*it);
            if ( sourceIt.get() == 0 )
            {
                m_logger << log4cpp::Priority::INFO << "Found new source " << *it;
                m_sources.push_back ( SourcePtr(new Source(*it)) );
            }
        }
    }

    for ( std::vector<SourcePtr>::iterator it = m_sources.begin(); it != m_sources.end(); )
    {
        if ( exists(it->get()->directory()) )
        {
            it->get()->refresh( false );
            it++;
        }
        else
        {
            it = m_sources.erase(it);
        }
    }
}

std::vector<std::string> Repository::sourceNames ()
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_lock );

    std::vector<std::string> ret;

    for ( std::vector<SourcePtr>::iterator it = m_sources.begin(); it != m_sources.end(); ++it )
    {
        ret.push_back ( it->get()->name() );
    }
    return ret;
}

SourcePtr Repository::findSourceByName ( std::string _name )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_lock );

    for ( std::vector<SourcePtr>::iterator it = m_sources.begin(); it != m_sources.end(); ++it )
    {
        if ( it->get()->name() == _name )
        {
            return *it;
        }
    }
    return SourcePtr();
}

SourcePtr Repository::findSourceByPath ( path _directory )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_lock );

    for ( std::vector<SourcePtr>::iterator it = m_sources.begin(); it != m_sources.end(); ++it )
    {
        if ( it->get()->directory() == _directory )
        {
            return *it;
        }
    }
    return SourcePtr();
}

SourcePtr Repository::getOrCreateSourceByName ( std::string _name )
{
    boost::lock_guard<boost::recursive_mutex> guard ( m_lock );

    SourcePtr source = findSourceByName(_name);
    if ( !source )
    {
        m_logger << log4cpp::Priority::INFO << "Adding new source " << _name;
        source = SourcePtr(new Source(m_dataDirectory / _name));
        m_sources.push_back (  source );
    }
    return source;
}
