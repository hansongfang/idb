#ifndef DAGMERGEUP_H
#define DAGMERGEUP_H

#include <unordered_map>
#include "dag-lib/ModelBase.h"
#include "dag-lib/GraphCommon.h"
#include <deque>

class DagMergeUp
{
    ModelBase                            *_pModel; // fine model without back face
    ModelBase                            *_pbModel;// fine model with back face
    ModelBase                            *_pCModel;// coarse model without back face

    vector<Triangle>                     _Triangles; // fine model triangles
    vector<Triangle>                     _cTriangles; // coarse model triangles
    int                                  _nTris; // num tris for fine model
    int                                  _ncTris;// num tris for coarse model

    std::unordered_map<int,vector<int>>  _triAdj1;//construct neighbor map for fine model
    std::unordered_map<int,vector<int>>  _triAdj2;//construct neighbor map for fine model

    vector<int>                          _triStick;//map from fine tri to coarse tri
                                                   //-1 means not find, others return the coarse tri
    std::unordered_map<int,vector<int>>  _cTriStick;//map from coarse tri to fine tris
                                                    // one corase tri could map to multiple fine tris
    deque<int>                           _candidates;

    vector<int>                          _assignments;

private:
    void buildEdgeAdj(); // neighbor sharing edge
    void buildPointAdj(); // neighbor sharing point
    void initFind();
    int popNext();
    void findCor(const int& triId,const int& cBaseId);
    bool testFind(const int& triId);
    bool checkFinish();

    void enrich();//add backFaces correspondence
    void revert();// map from coarse tri to fine tris

    //for save indices
    void transClusterIndices(int clusterId,string cacheDir, string cCacheDir);
    void loadAssigns(string cacheDir, string cCacheDir);

public:
    DagMergeUp();
    void init(const char* fileName, const char* sFileName,int nCacheSize = 20);
    void buildTri();// construct map from coarse tri to find tris


    void transIndices(string cacheDir, string cCacheDir);
};

#endif // DAGMERGEUP_H
