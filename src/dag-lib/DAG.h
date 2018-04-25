#ifndef DAG_H
#define DAG_H

#include "DagGraph.h"
#include "GraphCommon.h"
#include "SFVector.h"
#include <stdexcept>
#include <map>
#include <deque>
/*
 * Calculate and save DAG for a single view
 */
class TRIDAG_LIB DAG
{
    int                             _id;
    DagGraph                        _dagGraph;
protected:
    string getFilePath(string rootFolder) const;
public:
    DAG(int id = -1,int numVertices = 0);
    DAG(int id, string cacheFolder);
    DAG(const DAG &other);
    ~DAG();

    bool find(int e1, int e2) const; // find edge
    bool addEdge(int e1, int e2, int count = 1);
    bool rmEdge(int e1, int e2, int count = 1);
    bool updateDAG(const DAG &dag);
    bool rmUpdateDag(const DAG &dag);

    bool detectCycle() const;

    const DagGraph& getDagGraph() const;
	unsigned int getEdgeSize() const;
    unsigned int getNodeSize() const;
    std::unordered_set<int> getVertices() const;
    unsigned int shareSize(const DagGraph& dagGraph) const;

    // io
    int getId() const;
    void saveDAG(string rootFolder) const;
    void loadDAG(string rootFolder);
};

#endif // DAG_H
