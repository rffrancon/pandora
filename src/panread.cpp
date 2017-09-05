#include <iostream>
#include <string>
#include <fstream>
#include "panread.h"
#include "panedge.h"
#include "pannode.h"
#include "minihits.h"

using namespace std;

PanRead::PanRead (const uint32_t i): id(i) {}

void PanRead::add_hits(const uint32_t node_id, const set<MinimizerHit*, pComp>& c)
{
    /*map<uint32_t, std::set<MinimizerHit*, pComp>>::iterator it=hits.find(node_id);
    if(it==hits.end())
    {
	hits[node_id] = {};
    }*/
    hits[node_id].insert(c.begin(), c.end());
}

bool PanRead::operator == (const PanRead& y) const {
    if (id != y.id) {return false;}
    /*if (edges.size() != y.edges.size()) {return false;}
    for (uint i=0; i!=edges.size(); ++i)
    {
	if (edges[i] != y.edges[i]) {return false;}
    }*/
	
    return true;
}

bool PanRead::operator != (const PanRead& y) const {
    return !(*this == y);
}

bool PanRead::operator < (const PanRead& y) const {
    return (id < y.id);
}

std::ostream& operator<< (std::ostream & out, PanRead const& r) {
    out << r.id << "\t";
    for (uint i=0; i!=r.edges.size(); ++i)
    {
	out << *r.edges[i] << ", ";
    }
    return out ;
}