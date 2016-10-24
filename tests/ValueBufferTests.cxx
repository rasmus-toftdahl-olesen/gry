#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <gry/valuebuffer.h>
#include <gry/utils.h>

using namespace gry;

struct ValueBufferFixture : public ValueBuffer
{
    ValueBufferFixture * m_fixtureNext;

    ValueBufferFixture()
        : ValueBuffer(10, boost::chrono::seconds(1))
    {
        m_fixtureNext = new ValueBufferFixture(10, boost::chrono::minutes(1));
        m_next = m_fixtureNext;
    }

    ValueBufferFixture(size_t _size, Duration _duration)
        : ValueBuffer(_size, _duration)
    {
    }

    ~ValueBufferFixture()
    {
        delete m_next;
    }

    const boost::circular_buffer<double> & values() const { return m_values; }
};

BOOST_FIXTURE_TEST_CASE ( TestValueBufferIsAllZeroOnCreation, ValueBufferFixture )
{
    BOOST_CHECK_EQUAL ( this->m_values.size(), 10 );
    for ( size_t i = 0; i < 10; i++ )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferDuration, ValueBufferFixture )
{
    BOOST_CHECK_EQUAL ( this->bufferDuration(), boost::chrono::seconds(10) );
}

ValueBuffer::Duration abs(ValueBuffer::Duration duration)
{
    ValueBuffer::Duration ret(abs(duration.count()));
    return ret;
}

BOOST_FIXTURE_TEST_CASE ( TestAbsTestUtilityFunction, ValueBufferFixture )
{
    BOOST_CHECK_EQUAL ( ValueBuffer::Duration(1), abs(ValueBuffer::Duration(-1)) );
    BOOST_CHECK_EQUAL ( boost::chrono::seconds(1), abs(boost::chrono::seconds(-1)) );
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferTimeSinceLastValue, ValueBufferFixture )
{
    // No values added yet, time since last value should be -1
    BOOST_CHECK_EQUAL ( this->timeSinceLastValue(), ValueBuffer::Duration(-1) );

    this->add ( TimeSource::now(), 1.0 );
    BOOST_CHECK_LT ( abs(this->timeSinceLastValue() - boost::chrono::seconds(0)), boost::chrono::milliseconds(1) );

    boost::this_thread::sleep(boost::posix_time::seconds(1));

    BOOST_CHECK_LT ( abs(this->timeSinceLastValue() - boost::chrono::seconds(1)), boost::chrono::milliseconds(1) );
    this->add ( TimeSource::now(), 2.0 );

    BOOST_CHECK_LT ( abs(this->timeSinceLastValue()), boost::chrono::milliseconds(1) );
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferAddFirstSample, ValueBufferFixture )
{
    BOOST_CHECK ( std::isnan(this->m_values.back()) );

    Timestamp now = TimeSource::now();

    this->add ( now, 1 );

    BOOST_CHECK_EQUAL ( this->m_values.back(), 1 );

    for ( size_t i = 0; i < this->m_values.size() - 1; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }

    BOOST_CHECK_EQUAL ( this->m_lastValue, now );
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferSamplesAddedWithinTheSecondMeansThatTheLastValueWins, ValueBufferFixture )
{
    BOOST_CHECK ( std::isnan(this->m_values.back()) );

    Timestamp now = TimeSource::now();

    this->add ( now + boost::chrono::milliseconds(100), 1 );

    BOOST_CHECK_EQUAL ( this->m_values.back(), 1 );

    this->add ( now + boost::chrono::milliseconds(200), 2 );
    this->add ( now + boost::chrono::milliseconds(300), 3 );
    this->add ( now + boost::chrono::milliseconds(400), 4 );

    BOOST_CHECK_EQUAL ( this->m_values.back(), 4 );

    for ( size_t i = 0; i < this->m_values.size() - 1; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferSampleAddedEverySecond, ValueBufferFixture )
{
    Timestamp now = TimeSource::now();

    this->add ( now, 1 );
    this->add ( now + boost::chrono::seconds(1), 2 );
    this->add ( now + boost::chrono::seconds(2), 3 );
    this->add ( now + boost::chrono::seconds(3), 4 );

    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 1], 4 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 2], 3 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 3], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 4], 1 );

    for ( size_t i = 0; i < this->m_values.size() - 4; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferaAddWithFiveSecondInterval, ValueBufferFixture )
{
    Timestamp now = TimeSource::now();

    this->add ( now, 1 );
    this->add ( now + boost::chrono::seconds(5), 2 );

    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 1], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 2], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 3], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 4], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 5], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 6], 1 );

    for ( size_t i = 0; i < this->m_values.size() - 6; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }
}

BOOST_FIXTURE_TEST_CASE ( TestValueBufferaAddWithMoreTimeThanTheBufferCanHold, ValueBufferFixture )
{
    Timestamp now = TimeSource::now();

    this->add ( now, 1 );
    this->add ( now + boost::chrono::seconds(1), 1 );
    this->add ( now + boost::chrono::seconds(65), 2 );

    for ( size_t i = 0; i < this->m_values.size(); ++i )
    {
        BOOST_CHECK_EQUAL ( this->m_values[i], 2 );
    }

    //this->m_fixtureNext->dump(std::cout);

    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[8], 1 );
    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[9], 2 );
    for ( size_t i = 0; i < this->m_fixtureNext->values().size() - 2; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_fixtureNext->values()[i]) );
    }
}
