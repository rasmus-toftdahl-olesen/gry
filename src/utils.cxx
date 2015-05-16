#include <gry/utils.h>
#include <fstream>
#include <sstream>

using namespace std;

namespace gry
{
    string read_text_file ( const string & _filepath )
    {
        ifstream t(_filepath.c_str());
        stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    void write_text_file ( const string & _filepath, const string & _content )
    {
        ofstream t(_filepath.c_str());
        t << _content;
    }
}
