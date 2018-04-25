#ifndef SFGRAPHCOMMON_H
#define SFGRAPHCOMMON_H

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <deque>
#include <set>
#include "DLL.h"
#include "SFVector.h"
#include "GraphCommon.h"

typedef std::pair<int, int> Edge;
typedef std::unordered_map<int, std::unordered_map<int,int> > DagMap;

/*
 * Calculate and save DAG for a single view
 */

class TRIDAG_LIB DagGraph
{
private:
    enum default_color_type{ white_color, gray_color, black_color };

protected:
    bool has_cycle_dfs(int vId, default_color_type* color) const;

public:
    DagMap                              _dagMap;
    unsigned int                        _edgeSize;
    unsigned int                        _numVertices;

    DagGraph();
    DagGraph(int nVertices);

    DagGraph(const DagGraph& dagGraph);
    bool find(int e1, int e2) const;
    bool addEdge(int e1, int e2, int count = 1);
    bool rmEdge(int e1, int e2, int count = 1);
    bool updateGraph(const DagGraph& dagGraph);
    bool rmUpdateGraph(const DagGraph& dagGraph);
	void minusInter(const DagGraph& keep, const DagGraph& rm);
    DagMap getDagMap();

    //attr
     bool detectCycle() const;

     //io
     void saveGraph(std::string filename) const;
     void loadGraph(std::string filename);

     unsigned int getEdgeSize() const;
     unsigned int getNodeSize() const;
     unsigned int shareSize(const DagGraph& dag) const;
     std::unordered_set<int> getVertices() const;
};

#endif // SFGRAPHCOMMON_H
