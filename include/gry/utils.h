#include <string>
#include <boost/filesystem.hpp>

namespace gry
{
    std::string read_text_file ( const std::string & _filepath );
    void write_text_file ( const std::string & _filepath, const std::string & _content );

    inline std::string read_text_file ( const boost::filesystem::path & _filepath )
    {
        return read_text_file ( _filepath.native() );
    }

    inline void write_text_file ( const boost::filesystem::path & _filepath, const std::string & _content )
    {
        write_text_file ( _filepath.native(), _content );
    }
}
