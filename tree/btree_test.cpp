/*g++ test.cpp -I btree*/
#include <iostream>
#include <stdlib.h>
#include <stx/btree_multiset>
using namespace std;
/// Traits used for the speed tests, BTREE_DEBUG is not defined.
template <int _innerslots, int _leafslots>
struct btree_traits_speed
{
    static const bool   selfverify = false;
    static const bool   debug = false;

    static const int    leafslots = _innerslots;
    static const int    innerslots = _leafslots;
};
int main(int argc, const char *argv[])
{
    stx::btree_multiset<unsigned int>  bt;
    srand(1000);
    int insertnum=10000;
    for(unsigned int i = 0; i < insertnum; i++)
        bt.insert(rand());
    
    assert( bt.size() == insertnum );
    return 0;
}
