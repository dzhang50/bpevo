#ifndef HELPER
#define HELPER
#endif

#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS

#include <boost/functional/hash.hpp>
#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>

using namespace boost;
using namespace std;

size_t lengthlimit = sizeof(ulong) * 8;

namespace boost {
    template <typename B, typename A>
    std::size_t hash_value(const boost::dynamic_bitset<B, A>& bs) {            
        return boost::hash_value(bs.m_bits);
    }
}
