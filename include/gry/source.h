#include <string>
#include <boost/filesystem.hpp>

namespace gry
{
    class Source
    {
    private:
        boost::filesystem::path m_directory;
        std::string m_name;

    public:
        Source ( const boost::filesystem::path & _directory );

        const std::string & name () const
        {
            return m_name;
        }

        const boost::filesystem::path & directory () const
        {
            return m_directory;
        }

        void refresh();
    };
}
