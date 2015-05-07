#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <log4cpp/Category.hh>
#include <gry/source.h>

namespace gry
{
    typedef boost::shared_ptr<Source> SourcePtr;

    class Repository
    {
    private:
        static Repository s_instance;

        log4cpp::Category & m_logger;
        boost::filesystem::path m_dataDirectory;
        std::vector<SourcePtr> m_sources;
        boost::recursive_mutex m_lock;

        Repository();

    public:
        static Repository & instance()
        {
            return s_instance;
        }

        const boost::filesystem::path dataDirectory() const
        {
            return m_dataDirectory;
        }

        void discover();

        std::vector<std::string> sourceNames ();

        SourcePtr findSourceByName ( std::string _name );
        SourcePtr findSourceByPath ( boost::filesystem::path _directory );
    };
}
