#include <boost/test/unit_test.hpp>
#include <gry/source.h>
#include <gry/utils.h>

using namespace gry;

struct F
{
    boost::filesystem::path data_dir;
    Source source;

    F()
        : data_dir("unit_test_source"),
          source(data_dir)
    {
    }


    ~F()
    {
        remove_all(data_dir);
    }
};

BOOST_FIXTURE_TEST_CASE ( TestSourceDataDirectorIsCreatedOnCreation, F )
{
    BOOST_CHECK ( exists(data_dir) );
}

BOOST_FIXTURE_TEST_CASE ( TestSourceSettingNameChangesNameFile, F )
{
    source.setName ( "New Name" );

    BOOST_CHECK_EQUAL ( read_text_file(data_dir / "name.txt"), "New Name" );
}

BOOST_FIXTURE_TEST_CASE ( TestSourceNameIsStillSetAfterRefresh, F )
{
    source.setName ( "My Source" );
    source.refresh();
    BOOST_CHECK_EQUAL(source.name(), "My Source" );
}
