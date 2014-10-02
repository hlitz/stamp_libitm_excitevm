#pragma once

#include <map>
#include "element.h"

// In the version of GCC we are currently using, the use of std::make_pair()
// in two different files is resulting in link-time errors due to transaction
// clones for std::pair constructors being multiply defined.  We are hacking
// our way around this by wrapping the insertion in a function call, and then
// doing all inserts from one place.

__attribute__((transaction_safe))
bool custom_map_insertion(std::map<edge_t*, element_t*, element_mapCompareEdge_t>* map,
                          edge_t* edge, element_t* element);
