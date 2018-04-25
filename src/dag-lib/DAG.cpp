#include "DAG.h"
#include "Log.h"
#include <boost/filesystem.hpp>
#include <boost/serialization/serialization.hpp>
#include<boost/serialization/unordered_map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/archive/text_oarchive.hpp>
#include <fstream>

namespace fs = boost::filesystem;

DAG::DAG(int id, int numVertices)
    :_id(id)
    , _dagGraph(numVertices)
{
}

DAG::DAG(int id, string cacheFolder)
    :_id(id)
{
	this->loadDAG(cacheFolder);
}

DAG::DAG(const DAG &other)
    :_id(other._id)
    ,_dagGraph(other._dagGraph)
{
}

DAG::~DAG()
{
}

bool DAG::find(int e1, int e2) const
{
    return _dagGraph.find(e1,e2);
}

bool DAG::addEdge(int e1, int e2, int count)
{
    return _dagGraph.addEdge(e1,e2,count);
}

bool DAG::rmEdge(int e1, int e2,int count)
{
    return _dagGraph.rmEdge(e1,e2,count);
}

bool DAG::updateDAG(const DAG &dag)
{
    return _dagGraph.updateGraph(dag.getDagGraph());
}

bool DAG::rmUpdateDag(const DAG &dag)
{
    return _dagGraph.rmUpdateGraph(dag.getDagGraph());
}

int DAG::getId() const
{
    return _id;
}

const DagGraph& DAG::getDagGraph() const
{
    return _dagGraph;
}

bool DAG::detectCycle() const
{
    return _dagGraph.detectCycle();
}

std::unordered_set<int> DAG::getVertices() const
{
    return _dagGraph.getVertices();
}

unsigned int DAG::getEdgeSize() const
{
    return _dagGraph.getEdgeSize();
}

unsigned int DAG::getNodeSize()const
{
    return _dagGraph.getNodeSize();
}

// traversing all edges in _dagGraph
unsigned int DAG::shareSize(const DagGraph& dagGraph) const
{
    return _dagGraph.shareSize(dagGraph);
}

string DAG::getFilePath(string rootFolder) const
{
    //filename is the viewpoint id
    fs::path dir(rootFolder);
    fs::path f( std::to_string(_id) + ".dat" );
    fs::path filepath = dir / f;
    return filepath.string();
}

void DAG::saveDAG(string rootFolder) const
{
    string filename = getFilePath(rootFolder);
    _dagGraph.saveGraph(filename);
}

void DAG::loadDAG(string rootFolder)
{
    string filename = getFilePath(rootFolder);
    _dagGraph.loadGraph(filename);
}
