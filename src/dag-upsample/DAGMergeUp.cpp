#include "DAGMergeUp.h"
#include "dag-lib/Log.h"
#include <set>
#include <boost/serialization/serialization.hpp>
#include<boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <filesystem>
#include <fstream>
#include <cstdint>
namespace fs = std::experimental::filesystem;

struct pairHash{
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

DagMergeUp::DagMergeUp()
    :_nTris(0)
    ,_ncTris(0)
{
    _triAdj1.clear();
    _triAdj2.clear();

    _triStick.clear();
    _candidates.clear();
}

void DagMergeUp::init(const char *fileName, const char *sFileName, int nCacheSize)
{
    //first do the look up with not back faces
    _pModel = new ModelBase(fileName, false);
    _pbModel = new ModelBase(fileName, true);
    _pCModel = new ModelBase(sFileName,false);

    // getTriangles
    _nTris = _pModel->getNumberOfTriangles();
    _ncTris = _pCModel->getNumberOfTriangles();

    _Triangles = _pModel->getTriangles();
    _cTriangles = _pCModel->getTriangles();

    _triStick.clear();
    _triStick.resize(_nTris,-1);
    _cTriStick.clear();
    _candidates.clear();
    _assignments.clear();

    _triAdj1.clear();
    buildEdgeAdj();
    _triAdj2.clear();
    buildPointAdj();

    gLogInfo<<"finish init";
}

void DagMergeUp::buildEdgeAdj()
{
    //for fine model
    //build a map from Edge to triangles containing it
    std::unordered_map<Edge,vector<int>,pairHash> edgeToTri;
    edgeToTri.reserve(_nTris*2);
    for(int triId =0;triId<_nTris;triId++){
        const Triangle& tri = _Triangles[triId];
        for(int id =0;id<3;id++){
            int v1 = tri._vId[id];
            int v2 = tri._vId[(id+1)%3];
            Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
            if(edgeToTri.find(e) == edgeToTri.end())
                edgeToTri[e] = vector<int>{triId};
            else
                edgeToTri.at(e).push_back(triId);
        }
    }

    // based on edgeToTri map
    // build map from triangle to its neighbor triangles which share edge
    for(int triId =0;triId<_nTris;triId++){
        const Triangle& tri = _Triangles[triId];
        _triAdj1[triId] = vector<int>{};
        for(int id =0;id<3;id++){
            int v1 = tri._vId[id];
            int v2 = tri._vId[(id+1)%3];
            Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
            for(auto tmpTriId: edgeToTri.at(e)){
                if(tmpTriId != triId)
                    _triAdj1.at(triId).push_back(tmpTriId);
            }
        }
    }
}

void DagMergeUp::buildPointAdj()
{
    //for coarse model
    //build a map from vertex to triangle containing it
    std::unordered_map<int, vector<int>> vertToTri;
    for(int triId =0;triId<_ncTris;triId++){
        const Triangle& tri = _cTriangles[triId];
        for(auto id=0;id<3;id++){
            int vId = tri._vId[id];
            if(vertToTri.find(vId) == vertToTri.end())
                vertToTri[vId] = vector<int>{triId};
            else
                vertToTri.at(vId).push_back(triId);
        }
    }

    // construct map from triangle to neighbor triangle
    // which share vertex
    for(int triId =0;triId<_ncTris;triId++){
        const Triangle& tri = _cTriangles[triId];
        std::set<int> tempAdj;
        for(auto id=0;id<3;id++){
            int vId = tri._vId[id];
            tempAdj.insert(vertToTri.at(vId).begin(),vertToTri.at(vId).end());
        }
        _triAdj2[triId]=vector<int>(tempAdj.begin(),tempAdj.end());
    }
}

void DagMergeUp::buildTri()
{
    // find nearest coarse triangle for one fine triangle
    initFind();

    // incrementally find corresponding for other fine triangles
    int next = popNext();
    while(next!= -1){
        int nextStick = _triStick[next];
        for(auto triId : _triAdj1.at(next))
            findCor(triId, nextStick);
        next = popNext();
    }

    if(!checkFinish()){
        gLogError<<"the model is not all connected together";
    }
    enrich();
    revert();
    gLogInfo<<"finish build Tri correlation";
}


void DagMergeUp::initFind()
{
    for(auto triId =0;triId<_nTris;triId++){
        if(!testFind(triId)){
            const Triangle& tri =  _Triangles[triId];
            _candidates.push_back(triId);

            //iterate all tris to find the most nearest one
            double minDist = (double)INT_MAX;
            int minId;
            for(auto cId = 0;cId<_ncTris;cId++){
                const Triangle& sTri = _cTriangles[cId];
                auto tempDist = (tri._center - sTri._center).Len();
                if(tempDist < minDist){
                    minDist = tempDist;
                    minId = cId;
                }
            }
            _triStick[triId] = minId;
            return;
        }
    }
}

int DagMergeUp::popNext()
{
    int next = -1;
    if(_candidates.size() > 0){
        next = _candidates.front();
        _candidates.pop_front();
        return next;
    }
    return next;
}

// search among cBasedId and cBasedId neighbors
void DagMergeUp::findCor(const int& triId, const int& cBaseId)
{
    //increamentally search
    // fine tri a1 --  coarse tri b1
    // for a2 which is the neighbor a1
    // search among b1 and b1 neighbors
    if(testFind(triId))
        return;
    _candidates.push_back(triId);
    double minDist = (double)INT_MAX;
    int minId =0;
    const Triangle& tri = _Triangles[triId];
    for(auto sId:_triAdj2.at(cBaseId)){
        const Triangle& sTri = _cTriangles[sId];
        double tempDist = (tri._center - sTri._center).Len();
        if(tempDist < minDist){
            minDist = tempDist;
            minId = sId;
        }
    }
    _triStick[triId] = minId;
}

bool DagMergeUp::testFind(const int& triId)
{
    //check if the tri is find or not
    if(_triStick[triId] == -1)
        return false;

    return true;
}

bool DagMergeUp::checkFinish()
{
    //check if the tri is finish or not
    for(auto stick : _triStick){
        if(stick == -1){
            return false;
            gLogError<<"Model has split part or our algorithm is wrong at some part";
        }
    }
    return true;
}

void DagMergeUp::enrich()
{
    //add backFaces correspondence
    _triStick.reserve(_nTris*2);
    for(auto i=0;i<_nTris;i++){
        auto sId = _triStick[i];
        if(sId == -1){
            gLogError<<"tri not find corresponding stri";
            continue;
        }
        sId += _ncTris;
        _triStick.push_back(sId);
    }
}

void DagMergeUp::revert()
{
    // map from coarse tri to fine tris
    if(_triStick.size() != _nTris*2){
        gLogError<<"revert should use after enrich"<<_nTris<<" "<<_triStick.size();
    }


    _cTriStick.clear();
    for(auto i=0;i<_ncTris*2;i++){
        _cTriStick[i] = vector<int>{};
    }
    for(auto triId =0;triId<_triStick.size();triId++){
        int sId = _triStick[triId];
        _cTriStick.at(sId).push_back(triId);
    }
}

void DagMergeUp::loadAssigns(string cacheDir, string cCacheDir)
{
    //read in assignments
    string cFilename = cCacheDir +"/assignments.txt";
    string filename = cacheDir+"/assignments.txt";
    try{
        fs::copy(cFilename, filename);
    }
    catch(std::exception &ex)
    {
        gLogError<<"fail copy assignment "<<ex.what();
    }

    _assignments.clear();
    ifstream ifs(filename);
    if(ifs.is_open()){
        double theta, phi;
        int bufferId;
        while (!ifs.eof() ){
            ifs >> bufferId >> theta >> phi;
            _assignments.push_back(bufferId);
        }
    }else{
        gLogError<<" assignment file not found "<<filename;
    }
    ifs.close();
}

void DagMergeUp::transClusterIndices(int bufferId, string cacheDir, string cCacheDir)
{
    std::string fileName = cCacheDir +"/triOrder_"+to_string(bufferId)+".txt";
    vector<int> cTriOrder;
    cTriOrder.reserve(_ncTris*2);
    std::ifstream ifs(fileName);
    if(ifs.is_open()){
        int id;
        while(ifs >> id)
            cTriOrder.push_back(id);
    }
    ifs.close();

    // tri order buffer
    vector<int> triOrder;
    triOrder.reserve(_nTris*2);
    for(auto cTriId : cTriOrder){
        if(_cTriStick.find(cTriId) == _cTriStick.end()){
            gLogError<<"fix situation when the closet triangle is not active";
            continue;
        }
        for(auto triId : _cTriStick.at(cTriId) )
            triOrder.push_back(triId);
    }

    // indice buffer
    int tempTris = (int)triOrder.size();
    vector<int> outIndices;
    outIndices.clear();
    outIndices.reserve(tempTris*3);
    const auto& oldIndices = _pbModel->getIndices();
    for(auto triId : triOrder){
        for(int j =0;j<3;j++)
            outIndices.push_back(oldIndices[triId*3+j]);
    }

    fileName = cacheDir +"/triOrder_"+to_string(bufferId)+".txt";
    std::ofstream fout(fileName);
    for(auto triId : triOrder)
        fout<<triId<<std::endl;
    fout.close();

    fileName = cacheDir +"/Indices_"+to_string(bufferId)+".txt";
    fout.open(fileName);
    for(auto vid : outIndices)
        fout<<vid<<std::endl;
    fout.close();
}

void DagMergeUp::transIndices(string cacheDir, string cCacheDir)
{
    gLogInfo<<"copy assignments";
    loadAssigns(cacheDir, cCacheDir);
    gLogInfo<<"transfer indice buffers";
    set<int> bufferIds(_assignments.begin(), _assignments.end());
    for(auto bufferId : bufferIds){
        gLogInfo<<"transfer buffer "<<bufferId;
        transClusterIndices(bufferId,cacheDir,cCacheDir);
    }
    gLogInfo<<"finish order upsampling";
}

