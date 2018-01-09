#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <gry/valuebuffer.h>
#include <gry/utils.h>

using namespace gry;

struct ValueBufferRealWorldFixture : public ValueBuffer
{
    ValueBufferRealWorldFixture * m_fixtureNext;

    ValueBufferRealWorldFixture()
        : ValueBuffer(120, boost::chrono::seconds(1))
    {
        m_fixtureNext = new ValueBufferRealWorldFixture(10, boost::chrono::minutes(1));
        m_next = m_fixtureNext;
    }

    ValueBufferRealWorldFixture(size_t _size, Duration _duration)
        : ValueBuffer(_size, _duration)
    {
    }

    ~ValueBufferRealWorldFixture()
    {
        delete m_next;
    }

    const boost::circular_buffer<double> & values() const { return m_values; }
};


BOOST_FIXTURE_TEST_CASE ( Test1, ValueBufferRealWorldFixture )
{
    BOOST_CHECK ( std::isnan(this->m_values.back()) );

    Timestamp now = TimeSource::now();

    this->add ( now, 1 );

    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[this->m_fixtureNext->values().size() - 1], 1 );

    this->add ( now + boost::chrono::seconds(1), 2 );
    this->add ( now + boost::chrono::seconds(2), 3 );
    this->add ( now + boost::chrono::seconds(3), 4 );
    this->add ( now + boost::chrono::seconds(4), 5 );

    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 1], 5 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 2], 4 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 3], 3 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 4], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 5], 1 );

    for ( size_t i = 0; i < this->m_values.size() - 5; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }

    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[this->m_fixtureNext->values().size() - 1], 1 );
    for ( size_t i = 0; i < this->m_fixtureNext->values().size() - 1; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_fixtureNext->values()[i]) );
    }

    this->add ( now + boost::chrono::seconds(61), 6 );
    this->add ( now + boost::chrono::seconds(62), 7 );
    this->add ( now + boost::chrono::seconds(63), 8 );
    this->add ( now + boost::chrono::seconds(64), 9 );

    //this->dump(std::cout);
    //othis->m_fixtureNext->dump(std::cout);

    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 1], 9 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 2], 8 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 3], 7 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 4], 6 );

    for ( size_t i = this->m_values.size() - 60; i < this->m_values.size() - 4; ++i )
    {
        //std::cout << i << "   " << m_values[i] << std::endl;
        BOOST_CHECK_EQUAL ( this->m_values[i], 6 );
    }

    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 61], 5 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 62], 4 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 63], 3 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 64], 2 );
    BOOST_CHECK_EQUAL ( this->m_values[this->m_values.size() - 65], 1 );

    for ( size_t i = 0; i < 55; ++i )
    {
        //std::cout << i << "   " << m_values[i] << std::endl;
        BOOST_CHECK ( std::isnan(this->m_values[i]) );
    }

    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[this->m_fixtureNext->values().size() - 1], 5.9 );
    BOOST_CHECK_EQUAL ( this->m_fixtureNext->values()[this->m_fixtureNext->values().size() - 2], 1 );
    for ( size_t i = 0; i < this->m_fixtureNext->values().size() - 2; ++i )
    {
        BOOST_CHECK ( std::isnan(this->m_fixtureNext->values()[i]) );
    }
}
