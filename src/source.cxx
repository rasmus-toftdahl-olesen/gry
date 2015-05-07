#include <gry/source.h>

using namespace gry;
using namespace boost::filesystem;

Source::Source(const path & _directory)
    : m_directory(_directory)
{
    assert ( !is_regular_file(_directory) );

    refresh();
}

void Source::refresh()
{
    if ( !exists(m_directory) )
    {
        create_directory(m_directory);
    }

    path namePath = m_directory / "name.txt";
    if ( exists(namePath) )
    {

    }
    else
    {
        m_name = m_directory.filename().native();
    }
}
