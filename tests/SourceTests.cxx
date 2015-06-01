#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <gry/source.h>
#include <gry/utils.h>

using namespace gry;

static boost::filesystem::path data_dir("unit_test_source");

struct F : public Source
{
    F()
        : Source(data_dir)
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
    this->setName ( "New Name" );

    BOOST_CHECK_EQUAL ( read_text_file(data_dir / "name.txt"), "New Name" );
}

BOOST_FIXTURE_TEST_CASE ( TestSourceNameIsStillSetAfterRefresh, F )
{
    this->setName ( "My Source" );
    this->refresh( false );
    BOOST_CHECK_EQUAL(this->name(), "My Source" );
}

BOOST_FIXTURE_TEST_CASE ( TestSourceDataIsAllNulUponCreation, F )
{
    size_t v = DEFAULT_BY_SECOND_VALUES;
    BOOST_CHECK_EQUAL ( this->m_bySecond.size(), v );

    v = DEFAULT_BY_MINUTE_VALUES;
    BOOST_CHECK_EQUAL ( this->m_byMinute.size(), v );

    v = DEFAULT_BY_HOUR_VALUES;
    BOOST_CHECK_EQUAL ( this->m_byHour.size(), v );

    v = DEFAULT_BY_DAY_VALUES;
    BOOST_CHECK_EQUAL ( this->m_byDay.size(), v );
}

BOOST_FIXTURE_TEST_CASE ( TestSourceDataAddFirstSample, F )
{
    BOOST_CHECK_EQUAL ( this->m_bySecond.back(), 0 );

    this->add ( 1 );

    BOOST_CHECK_EQUAL ( this->m_bySecond.back(), 1 );

    for ( size_t i = 0; i < this->m_bySecond.size() - 1; ++i )
    {
        BOOST_CHECK_EQUAL ( this->m_bySecond[i], 0 );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestSourceDataAddedWithinTheSecondMeansThatTheLastValueWins, F )
{
    BOOST_CHECK_EQUAL ( this->m_bySecond.back(), 0 );

    this->add ( 1 );
    this->add ( 2 );
    this->add ( 3 );
    this->add ( 4 );

    BOOST_CHECK_EQUAL ( this->m_bySecond.back(), 4 );

    for ( size_t i = 0; i < this->m_bySecond.size() - 1; ++i )
    {
        BOOST_CHECK_EQUAL ( this->m_bySecond[i], 0 );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestSourceDataAddEverySecond, F )
{
    this->add ( 1 );

    boost::this_thread::sleep_for ( boost::chrono::seconds(1) );

    this->add ( 2 );

    boost::this_thread::sleep_for ( boost::chrono::seconds(1) );

    this->add ( 3 );

    boost::this_thread::sleep_for ( boost::chrono::seconds(1) );

    this->add ( 4 );

    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 1], 4 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 2], 3 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 3], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 4], 1 );

    for ( size_t i = 0; i < this->m_bySecond.size() - 4; ++i )
    {
        BOOST_CHECK_EQUAL ( this->m_bySecond[i], 0 );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestSourceDataAddWithFiveSecondInterval, F )
{
    this->add ( 1 );

    boost::this_thread::sleep_for ( boost::chrono::seconds(5) );

    this->add ( 2 );

    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 1], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 2], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 3], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 4], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 5], 2 );
    BOOST_CHECK_EQUAL ( this->m_bySecond[this->m_bySecond.size() - 6], 1 );

    for ( size_t i = 0; i < this->m_bySecond.size() - 6; ++i )
    {
        BOOST_CHECK_EQUAL ( this->m_bySecond[i], 0 );
    }
}
