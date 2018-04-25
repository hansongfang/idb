#include "BufferMaker.h"
#include "dag-lib/Log.h"
#include "dag-lib/ViewPoint.h"
#include "dag-lib/ViewFacet.h"
#include <time.h>
#include <random>
#include <boost/filesystem.hpp>
#include <boost/serialization/serialization.hpp>
#include<boost/serialization/unordered_map.hpp>
#include<boost/serialization/set.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/archive/text_oarchive.hpp>
#include <fstream>
#include <time.h>
#include <math.h>

namespace fs = boost::filesystem;

BufferMaker::BufferMaker(ModelBase* pModel, DAGCenter* pDagCenter)
    :_pModel(pModel)
    ,_pDagCenter(pDagCenter)
    ,_pViewModel(0)
{
}

BufferMaker::~BufferMaker()
{
    if (_pDagMerger)
        delete _pDagMerger;

    if (_dagFanVCache)
        delete _dagFanVCache;

    if (_pViewModel)
        delete _pViewModel;
}

void BufferMaker::init(int nSplit, int metric)
{
    auto center = _pModel->getCenter3d();
    auto radius = _pModel->getRadius();
    _pViewModel = new SFViewFacet(center, 3.0*radius, nSplit);
    _pViewModel->initViewNodes();

    auto nDags = _pViewModel->getNumberOfNodes();
    _pDagMerger = new DAGMerger(nDags, _pDagCenter,_pViewModel,metric);
}

void BufferMaker::merge()
{
    while( _pDagMerger->hasQueue() ) {
        _pDagMerger->mergeTop();
    }

    _pDagMerger->showResult();
}

void BufferMaker::updateClusters()
{
    int nNodes = _pViewModel->getNumberOfNodes();
    _pDagMerger->updateClusterNbs(_pViewModel->getViewEdges());

    std::default_random_engine generator(std::time(0));
    std::uniform_int_distribution<unsigned> distribution(0,nNodes-1);
    int dagId = (int) distribution(generator);

    int clusterId = _pDagMerger->getClusterId(dagId);
    auto clusterNbs = _pDagMerger->getClusterNb(clusterId);
    clusterNbs.insert(clusterId);

    gLogInfo<<"relax cluster "<<clusterId;
    // ship from 1-ring to 2-ring
    for(auto cluster :clusterNbs){
        if(cluster == clusterId)
            continue;
        std::unordered_map<int, std::set<int>> ignoreClusters;
        while(_pDagMerger->shipBoundDag(cluster,clusterNbs, ignoreClusters)){}
    }
    // ship from center to 1-ring neighbor
    std::set<int> temp{clusterId};
    std::unordered_map<int, std::set<int>> ignoreClusters;
    while(_pDagMerger->shipBoundDag(clusterId,temp,ignoreClusters)){}

    _pDagMerger->cleanEmpty(clusterNbs);
}

int BufferMaker::getNClusters()
{
    return _pDagMerger->getNumberOfClusters();
}

void BufferMaker::saveAssign(string cacheDir) const
{
    string fileName = cacheDir +"/assignments.txt";
    ofstream ofs(fileName);

    auto assigns = _pDagMerger->getAssignments();

    int nNodes = assigns.size();
    auto center = _pModel->getCenter3d();
    auto viewNodes = _pViewModel->getNodes();
    for(int k=0;k<nNodes;k++){
        auto tri = static_cast<const ViewNodeTriangle*>(viewNodes[k])->getTriangle();
        auto triVec = (tri.getCenter() - center);
        triVec.Normalize();

        double theta = acos(triVec[2]);
        double phi = atan2(triVec[1], triVec[0]);
        if(k != nNodes -1)
            ofs<< assigns[k]<<" "<< theta<<" "<<phi<<endl;
        else
            ofs<< assigns[k]<<" "<< theta<<" "<<phi;
    }
    ofs.close();
}

//  DAGMerger need add getClusterIds
//  DAGMerger move getClusterDag to public
//  DAGMerger add clean clusterDag
void BufferMaker::vCacheOrder(string outDir)
{
    _dagFanVCache = new DagFanVCache(_pModel->getNumberOfVertices(),_pModel->getNumberOfTriangles(),20);

    vector<int> clusterIds = _pDagMerger->getClusterIds();
    for(auto clusterId : clusterIds){
        this->_vCacheOrder(clusterId, outDir);
        //_pDagMerger->clearClusterDag(clusterId);
    }
}

void BufferMaker::_vCacheOrder(int clusterId, string outDir)
{
    // compute DAG
    // Given a occlude b
    // change to b--> a from a --> b
    auto clusterDag = _pDagMerger->getClusterDag(clusterId);
    auto occGraph = clusterDag->getDagGraph();
    auto occMap = occGraph.getDagMap();

    DagGraph tempGraph;
    for(auto it = occMap.cbegin();it!= occMap.cend();it++){
        for(auto innerIt = occMap.at(it->first).cbegin();innerIt != occMap.at(it->first).cend();innerIt++){
            tempGraph.addEdge(innerIt->first,it->first,innerIt->second);
        }
    }
    auto revertMap = tempGraph.getDagMap();

    auto oldIndices = _pModel->getIndices();
    _dagFanVCache->init(revertMap,oldIndices);
    auto newIndices = _dagFanVCache->sort(oldIndices);
    //log the indices out and render to see the result
    //save the indices
    string fileName = outDir +"/Indices_"+to_string(clusterId)+".txt";
    gLogInfo<<"save "<<fileName;
    std::ofstream fout(fileName);
    for(auto vid : newIndices)
        fout<<vid<<std::endl;
    fout.close();

    //save the triOrder used in order-upsample
    auto triOrders = _dagFanVCache->getTriOrder();
    fileName = outDir +"/triOrder_"+to_string(clusterId)+".txt";
    gLogInfo<<"save "<<fileName;
    fout.open(fileName);
    for(auto triId : triOrders)
        fout<<triId<<std::endl;
    fout.close();
}
