#include <boost/test/unit_test.hpp>
#include <boost/circular_buffer.hpp>

using namespace boost;

BOOST_AUTO_TEST_CASE ( TestCircularBuffer )
{
    circular_buffer<int> b ( 4 );
    BOOST_CHECK_EQUAL(0, b.size());
    b.push_back(0);
    b.push_back(0);
    b.push_back(0);
    b.push_back(0);
    BOOST_CHECK_EQUAL(4, b.size());
    BOOST_CHECK_EQUAL ( 0, b[0] );
    BOOST_CHECK_EQUAL ( 0, b[1] );
    BOOST_CHECK_EQUAL ( 0, b[2] );
    BOOST_CHECK_EQUAL ( 0, b[3] );
    b.push_back(0);
    b.push_back(0);
    BOOST_CHECK_EQUAL ( 0, b[0] );
    BOOST_CHECK_EQUAL ( 0, b[1] );
    BOOST_CHECK_EQUAL ( 0, b[2] );
    BOOST_CHECK_EQUAL ( 0, b[3] );
    b.push_back(1);
    BOOST_CHECK_EQUAL ( 0, b[0] );
    BOOST_CHECK_EQUAL ( 0, b[1] );
    BOOST_CHECK_EQUAL ( 0, b[2] );
    BOOST_CHECK_EQUAL ( 1, b[3] );
    b.push_back(2);
    BOOST_CHECK_EQUAL ( 0, b[0] );
    BOOST_CHECK_EQUAL ( 0, b[1] );
    BOOST_CHECK_EQUAL ( 1, b[2] );
    BOOST_CHECK_EQUAL ( 2, b[3] );
    b.push_back(3);
    BOOST_CHECK_EQUAL ( 0, b[0] );
    BOOST_CHECK_EQUAL ( 1, b[1] );
    BOOST_CHECK_EQUAL ( 2, b[2] );
    BOOST_CHECK_EQUAL ( 3, b[3] );
    b.push_back(4);
    BOOST_CHECK_EQUAL ( 1, b[0] );
    BOOST_CHECK_EQUAL ( 2, b[1] );
    BOOST_CHECK_EQUAL ( 3, b[2] );
    BOOST_CHECK_EQUAL ( 4, b[3] );
    b.push_back(5);
    BOOST_CHECK_EQUAL ( 2, b[0] );
    BOOST_CHECK_EQUAL ( 3, b[1] );
    BOOST_CHECK_EQUAL ( 4, b[2] );
    BOOST_CHECK_EQUAL ( 5, b[3] );
}
