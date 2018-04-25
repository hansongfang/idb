#ifndef DAGFANVCACHE_H
#define DAGFANVCACHE_H

#include "dag-lib/DagGraph.h"
#include <set>
#include <unordered_map>
#include <deque>

class DagFanVCache
{
    int                                          _numTris;
    int                                          _numVerts;
    int                                          _cachePos;
    int                                          _cacheSize;

    vector<int>                                  _inDegrees;// from triId to indegree
    vector<int>                                  _triStatus; // from triId to active status
    vector<int>                                  _remValence;//from vertex Id to # active tris
    std::unordered_map<int,vector<int>>          _adjTris;//from vertexId to adjTris
    vector<int>                                  _cacheStamp;//from vertexId to cacheStamp

    DagMap                                       _occMap;
    set<int>                                     _actiTris;
    vector<int>                                  _indicesOut;
    deque<int>                                   _startTailList;
    int                                          next;
    vector<int>                                  _triOrderOut;

private:
    // when triId is put to outIndice
    int firstChoose();
    void updateInDeg(int triId);
    bool checkFinish();
    void fanTop(int vId,const vector<int>& indices);
    bool checkTri(int triId);
    bool insideCache(int vId);
    int cf(int p, int c, int v);
    int chooseVert(const vector<int>& indices);
    int numActiTris(int vId);
public:
    DagFanVCache(int numVerts,int numTris,int cacheSize);
    // this indices must be corresponding to the triId in occMap
    //model->getIndices();
    void init(const DagMap& occMap,const vector<int>& indices);
    vector<int> sort(const vector<int>& indices);
    vector<int> getTriOrder() const;
};
inline bool DagFanVCache::checkTri(int triId)
{
    return _triStatus.at(triId) == 1 && _inDegrees[triId] == 0;
}

inline int DagFanVCache::cf(int p, int c, int v)
{
    return (((p-c+2*v) > _cacheSize) ? (0) : (p-c));
}


#endif // DAGFANVCACHE_H
