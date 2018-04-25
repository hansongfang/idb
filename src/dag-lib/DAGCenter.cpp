#include "DAGCenter.h"
#include "Log.h"

#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

DAGCenter::DAGCenter(int nTris)
    :_nDags(-1)
    ,_nTris(nTris)
    ,_nPointDags(-1)
    ,_numFailed(0)

{
    _dags.clear();
    _pointDags.clear();
}

DAGCenter::~DAGCenter()
{
}

void DAGCenter::initDags(int nDags)
{
    _dags.resize(nDags, 0);
    for (auto i = 0; i < nDags; ++i)
        _dags[i] = new DAG(i, _nTris);

    _nDags = nDags;
}

void DAGCenter::initPointDags(int nPointDags)
{
    _pointDags.resize(nPointDags, 0);
    for(auto i=0;i<nPointDags;i++){
        _pointDags[i] = new DAG(i, _nTris);
    }
    _nPointDags = nPointDags;
}

DAG* DAGCenter::getPointDag(int id)
{
    return _pointDags[id];
}

const DAG* DAGCenter::getDag(int id) const
{
	return _dags[id];
}

DAG* DAGCenter::getDag2(int id)
{
    return _dags[id];
}

void DAGCenter::cleanDag(int id)
{
    if(_dags[id]){
       delete _dags[id];
       _dags[id] = nullptr;
    }
}

void DAGCenter::cleanPointDags()
{
    for(int id =0; id < _nPointDags; id++){
        if(_pointDags[id]){
            delete _pointDags[id];
            _pointDags[id] = nullptr;
        }
    }
}

string DAGCenter::getInitFilePath(string cacheFolder) const
{
    fs::path dir(cacheFolder);
    return fs::complete((dir / "index")).string();
}

bool DAGCenter::load(string cacheFolder)
{
    for (auto i = 0; i < _nDags; ++i) {
        _dags[i] = new DAG(i, cacheFolder);
    }
    gLogInfo << "Loaded DAGs ";
    return true;
}

void DAGCenter::save(string cacheFolder)
{
    if ( cacheFolder.empty() ) {
        gLogWarn << "No cache folder provided, skip saving!";
        return;
    }

    //1. save nDags and nNodes to file
    auto filename = getInitFilePath(cacheFolder);
    gLogInfo << "Index file: " << filename;
    ofstream fout(filename);
    fout << _dags.size() << std::endl;
    fout << _nDags << std::endl;
    fout << cacheFolder << std::endl;
    fout.close();
    gLogInfo << "Index file saved: " << filename;

    //2. save individual Dag
    for(const auto&dag: _dags) {
        dag->saveDAG(cacheFolder);
    }
}

