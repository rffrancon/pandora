#ifndef __LOCALGRAPH_H_INCLUDED__   // if localgraph.h hasn't been included yet...
#define __LOCALGRAPH_H_INCLUDED__

class LocalNode;

#include <cstring>
#include <map>
#include <set>
#include "interval.h"
#include "path.h"
#include "localnode.h"

using namespace std;

class LocalGraph {
  public:
    map<uint32_t, LocalNode*> nodes; // representing nodes in graph
    LocalGraph() {}
    ~LocalGraph();
    bool operator == (const LocalGraph& y) const;
    void add_node (const uint32_t& id, const string& seq, Interval pos);
    void add_edge (const uint32_t&, const uint32_t&);
    void write_gfa (string);
    set<Path> walk(uint32_t, uint32_t, uint32_t);
};

#endif
