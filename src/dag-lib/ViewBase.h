#ifndef SFViewBase_H
#define SFViewBase_H

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/topological_sort.hpp"
#include "boost/graph/exception.hpp"
#include "GraphCommon.h"
#include "ModelBase.h"
#include "DLL.h"
#include "DagGraph.h"

using namespace boost;

/*
 * Base class for representative views
 *
 */
class TRIDAG_LIB SFViewBase
{
    struct vp_index_t {
      typedef vertex_property_tag kind;
    };
    typedef property<vp_index_t, int> ViewPointIndexProperty;
    typedef adjacency_list<setS, vecS, undirectedS, ViewPointIndexProperty> UnDirectedGraph;
    typedef graph_traits<UnDirectedGraph>::edge_iterator edge_iterator;
    typedef graph_traits<UnDirectedGraph>::vertex_iterator vertex_iterator;
    typedef boost::graph_traits<UnDirectedGraph>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<UnDirectedGraph>::edge_descriptor edge_t;

    property_map<UnDirectedGraph, vp_index_t>::type _vp_index;
    UnDirectedGraph *_pGraph;

protected:
    vector<ViewNode*>             _viewNodes;

    void initGraph();// need to be called after initViewNodes()
    int getVertexIndex(vertex_t v) const;
    UnDirectedGraph &getG() const;
    vertex_t getVertex_t(int id) const;

public:
    SFViewBase();
    ~SFViewBase();

    vector<int> getNeighboor(int id) const;
    set<Edge> getViewEdges() const;
    set<Edge> getPartialEdge(const std::set<int>& nodeIds) const;
    void printEdges() const;
public:
    virtual set<Edge> getNodeAdjacencies() const = 0;
    virtual void initViewNodes() = 0;
    int getNumberOfNodes();
    const ViewNode *getNode(int id) const;
    const vector<ViewNode *> &getNodes() const;
};

#endif // SFViewBase_H
