#include "DAGMaker.h"
#include "dag-lib/Log.h"
#include "dag-lib/ViewPoint.h"
#include "dag-lib/ViewFacet.h"
#include "dag-lib/DAG.h"
#include "omp.h"
#include "dag-lib/DagGraph.h"
#include <fstream>
#include <time.h>
#include "dag-lib/SampledTriangle.h"
#include "dag-lib/DAGCenter.h"

DagMaker::DagMaker(ModelBase* pModel, DAGCenter* pDagCenter)
    :_pModel(pModel)
    ,_pPointViewModel(0)
    ,_pTriViewModel(0)
    ,_pDagCenter(pDagCenter)
    ,_pOccluder(0)
{

}

DagMaker::~DagMaker()
{
    if (_pPointViewModel)
        delete _pPointViewModel;

    if(_pTriViewModel)
        delete _pTriViewModel;

    if (_pOccluder)
        delete _pOccluder;
}

void DagMaker::init(int triSub,int pointSubLevel, double Parameter1)
{
    // Initiate View Model
    auto radius = _pModel->getRadius();
    auto center = _pModel->getCenter3d();
    _pTriViewModel = new SFViewFacet(center, 3.0*radius, triSub);
    _pTriViewModel->initViewNodes();
    _pPointViewModel = new SFViewPoint(center, 3.0*radius, pointSubLevel);
    _pPointViewModel->initViewNodes();

    // init Dags
    int nTriDags = _pTriViewModel->getNumberOfNodes();
    _pDagCenter->initDags(nTriDags);
    auto nPointDags = _pPointViewModel->getNumberOfNodes();
    _pDagCenter->initPointDags(nPointDags);

    // Dag occluder
    _pOccluder = new DagOccluder(
                _pPointViewModel->getNodes(), _pModel->getTriangles(),
                _pModel->getRadius()*3.0, Parameter1);
}

// compute partial order graphs associated with each view
void DagMaker::genDags()
{
    gLogInfo << "Generating DAGs";
    int numthreads = omp_get_max_threads();

    int nPointDags = _pPointViewModel->getNumberOfNodes();
    int nTris = _pModel->getNumberOfTriangles();

    _pOccluder->preprocess(numthreads);
    _pOccluder->preprocessRelation(numthreads);

    omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(numthreads -1);
#pragma omp parallel for
    for(auto k=0;k<nPointDags;++k){
        _pOccluder->preprocessVEMap(k);
        DAG* dag = _pDagCenter->getPointDag(k);
        for (auto i = 0; i < nTris; ++i) {
            if(_pOccluder->isFaceAway(k,i))
                continue;
            for (auto j = i+1; j < nTris; ++j) {
                if(_pOccluder->isFaceAway(k,j))
                    continue;

                auto r = _pOccluder->occludeSimple(k,i,j,false);
                if ( r == 0 )
                    continue;
                else if (r == 1)
                    dag->addEdge( i, j);
                else if (r == 2)
                    dag->addEdge( j, i);
                else if(r == 3 )
                    gLogError<<"Self cycle edge! ";
            }
        }
        _pOccluder->clearVEMap(k);
    }
    gLogInfo << "Successfully generated point DAGs";
}

void DagMaker::savePointDags(string pointDir)
{
    int nPointDags = _pPointViewModel->getNumberOfNodes();
//#pragma omp parallel for
    for(auto k=0;k<nPointDags;++k){
        DAG* dag = _pDagCenter->getPointDag(k);
        dag->saveDAG(pointDir);
    }
    gLogInfo<<"Successfully saving point DAGs";
}

void DagMaker::preClustering(int innerSub)
{
    gLogInfo<<"pre clustering";
    auto radius = 3.0 * _pModel->getRadius();
    auto vertices = static_cast<const SFViewFacet*>(_pTriViewModel)->sfGetVertices();
    auto faces = static_cast<const SFViewFacet*>(_pTriViewModel)->sfGetTriangles();
    SampledTriangle sampledTri(innerSub,vertices,faces,
                                     radius,_pModel->getCenter3d());
    auto preClusteringMap = sampledTri.getMap();
    int nDags = _pTriViewModel->getNumberOfNodes();
    for(int triId=0;triId<nDags;triId++){
        auto triDag = _pDagCenter->getDag2(triId);
        auto pointIds = preClusteringMap.at(triId);
        for(auto pointId:pointIds){
            auto pointDag = _pDagCenter->getPointDag(pointId);
            triDag->updateDAG(*pointDag);
        }
    }
    _pDagCenter->cleanPointDags();
    gLogInfo<<"finish preclustering";
}
