#include "DagMerger.h"
#include "Log.h"
#include <unordered_set>
#include <boost/serialization/serialization.hpp>
#include<boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/archive/text_oarchive.hpp>

DAGMerger::DAGMerger(int nNodes,DAGCenter* pDagCenter, SFViewBase *pViewMesh, int metricMode)
    :_metricMode(metricMode)
    ,_pDagCenter(pDagCenter)
{
    gLogInfo<<"init cluster DAG";
    for(auto id=0; id<nNodes; id++){
       auto tempDag = _pDagCenter->getDag(id);
       _clusterDags[id] = new DAG(*tempDag);
    }
    gLogInfo<<"init assignments, clusterAssign,cluster neighbour";
    init(nNodes,pViewMesh);
}

DAGMerger::~DAGMerger()
{
    // delete the clusterDags;
    for(auto it = _clusterDags.begin();it != _clusterDags.end();it++)
        delete it->second;
}

void DAGMerger::init(int nNodes, SFViewBase *pViewMesh)
{
    for(auto nodeId =0;nodeId<nNodes;nodeId++){
        _assignments[nodeId] = nodeId;
        _clusterAssigns[nodeId] = vector<int>{nodeId};
    }
    this->initEdges(pViewMesh->getViewEdges());
}

void DAGMerger::initEdges(const set<Edge> &edges)
{
    // Create new priority
    _prioritylist.clear();

    // Assign new edges with original priority
    for (auto edge: edges) {
        this->updatePriority( edge );
        this->_addClusterNb(edge.first,edge.second);
        this->_addClusterNb(edge.second,edge.first);
    }
    _edges = _clusterNbs;
}


bool DAGMerger::hasQueue() const
{
 return 0<_prioritylist.size();
}

void DAGMerger::mergeTop()
{
    gLogDebug << "########Start Merging";
    // Get top edge
    auto edge = getTopEdge();
    // if have clusterDags
    // Get 2 dags
    auto dag1 = getClusterDag( edge.first );
    auto dag2 = getClusterDag( edge.second );

    // Try merge DAGs
    // if merge successful, push to clusterDag and change its clusterId to this
    auto dag3 = mergeDAGs( dag1, dag2 );

    if (dag3) {
        gLogDebug << "Edge:"<< edge.first << " -- " << edge.second;

        // if doing incrementally , collopseEdge first
        this->collopseEdge(edge);

        // update
        int clusterId1 = edge.first;
        int clusterId2 = edge.second;
        gLogDebug<<"clusterId "<<clusterId1<<" "<<clusterId2;
        auto tempFind = _clusterDags.find(clusterId2);
        delete tempFind->second;
        _clusterDags[clusterId2] = dag3;
        _clusterAssigns[clusterId2].insert(_clusterAssigns[clusterId2].end(), _clusterAssigns[clusterId1].begin(),_clusterAssigns[clusterId1].end());

        // update assignments
        const auto& clusterNodes = _clusterAssigns.at(clusterId1);
        for(auto it = clusterNodes.begin();it!= clusterNodes.end();it++){
            _assignments[*it] = clusterId2;
        }

        tempFind = _clusterDags.find(clusterId1);
        delete tempFind->second;
        _clusterDags.erase(clusterId1);
        _clusterAssigns.erase(clusterId1);

        gLogInfo << "Merged edge: " << edge.first << " -- " << edge.second;
        gLogDebug<<"clusterDag size: "<<_clusterDags.size();

    }
    else {
        // can't merge
        gLogDebug << "Can't merge edge: " << edge.first << " -- " << edge.second;
    }
    this->popEdge(edge);
}

void DAGMerger::updatePriority(const Edge &edge)
{
    _prioritylist[edge] = this->calculatePriority(edge.first, edge.second);
    gLogDebug << "Updated Priority: "<< edge.first << " -- " << edge.second << " = " << _prioritylist[edge];
}

Edge DAGMerger::getTopEdge() const
{
    //float priority = -1;
    float priority = numeric_limits<int>::min();
    Edge res;
    for( auto element: _prioritylist) {
        if (priority < element.second) {
            priority = element.second;
            res = element.first;
        }
    }
    gLogDebug << "Find edge: " << res.first << " -- " << res.second << " with priority: " << priority;
    return res;
}

void DAGMerger::popEdge(Edge edge)
{
    gLogDebug << "Remove edge: " << edge.first << " -- " << edge.second;
    _prioritylist.erase( edge );
}

const DAG* DAGMerger::getDag(int id) const
{
 return _pDagCenter->getDag(id);
}

const DAG* DAGMerger::getClusterDag(int id) const
{
    return _clusterDags.at(id);
}

DAG* DAGMerger::mergeDAGs(const DAG *dag1, const DAG *dag2) const
{
    // if merge fail, return 0
    // if merge good, return DAG with same id as dag2
    DAG *dag3 = new DAG(*dag2);
    if ( dag3->updateDAG(*dag1) ){
        gLogDebug << "Find dag candidates for merge " << dag1->getId() << " - " << dag2->getId();
        return dag3;
    }
    else {
        delete dag3;
        return 0;
    }
}

void DAGMerger::_addClusterNb(const int& id1, const int& id2)
{
    if(_clusterNbs.find(id1) == _clusterNbs.end())
        _clusterNbs[id1] = std::unordered_set<int>{id2};
    else
        _clusterNbs.at(id1).insert(id2);
}

// merge two cluster
// update proritylist and cluster neighbor relation
void DAGMerger::collopseEdge(const Edge &edge)
{
    //1. remove all edges related to edge.first
    //2. update all edges related to edge.second
    gLogDebug << "Collapsing Edge: "<< edge.first << " -- " << edge.second;
    int remove = edge.first;
    int keep = edge.second;

    _clusterNbs.at(remove).erase(keep);
    _clusterNbs.at(keep).erase(remove);
    _prioritylist.erase(edge);

    //doing incrementally
    //update priority before update cluster c2 into cluster c1c2
    const auto& rmDagMap = this->getClusterDag(remove)->getDagGraph();
    const auto& kpDagMap = this->getClusterDag(keep)->getDagGraph();
    DagGraph smallKp;
	smallKp.minusInter(kpDagMap,rmDagMap);
    DagGraph smallRm;
    smallRm.minusInter(rmDagMap,kpDagMap);

    // sharesize(C,  A ∪ B ) = sharesize(C, A) + sharesize(C, B -A)
    for(auto it = _clusterNbs.at(remove).begin();it!= _clusterNbs.at(remove).end();++it){
        Edge old = remove < *it? Edge(remove,*it):Edge(*it, remove);
        float oldPrio = _prioritylist[old];
        _prioritylist.erase(old);
        Edge newE = keep < *it? Edge(keep, *it):Edge(*it, keep);
        if(_metricMode == 1)
            _prioritylist[newE] = oldPrio + this->getClusterDag(*it)->shareSize(smallKp);
        else {
            gLogInfo<<"shouldn't update priority incrementally";
        }
    }

    for(auto it = _clusterNbs.at(keep).begin();it!= _clusterNbs.at(keep).end();++it){
        //update priority have been calculated
        if (_clusterNbs.at(remove).find(*it) != _clusterNbs.at(remove).end())
            continue;
        Edge old = keep < *it? Edge(keep, *it):Edge(*it, keep);
        _prioritylist[old] += this->getClusterDag(*it)->shareSize(smallRm);
    }
	//_clusterNbs don't have keep
    for(auto it = _clusterNbs.at(remove).begin();it !=_clusterNbs.at(remove).end() ; it++){
       _clusterNbs.at(*it).erase(remove);
       _clusterNbs.at(*it).insert(keep);
    }
    _clusterNbs.at(keep).insert(_clusterNbs.at(remove).begin(),_clusterNbs.at(remove).end());
    _clusterNbs.erase(remove);
    gLogDebug << "Merged viewpoints: " << edge.first << " -- " << edge.second;;
}

float DAGMerger::calculatePriority(int id1, int id2) const
{
    auto dag1 = getClusterDag(id1);
    auto dag2 = getClusterDag(id2);

    int size1 = (int)dag1->getEdgeSize();
    int size2 =(int) dag2->getEdgeSize();
    int shareSize = 0;
    if(size1 < size2)
        shareSize = (int) dag1->shareSize(dag2->getDagGraph());
    else
        shareSize = (int) dag2->shareSize(dag1->getDagGraph());

    float res;
    if(_metricMode == 1)
        res = shareSize;
    else if(_metricMode == 2)
        res = numeric_limits<int>::max() - (size1+size2-shareSize);
    else if(_metricMode == 3)
        res = numeric_limits<int>::max() - (size1+size2-2*shareSize);
    else if(_metricMode == 4){
        auto vertices1 = dag1->getVertices();
        auto vertices2 = dag2->getVertices();
        vertices1.insert(vertices2.begin(),vertices2.end());
        int nVsize = (int)vertices1.size();
        res = -(float)(size1-shareSize)/(float)nVsize;
    }

    if (res<0 && _metricMode != 4) {
        gLogFatal << "edge size:" << size1+size2-shareSize;
        gLogFatal << "max int:" << numeric_limits<int>::max();
        gLogFatal << "Priority upperband too small, need increase";
        exit(-1);
    }
    gLogDebug<<"calculate Priority2 "<<id1<<" "<<id2<<" "<<res;
    return res;
}

vector<int> DAGMerger::getAssignments()
{
    int numNodes = (int)_assignments.size();
    vector<int> assigns(numNodes, 0);
    for(int i=0;i<numNodes;i++){
        assigns[i] = _assignments[i];
    }
    return assigns;
}

void DAGMerger::showResult()
{
    gLogInfo << "Final #set = " << _clusterDags.size();
    gLogInfo << _assignments.size() << " --> " << _clusterDags.size();
}

int DAGMerger::getNumberOfClusters( ) const
{
    return (int)_clusterDags.size();
}

vector<int> DAGMerger::getClusterIds() const
{
    vector<int> clusterIds;
    for(auto it = _clusterDags.cbegin();it != _clusterDags.cend();it++){
        auto clusterId = it->first;
        clusterIds.push_back(clusterId);
    }
    return clusterIds;
}

int DAGMerger::getClusterId(int dagId) const
{
    return _assignments.at(dagId);
}

std::set<int> DAGMerger::getClusterNb(int clusterId) const
{
    if(_clusterNbs.find(clusterId) == _clusterNbs.end()){
        gLogError<<"cluster "<<clusterId<<" is not find, please check";
    }
    std::set<int> clusterNbs;
    for(auto cluster :_clusterNbs.at(clusterId) )
        clusterNbs.insert(cluster);
    return clusterNbs;
}

bool DAGMerger::cleanEmpty(set<int> &clusterIds) const
{
    std::set<int> tempIds;
    bool empty = false;
    for(auto cluster : clusterIds){
        if(_clusterAssigns.find(cluster) == _clusterAssigns.end())
            empty = true;
        else
            tempIds.insert(cluster);
    }
    clusterIds = tempIds;
    return empty;
}

bool DAGMerger::shipBoundDag(int clusterId,const set<int>& clusterIds, std::unordered_map<int, std::set<int>>& ignoreClusters )
{
    if(_clusterAssigns.find(clusterId) == _clusterAssigns.end()){
        gLogError<<"cluster "<<clusterId<<" is not find in assign, please check";
    }
    const auto& dagIds = _clusterAssigns.at(clusterId);
    if(dagIds.size() == 0){
        gLogInfo<<"clean cluster "<<clusterId;
        delete _clusterDags.at(clusterId);
        _clusterDags.erase(clusterId);
        _clusterAssigns.erase(clusterId);
        for(auto it = _clusterNbs.at(clusterId).begin();it !=_clusterNbs.at(clusterId).end() ; it++){
           _clusterNbs.at(*it).erase(clusterId);
        }
        _clusterNbs.erase(clusterId);
        return false;
    }

    for(auto dagId:dagIds){
        const auto& nbDags = _edges.at(dagId);
        for(auto nbDag : nbDags){
            if(_assignments[nbDag] != _assignments[dagId]){
                int newClusterId = _assignments[nbDag];
                gLogDebug<<" find neighbour dag "<<dagId <<" "<<clusterId<<" "<<nbDag<<" "<<newClusterId;
                if(clusterIds.find(newClusterId) != clusterIds.end()){
                    gLogDebug<<"new cluster is among inside clusters";
                    continue;
                }
                if(ignoreClusters.find(dagId) != ignoreClusters.end())
                    if(ignoreClusters.at(dagId).find(newClusterId) != ignoreClusters.at(dagId).end()){
                        gLogDebug<<"ignore ship "<<dagId<<" to cluster "<<newClusterId;
                        continue;
                    }
                if(shipDag(dagId,clusterId,newClusterId)){
                    gLogDebug<<"ship dag "<<dagId <<" from "<<clusterId<<" to "<<newClusterId;
                    return true;
                }

                if(ignoreClusters.find(dagId) == ignoreClusters.end())
                    ignoreClusters[dagId] = std::set<int> {newClusterId};
                else
                    ignoreClusters.at(dagId).insert(newClusterId);
            }
        }
    }
    return false;
}

bool DAGMerger::shipDag(int dagId, int oldClusterId, int newClusterId)
{
    auto dag1 = getDag(dagId);
    auto dag3 = mergeDAGs(dag1,_clusterDags.at(newClusterId));
    if(dag3){
        delete _clusterDags.at(newClusterId);
        if(dag3->detectCycle()){
            gLogInfo<<"Please check there is cycle!";
        }
        _clusterDags.at(newClusterId) = dag3;
        _clusterDags.at(oldClusterId)->rmUpdateDag(*dag1);
        _assignments[dagId] = newClusterId;
        _clusterAssigns.at(newClusterId).push_back(dagId);
         auto it =std::find(_clusterAssigns.at(oldClusterId).begin(),_clusterAssigns.at(oldClusterId).end(),dagId);
         if(it == _clusterAssigns.at(oldClusterId).end()){
             gLogError<<"couldn't find dag in oldClusterId";
         }else{
             _clusterAssigns.at(oldClusterId).erase(it,it+1);
         }
         return true;
    }
    else{
        return false;
    }
    return false;
}

void DAGMerger::updateClusterNbs(const set<Edge> &edges)
{
    _clusterNbs.clear();
    for (Edge edge: edges) {
        auto clusterId1 = _assignments[edge.first];
        auto clusterId2 = _assignments[edge.second];
        if(clusterId1 != clusterId2){
            _addClusterNb(clusterId1,clusterId2);
            _addClusterNb(clusterId2,clusterId1);
        }
    }
}
