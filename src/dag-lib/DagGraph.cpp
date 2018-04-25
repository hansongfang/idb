#include "DagGraph.h"
#include "Log.h"
#include <boost/serialization/serialization.hpp>
#include<boost/serialization/unordered_map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/archive/text_oarchive.hpp>
#include <fstream>

float loadFactor = 16.0;
class viewSort{
public:
    float dist;
    int id;
};
bool sortView(const viewSort& a,const viewSort& b )
{
    return a.dist < b.dist;
}

DagGraph::DagGraph()
    :_edgeSize(0)
    ,_numVertices(0)
{
    _dagMap.clear();
    _dagMap.max_load_factor(loadFactor);
}
DagGraph::DagGraph(int nVertices)
    :_edgeSize(0)
    ,_numVertices(nVertices)
{
    _dagMap.max_load_factor(loadFactor);
}

DagGraph::DagGraph(const DagGraph& dagGraph)
    :_edgeSize(dagGraph._edgeSize)
    ,_numVertices(dagGraph._numVertices)
{
    _dagMap.max_load_factor(loadFactor);
    //gLogInfo<<_dagMap.max_load_factor();
    auto & map = dagGraph._dagMap;
    auto size1 = map.size();
    _dagMap.reserve((int)size1/(int)loadFactor +1);
    for(auto it = map.cbegin();it!= map.cend();it++){
        auto size2 = map.at(it->first).size();
        bool hasReserve = false;
        for(auto innerIt = map.at(it->first).cbegin();innerIt != map.at(it->first).cend();innerIt++){
            this->addEdge(it->first,innerIt->first,innerIt->second);
            if(!hasReserve){
               _dagMap.at(it->first).max_load_factor(loadFactor);
               _dagMap.at(it->first).reserve((int)size2/(int)loadFactor +1);
               hasReserve = true;
            }
        }
    }
}

bool DagGraph::find(int e1, int e2) const
{
    if (_dagMap.find(e1) == _dagMap.end())
        return false;
    if (_dagMap.at(e1).find(e2) == _dagMap.at(e1).end())
        return false;
    return true;
}

bool DagGraph::addEdge(int e1, int e2, int count)
{
    try{
    if (this->find(e1, e2)){
        _dagMap[e1][e2] += count;
        return true;
    }
    // when it's cluster add to cluster it's not adding by 1
    _dagMap[e1][e2] = count;
    //_dagMap.at(e1).max_load_factor(loadFactor);
    _edgeSize++;
    return true;
    }
    catch(std::exception &ex)
    {
        gLogInfo<<e1<<" "<<e2;
        gLogError<<"fail in addEdge "<<ex.what();
    }
    return true;
}

bool DagGraph::rmEdge(int e1, int e2, int count)
{
    if (!this->find(e1, e2)){
        gLogError<< "Not find!";
        return false;
    }
    // if no error ,you could delete this
    if (_dagMap[e1][e2] < count){
        gLogError << "edge count less than rm, please check!";
        return false;
    }

    _dagMap[e1][e2] -= count;
    if (_dagMap[e1][e2] == 0){
        _dagMap[e1].erase(e2);
        if (_dagMap[e1].empty())
            _dagMap.erase(e1);
        _edgeSize--;
    }
    return true;
}

bool DagGraph::updateGraph(const DagGraph &dagGraph)
{
    const auto& dagMap = dagGraph._dagMap;
    for(auto it = dagMap.cbegin();it!= dagMap.cend(); it++){
        for(auto it2 = it->second.cbegin();it2!= it->second.cend();it2++){
            this->addEdge(it->first,it2->first, it2->second);
        }
    }
    return !this->detectCycle();
}

bool DagGraph::rmUpdateGraph(const DagGraph &dagGraph)
{
    const auto& dagMap = dagGraph._dagMap;
    for(auto it = dagMap.cbegin();it!= dagMap.cend();it++){
        for(auto innerIt =it->second.cbegin();innerIt != it->second.cend();innerIt++){
            if(!this->rmEdge(it->first,innerIt->first,innerIt->second)){
                gLogInfo<<"remove edge wrong: "<<it->first<<" -- "<<innerIt->first<<" count "<<innerIt->second;
                return false;
            }

        }
    }
    return true;
}

void DagGraph::minusInter(const DagGraph& keepGraph, const DagGraph& rmGraph)
{
	const auto& keep = keepGraph._dagMap;
	const auto& rm = rmGraph._dagMap;
	for (auto it = keep.cbegin(); it != keep.cend(); it++) {
		for (auto innerIt = keep.at(it->first).cbegin(); innerIt != keep.at(it->first).cend(); innerIt++) {
			// find if this edge is in or not
			if (rm.find(it->first) == rm.end()) {
				this->addEdge(it->first, innerIt->first);
			}
			else if (rm.at(it->first).find(innerIt->first) == rm.at(it->first).end()) {
				this->addEdge(it->first, innerIt->first);
			}
		}
	}
}

unsigned int DagGraph::shareSize(const DagGraph& dag) const
{
    const auto& dagMap = dag._dagMap;
    unsigned int shareSize = 0;
    try {
        for (auto it = _dagMap.cbegin(); it != _dagMap.cend(); it++) {
            if (dagMap.find(it->first) == dagMap.end())
                continue;
            for (auto innerIt = it->second.cbegin(); innerIt != it->second.cend(); innerIt++) {
                // at might have some problem
                if (dagMap.at(it->first).find(innerIt->first) != dagMap.at(it->first).end())
                    shareSize++;
            }
        }
    }
    catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range errorL " << oor.what() << '\n';
        std::cout << "please check" << std::endl;
    }

    return shareSize;
}

// search by depth first
// gray color meet graycolor there is a cycle
// gray color means visiting
// white color means never visit before
// black color means finish visiting
bool DagGraph::has_cycle_dfs(int vId, default_color_type* color) const
{
    color[vId] = gray_color;
    try{
        if (_dagMap.find(vId) == _dagMap.end()){
            color[vId] = black_color;
            return false;
        }
        auto adjVertices = _dagMap.at(vId);
        for (auto it = adjVertices.cbegin(); it != adjVertices.cend(); it++){
            if (color[it->first] == white_color){
                if (has_cycle_dfs(it->first, color))
                    return true;
            }
            else if (color[it->first] == gray_color)
                return true;
        }
        color[vId] = black_color;
        return false;
    }
    catch (const std::out_of_range& oor){
        std::cerr << "Out of Range errorL " << oor.what() << '\n';
        std::cout << "please check"<<std::endl;
    }
    return false;
}

bool DagGraph::detectCycle() const
{
    // iterate each vertex
    // dfs implement
    std::vector<default_color_type> color(_numVertices, white_color);
    for(auto it = _dagMap.cbegin();it!=_dagMap.cend();it++){
        if(color[it->first] == white_color){
            if (has_cycle_dfs(it->first, &color[0]))
               return true;
        }
    }
    return false;
}

DagMap DagGraph::getDagMap()
{
    return _dagMap;
}


unsigned int DagGraph::getEdgeSize() const
{
    return _edgeSize;
}

unsigned int DagGraph::getNodeSize() const
{
    std::set<int> vertices;
    for(auto it = _dagMap.cbegin();it != _dagMap.cend();it++){
        vertices.insert(it->first);
        for(auto innerIt = _dagMap.at(it->first).cbegin();innerIt != _dagMap.at(it->first).cend();innerIt++){
            vertices.insert(innerIt->first);
        }
    }
    gLogInfo<<"Dag have "<<vertices.size()<<" all vertices";
   return vertices.size();
}

std::unordered_set<int> DagGraph::getVertices() const
{
    std::unordered_set<int> vertices;
    for(auto it = _dagMap.cbegin();it != _dagMap.cend();it++){
        vertices.insert(it->first);
        for(auto innerIt = _dagMap.at(it->first).cbegin();innerIt != _dagMap.at(it->first).cend();innerIt++){
            vertices.insert(innerIt->first);
        }
    }
    gLogDebug<<"Dag have "<< _dagMap.size()<<"out vertices";
    gLogDebug<<"Dag have "<<vertices.size()<<" all vertices";
   return vertices;
}

void DagGraph::saveGraph(std::string filename) const
{
    try{
        std::ofstream ofs(filename);
        boost::archive::text_oarchive oa(ofs);
        oa << _numVertices;
        oa << _edgeSize;
        oa <<_dagMap;
        ofs.close();
    }
    catch(std::exception &ex){
        gLogError << "Failed to serialize DAG " << filename << " - " << ex.what();
    }
}

void DagGraph::loadGraph(std::string filename)
{
    std::ifstream ifs(filename);
    try{
        boost::archive::text_iarchive ia(ifs);
        ia >> _numVertices;
        ia >> _edgeSize;
        ia >> _dagMap;
        ifs.close();
    }
    catch(std::exception &ex){
        gLogError << "Failed to load DAG " << filename << " - " << ex.what();
    }
}
