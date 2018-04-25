#ifndef DAGMAKER_H
#define DAGMAKER_H

#include "dag-lib/ModelBase.h"
#include "dag-lib/ViewBase.h"
#include "dag-lib/Occluder.h"
#include "dag-lib/DAGCenter.h"
#include "dag-lib/SampledTriangle.h"

/*
 * class to compute partial order associated with each viewpoint
*/
class DagMaker
{
private:
    typedef std::pair<int,int> Edge;
    ModelBase        *_pModel;
    SFViewBase         *_pPointViewModel;
    SFViewBase         *_pTriViewModel;
    DAGCenter           *_pDagCenter;
    DagOccluder         *_pOccluder;

public:
    DagMaker(ModelBase* pModel, DAGCenter* pDagCenter);
    ~DagMaker();

    void init(int triSub,int pointSubLevel, double Parameter1 = 1000.0f);

    void genDags();

    // preclustering
    void preClustering(int innerSub);
    //to delete
    void savePointDags(string pointDir);
};

#endif // DAGMAKER_H
