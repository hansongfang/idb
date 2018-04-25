#ifndef DAGMerger_H
#define DAGMerger_H

#include "DAG.h"
#include "DAGCenter.h"
#include "ViewBase.h"
#include <unordered_map>

/*
 * Merge DAG to generate limited buffers
 *
 * notes: this file contains function for inital merging and relaxing clusters
 *
 */

class TRIDAG_LIB DAGMerger
{
    int                                                _metricMode;
    DAGCenter*                                         _pDagCenter;

    std::map<Edge,float>                               _prioritylist;
    std::unordered_map<int, DAG*>                      _clusterDags; //map clusterId to clusterDAG
    std::unordered_map<int, vector<int>>               _clusterAssigns; //map clusterId to dagIds within this cluster
    std::map<int,int>                                  _assignments;//map dagId to clusterId
    std::unordered_map<int, std::unordered_set<int>>   _clusterNbs;//map clusterId to its neighbor clusterIds
    std::unordered_map<int, std::unordered_set<int>>   _edges;//map dagId to its neighbor dagIds

public:
    DAGMerger(int nNodes,DAGCenter* pDagCenter, SFViewBase *pViewMesh, int metricMode);
    ~DAGMerger();

    void init(int nNodes, SFViewBase *pViewMesh);

    //merging
    bool hasQueue() const;
    void mergeTop(); // pop two cluster to merge

    //relaxing clusters
    bool shipBoundDag(int clusterId,const set<int>& clusterIds,std::unordered_map<int, std::set<int>>& ignoreClusters);
    void updateClusterNbs(const set<Edge> &edges);
    bool cleanEmpty(set<int>& clusterIds) const;

    //io
    void showResult();
    int getNumberOfClusters( ) const;
    vector<int> getAssignments();
    int getClusterId(int dagId) const;
    vector<int> getClusterIds() const;
    std::set<int> getClusterNb( int clusterId) const;
    const DAG* getDag(int id) const;
    const DAG* getClusterDag(int id) const;

protected:
    void initEdges( const set<Edge> &edges ); //init priority queue for neighbor clusters
    void _addClusterNb(const int& id1, const int& id2);//init cluster neighbor

    Edge getTopEdge() const;
    void popEdge(Edge edge);
    DAG *mergeDAGs(const DAG* dag1, const DAG* dag2) const;
    void collopseEdge( const Edge &edge );//incrementally collapse Edge<first, second>, remove first, keep second
    void updatePriority(const Edge &edge );// update priority
    float calculatePriority(int id1, int id2 ) const;//calculate priority

    bool shipDag(int dagId, int oldClusterId, int  newClusterId);// ship dag from oldcluster to new cluster
};

#endif // DAGMerger_H
