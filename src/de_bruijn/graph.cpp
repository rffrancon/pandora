#include <deque>
#include <limits>
#include <fstream>
#include <unordered_set>
#include <set>
#include <deque>
#include <iostream>
#include <memory>
#include <cassert>
#include "de_bruijn/graph.h"
#include "de_bruijn/node.h"
#include "noise_filtering.h"
#include "utils.h"

#define assert_msg(x) !(std::cerr << "Assertion failed: " << x << std::endl)

using namespace debruijn;

Graph::Graph(uint8_t s) : next_id(0), size(s) {
    nodes.reserve(200000);
};


Graph::~Graph()
{
    nodes.clear();
}

// add a node in dbg corresponding to a fixed size deque of pangenome graph
// node/orientation ids and labelled with the read_ids which cover it
NodePtr Graph::add_node (const deque<uint16_t>& node_ids, uint32_t read_id)
{
    assert(node_ids.size() == size);

    if (node_hash.find(node_ids) != node_hash.end()) {
        nodes[node_hash[node_ids]]->read_ids.insert(read_id);
        return nodes[node_hash[node_ids]];
    } else if (node_hash.find(rc_hashed_node_ids(node_ids)) != node_hash.end()) {
        nodes[node_hash[rc_hashed_node_ids(node_ids)]]->read_ids.insert(read_id);
        return nodes[node_hash[rc_hashed_node_ids(node_ids)]];
    }

    NodePtr n;
    n = make_shared<Node>(next_id, node_ids, read_id);
    nodes[next_id] = n;
    node_hash[node_ids] = next_id;

    if (next_id%1000==0)
    {
        cout << "added node " << next_id << endl;
    }

    next_id++;
    assert(next_id < numeric_limits<uint32_t>::max()||assert_msg("WARNING, reached max de bruijn graph node size"));
    return n;
}

// add an undirected edge
void Graph::add_edge (NodePtr from, NodePtr to)
{
    deque<uint16_t> rc_from = rc_hashed_node_ids(from->hashed_node_ids);
    deque<uint16_t> rc_to = rc_hashed_node_ids(to->hashed_node_ids);
    bool added_edge = false;

    if (overlap_forwards(from->hashed_node_ids, to->hashed_node_ids)
            or overlap_forwards(from->hashed_node_ids, rc_to)
            //or overlap_forwards(rc_from, to->hashed_node_ids)
            //or overlap_forwards(rc_from, rc_to)
            )
    {
        if (from->out_nodes.find(to->id) == from->out_nodes.end())
        {
            from->out_nodes.insert(to->id);
            to->in_nodes.insert(from->id);
        }
        added_edge = true;
    }
    if (overlap_backwards(from->hashed_node_ids, to->hashed_node_ids)
        or overlap_backwards(from->hashed_node_ids, rc_to)
        //or overlap_backwards(rc_from, to->hashed_node_ids)
        //or overlap_backwards(rc_from, rc_to)
        )
    {
        if (from->in_nodes.find(to->id) == from->in_nodes.end())
        {
            from->in_nodes.insert(to->id);
            to->out_nodes.insert(from->id);
        }
        added_edge = true;
    }
    assert(added_edge or assert_msg("did not add edge from " << *from << " to " << *to));
}

// remove de bruijn node with id given
void Graph::remove_node(const uint32_t dbg_node_id)
{
    //cout << "remove node " << dbg_node_id << endl;
    auto it = nodes.find(dbg_node_id);
    if ( it != nodes.end())
    {
        // remove this node from lists of out nodes from other graph nodes
        for (auto n : it->second->out_nodes)
        {
            nodes[n]->in_nodes.erase(dbg_node_id);
        }
        for (auto n : it->second->in_nodes)
        {
            nodes[n]->out_nodes.erase(dbg_node_id);
        }

        // and remove from nodes
        nodes.erase(dbg_node_id);
    }
}

// remove read from de bruijn node
void Graph::remove_read_from_node(const uint32_t read_id, const uint32_t dbg_node_id)
{
    //cout << "remove read " << (uint)read_id << " from node " << (uint)dbg_node_id << endl;
    auto it = nodes.find(dbg_node_id);
    bool found_read_intersect;
    if ( it != nodes.end())
    {
        //cout << "found node " << *(it->second) << endl;
        auto rit = it->second->read_ids.find(read_id);
        if (rit != it->second->read_ids.end())
        {
            //cout << "found read id on node" << endl;
            it->second->read_ids.erase(rit);
            //cout << "removed read id" << endl;

            // if there are no more reads covering it, remove the node

            if (it->second->read_ids.empty())
            {
                //cout << "no more reads, so remove node" << endl;
                remove_node(dbg_node_id);
            } else {
                // otherwise, remove any outnodes which no longer share a read
                //cout << "remove outnodes which no longer share a read" << endl;
                for (unordered_set<uint32_t>::iterator nit = it->second->out_nodes.begin();
                     nit != it->second->out_nodes.end();)
                {
                    //cout << "out node " << *nit << endl;
                    found_read_intersect = false;
                    for (auto r : it->second->read_ids) {
                        if (nodes[*nit]->read_ids.find(r) != nodes[*nit]->read_ids.end()) {
                            found_read_intersect = true;
                            //cout << " shares a read" << endl;
                            break;
                        }
                    }
                    if (found_read_intersect == false)
                    {
                        //cout << " does not share a read" << endl;
                        nodes[*nit]->in_nodes.erase(dbg_node_id);
                        nit = it->second->out_nodes.erase(nit);
                        //cout << "removed" << endl;
                    } else {
                        nit++;
                    }
                }
                for (unordered_set<uint32_t>::iterator nit = it->second->in_nodes.begin();
                     nit != it->second->in_nodes.end();)
                {
                    //cout << "out node " << *nit << endl;
                    found_read_intersect = false;
                    for (auto r : it->second->read_ids) {
                        if (nodes[*nit]->read_ids.find(r) != nodes[*nit]->read_ids.end()) {
                            found_read_intersect = true;
                            //cout << " shares a read" << endl;
                            break;
                        }
                    }
                    if (found_read_intersect == false)
                    {
                        //cout << " does not share a read" << endl;
                        nodes[*nit]->out_nodes.erase(dbg_node_id);
                        nit = it->second->in_nodes.erase(nit);
                        //cout << "removed" << endl;
                    } else {
                        nit++;
                    }
                }
            }
        }
    }
}

// get the dbg node ids corresponding to leaves
unordered_set<uint32_t> Graph::get_leaves(uint16_t covg_thresh)
{
    unordered_set<uint32_t> s;
    for (auto c : nodes)
    {
        if (c.second->read_ids.size() > covg_thresh) {
            continue;
        } else if (c.second->out_nodes.size() + c.second->in_nodes.size() <= 1) {
            s.insert(c.second->id);
        }
    }
    return s;
}

// get deques of dbg node ids corresponding to maximal non-branching paths in dbg
set<deque<uint32_t>> Graph::get_unitigs() {
    set<deque<uint32_t>> all_tigs;
    set<uint32_t> seen;

    for (auto node_entry : nodes) {
        const auto &id = node_entry.first;
        assert(id <= next_id);

        const auto &node_ptr = node_entry.second;
        assert(node_ptr != nullptr);

        bool node_seen = seen.find(id) != seen.end();
        bool at_branch = node_ptr->out_nodes.size() > 2;
        if (node_seen or at_branch)
            continue;

        deque<uint32_t> tig = {id};
        extend_unitig(tig);
        for (auto other_id: tig)
            seen.insert(other_id);
        all_tigs.insert(tig);
    }
    return all_tigs;
}

// extend a dbg path on either end to a branch point
void Graph::extend_unitig(deque<uint32_t>& tig)
{
    cout << "extend tig ";
    for (auto n : tig)
    {
	cout << n << " ";
    }
    cout << endl;
    bool tig_is_empty = (tig.size() == 0);
    bool node_is_isolated = (tig.size() == 1 and nodes[tig.back()]->out_nodes.size()+nodes[tig.back()]->in_nodes.size() == 0);
    if (tig_is_empty or node_is_isolated)
        return;

    while (nodes[tig.back()]->out_nodes.size() == 1
	    and nodes[tig.back()]->in_nodes.size() == 1
            and (tig.size() == 1 or tig.back()!=tig.front()))
    {
        tig.push_back(*nodes[tig.back()]->out_nodes.begin());
	cout << ".";
    }
    while (nodes[tig.front()]->in_nodes.size() == 1
	   and nodes[tig.front()]->out_nodes.size() == 1
           and (tig.size() == 1 or tig.back()!=tig.front()))
    {
        tig.push_front(*nodes[tig.front()]->in_nodes.begin());
	cout << ",";
    }
    cout << endl;
    if (tig.size() > 1 and tig.back() == tig.front())
        tig.pop_back();
    cout << "got tig ";
    for (auto n : tig)
    {
        cout << n << " ";
    }
    cout << endl;
}

bool Graph::operator == (const Graph& y) const
{
    // want the graphs to have the same nodes, even if
    // the ids given them is different.
    if (nodes.size() != y.nodes.size())
    {
        cout << "different num nodes" << endl;
	    return false;
    }

    for (const auto t : nodes) {
        bool out_found, found = false;
        for (const auto s : y.nodes) {
            if (*t.second == *s.second) {
                found = true;

                // also check the outnodes are the same
                if (t.second->out_nodes.size() != s.second->out_nodes.size())
                {
                    cout << "node has different number of outnodes" << endl;
                    return false;
                }
                for (const auto i : t.second->out_nodes) {
                    out_found = false;
                    for (const auto j : s.second->out_nodes) {
                        if (*nodes.at(i) == *y.nodes.at(j))
                        {
                            out_found = true;
                            break;
                        }
                    }
                    if (out_found == false)
                    {
                        cout << "did not find outnode" << endl;
                        return false;
                    }
                }

                break;
            }
        }
        if (found == false) {
            cout << "did not find node " << t.first << " " << *t.second << endl;
            return false;
        }
    }
    // nodes can't be equal within a graph so don't need to check vice versa
    return true;
}

bool Graph::operator!=(const Graph &y) const {
    return !(*this == y);
}

namespace debruijn {
    std::ostream &operator<<(std::ostream &out, const Graph &m) {
        for (const auto &n : m.nodes) {
            out << n.first << ": " << *(n.second) << endl;
            for (const auto &o : n.second->out_nodes) {
                out << n.first << " -> " << o << endl;
            }
        }
        return out;
    }
}
