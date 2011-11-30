#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS

#include <boost/functional/hash.hpp>
#include <boost/dynamic_bitset.hpp>

using namespace boost;
using namespace std;

namespace boost {
    template <typename B, typename A>
    std::size_t hash_value(const boost::dynamic_bitset<B, A>& bs) {            
        return boost::hash_value(bs.m_bits);
    }
}
