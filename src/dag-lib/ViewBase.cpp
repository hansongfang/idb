#include "ViewBase.h"
#include "Log.h"

void SFViewBase::initGraph()
{
    int nNodes = this->getNumberOfNodes();
    auto edges = this->getNodeAdjacencies();

    _pGraph = new UnDirectedGraph(edges.begin(), edges.end(), nNodes);
    _vp_index = get(vp_index_t(), *_pGraph);

    // add Index to each node, because BGL will change the index number after removing vertex
    for (auto i = 0; i < nNodes; ++i) {
        // fill index to the BGL adjacent list
        boost::put(_vp_index, i, i);
    }
}

int SFViewBase::getVertexIndex(SFViewBase::vertex_t v) const
{
    return boost::get(_vp_index, v);
}

SFViewBase::UnDirectedGraph &SFViewBase::getG() const
{
    return *_pGraph;
}

SFViewBase::vertex_t SFViewBase::getVertex_t(int id) const
{
    vertex_iterator first, last;
    boost::tie(first, last) = boost::vertices( getG() );
    while(first != last) {
        int value = getVertexIndex( *first );
        if (id == value)
            return *first;
        ++first;
    }
    gLogError << "Can't find id, this is not expected, please investigate!";
    exit(-1);
}

void SFViewBase::printEdges() const
{
    //gLogTrace << "Print edges of the view mesh";
    edge_iterator first, last;
    boost::tie(first, last) = boost::edges( getG() );
    int cnt = 0;
    while(first != last) {
        auto src = source(*first, getG());
        auto dst = target(*first, getG());
        int src_value = getVertexIndex( src );
        int targ_value = getVertexIndex( dst );
        gLogInfo << src_value << "(" << src << ") --> " << targ_value << "(" << dst <<")";
        ++first;
        ++cnt;
    }
    gLogInfo << "Total edges of the view mesh: " << cnt;
}

int SFViewBase::getNumberOfNodes()
{
    return (int)_viewNodes.size();
}

SFViewBase::SFViewBase()
    :_pGraph(0)
{
}

SFViewBase::~SFViewBase()
{
    if (_pGraph)
        delete _pGraph;
}

vector<int> SFViewBase::getNeighboor(int id) const
{
    auto v = getVertex_t(id);
    auto g = getG();
    auto nb = boost::adjacent_vertices(v, g);
    vector<int> res;
    for (auto it = nb.first; it != nb.second; ++it) {
        res.push_back( getVertexIndex(*it) );
    }
    return res;
}

set<Edge> SFViewBase::getViewEdges() const
{
    set<Edge> res;
    edge_iterator first, last;
    boost::tie(first, last) = boost::edges( getG() );
    while(first != last) {
        int v1 = getVertexIndex( source(*first, getG()) );
        int v2 = getVertexIndex( target(*first, getG()) );
        Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
        res.insert(e);
        ++first;
    }
    return res;
}

set<Edge> SFViewBase::getPartialEdge(const std::set<int> &nodeIds) const
{
    set<Edge> res;
    // for every node, find its related nodes
    for(auto it = nodeIds.cbegin();it!= nodeIds.cend();it++){
        auto v = getVertex_t(*it);
        auto g = getG();
        auto nb = boost::adjacent_vertices(v,g);
        for(auto innerIt = nb.first;innerIt != nb.second;++innerIt){
            auto vertexId = getVertexIndex(*innerIt);
            // for every related nodes, check it it's in the nodeIds
			if (nodeIds.find(vertexId) != nodeIds.end()) {
				Edge e = *it < vertexId ? Edge(*it, vertexId) : Edge(vertexId, *it);
				res.insert(e);
			}
            
        }
    }
    return res;
}

const ViewNode *SFViewBase::getNode(int id) const
{
    return _viewNodes[id];
}

const vector<ViewNode *> &SFViewBase::getNodes() const
{
    return _viewNodes;
}
