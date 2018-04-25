#include "DAGFanVCache.h"
#include "dag-lib/Log.h"

DagFanVCache::DagFanVCache(int numVerts,int numTris,int cacheSize)
    :_numVerts(numVerts)
    ,_numTris(numTris)
    ,_cacheSize(cacheSize)
    ,_cachePos(cacheSize)
{
}

// old indices or our bfs indices
void DagFanVCache::init(const DagMap& occMap,const vector<int> &indices)
{
    _cachePos = _cacheSize;
    _occMap = occMap;

    _actiTris.clear();
    _inDegrees.clear();
    _inDegrees.resize(_numTris,0);
    for(auto it = _occMap.cbegin();it != _occMap.cend();it++){
        _actiTris.insert(it->first);
        for(auto innerIt = _occMap.at(it->first).cbegin();innerIt != _occMap.at(it->first).cend();innerIt++){
            _actiTris.insert(innerIt->first);
            _inDegrees[innerIt->first] ++;
        }
    }

    //only set the active tri as available
    _triStatus.clear();
    _triStatus.resize(_numTris,0);
    for(auto id : _actiTris)
        _triStatus[id] = 1;

    _remValence.clear();
    _adjTris.clear();
    _remValence.resize(_numVerts,0);
    vector<int> emptyV;
    for(int i=0;i<_numVerts;i++)
        _adjTris[i] = emptyV;
    for(int i=0;i<indices.size()/3;i++){
        for(int j=0;j<3;j++){
            auto vId = indices[i*3+j];
            _adjTris.at(vId).push_back(i);
            _remValence[vId]++;
        }
    }

    _cacheStamp.clear();
    _cacheStamp.resize(_numVerts,-1);
    _indicesOut.clear();
    _indicesOut.reserve(_numTris*3);

    _triOrderOut.clear();
    _triOrderOut.reserve(_numTris);
}

bool DagFanVCache::insideCache(int vId)
{
    if(_cachePos - _cacheStamp[vId] > _cacheSize){
        _cacheStamp[vId] = _cachePos;
        _cachePos++;
       // gLogInfo<<"push "<<vId;
        return false;
    }
    else
        return true;
}

void DagFanVCache::updateInDeg(int id)
{
    if(_occMap.find(id) != _occMap.end()){
        for(auto it = _occMap.at(id).cbegin();it!= _occMap.at(id).cend();it++){
            _inDegrees[it->first] --;
            if(_inDegrees[it->first] < 0){
                gLogError<<"inDegree Error!";
            }
        }
    }
}


void DagFanVCache::fanTop(int vId,const vector<int>& indices)
{
    //gLogInfo<<"vId: "<<vId<<" pos "<<_cachePos - _cacheStamp[vId];

    int bestemitted = -INT_MAX;
    int cachePosFan = _cachePos;
    next = -1;
    for(auto triId : _adjTris.at(vId)){
       // gLogInfo<<"tri "<<triId<<" degree "<<_inDegrees[triId];
        //add check inDegree
        if(checkTri(triId)){
            //gLogInfo<<"emit tri "<<triId;
            _triOrderOut.push_back(triId);
            //add update inDegree
            updateInDeg(triId);
            _triStatus[triId] = 0;
            for(auto j=0;j<3;j++){
                auto id = indices[triId*3+j];
                _indicesOut.push_back(id);
                bool t = insideCache(id);
                --_remValence[id];
                int v = numActiTris(id);
                if(v>0 && id != vId){
                    if(!t){
                        _startTailList.push_back(id);
                    }

                    int f = cf(cachePosFan,_cacheStamp[id],v);
                    //gLogInfo<<id<<" "<<v<<" "<<t<<" "<<f;
                    if(f>bestemitted){
                        bestemitted = f;
                        next = id;
                    }
                }
            }
        }
    }
    //gLogInfo<<"result";
    for(auto triId : _adjTris.at(vId)){
        //gLogInfo<<"tri "<<triId<<" degree "<<_inDegrees[triId]<<" status "<<_triStatus[triId];
    }
    //getchar();
}
int DagFanVCache::numActiTris(int vId)
{
    int sum =0;
    for(auto triId:_adjTris.at(vId)){
        if(checkTri(triId))
            sum++;
    }
    return sum;
}

int DagFanVCache::firstChoose()
{
    int bestCount =0;
    int vertId;

    for(auto it : _adjTris){
        auto vId = it.first;
        int count =0;
        for(auto triId : it.second ){
            if(checkTri(triId))
                count ++;
        }
        if(count > bestCount){
            vertId = vId;
            bestCount = count;
        }
    }
    //gLogInfo<<"vertId "<<vertId<<" count "<<bestCount;
    return vertId;

}

int DagFanVCache::chooseVert(const vector<int>& indices)
{
    //nothing in cache
    if(_cachePos == _cacheSize){
        for(int triId=0;triId <_numTris;triId++){
            if(checkTri(triId))
                return indices[triId*3];
        }
        //return firstChoose();
    }
    //gLogInfo<<"next "<<next;
    if(next == -1){
        int notFind = 1;
        while(!_startTailList.empty()){
            int id = _startTailList.front();
            //gLogInfo<<"pop tail list "<<id;
            _startTailList.pop_front();
            if(numActiTris(id) > 0){
                notFind =0;
                //_startTailList.clear();
                return id;
            }
        }
        if(notFind){
            for(int triId=0;triId <_numTris;triId++){
                if(checkTri(triId))
                    return indices[triId*3];
            }
        }
    }
    else{
        //_startTailList.clear();
        return next;
    }
    return -1;
}

bool DagFanVCache::checkFinish()
{
    int sum=0;
    for(auto v: _triStatus)
        sum += v;
    return sum == 0 ? true : false;
}

vector<int> DagFanVCache::sort(const vector<int> &indices)
{
    while(!checkFinish()){
        auto vId = chooseVert(indices);
        fanTop(vId,indices);
    }
     //float acmr = (float)(_cachePos - _cacheSize)/_numTris;
     //gLogInfo<<"fanCache acmr: "<<acmr;
    return _indicesOut;
}

vector<int> DagFanVCache::getTriOrder() const
{
    return _triOrderOut;
}
