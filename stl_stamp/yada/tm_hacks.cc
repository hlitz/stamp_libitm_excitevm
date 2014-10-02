#include "tm_hacks.h"

bool custom_map_insertion(std::map<edge_t*, element_t*, element_mapCompareEdge_t>* map,
                          edge_t* edge,
                          element_t* element)
{
    return map->insert(std::make_pair(edge, element)).second;
}
