#ifndef __PANEDGE_H_INCLUDED__   // if panedge.h hasn't been included yet...
#define __PANEDGE_H_INCLUDED__

class PanNode;

class PanEdge {
  public:
    const PanNode* from;
    const PanNode* to;
    const uint orientation;	// 0 --
				// 1 +-
				// 2 -+
				// 3 ++
    uint32_t covg;

    PanEdge(const PanNode*, const PanNode*, uint);

    bool operator == (const PanEdge& y) const;
    friend std::ostream& operator<< (std::ostream& out, const PanEdge& e);
};

#endif
