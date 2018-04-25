#ifndef BufferMaker_H
#define BufferMaker_H

#include <string>
#include "dag-lib/DAGCenter.h"
#include "dag-lib/DagMerger.h"
#include "dag-lib/SampledTriangle.h"
#include "DAGFanVCache.h"
#include "dag-lib/DAGCenter.h"

class BufferMaker
{
private:
    ModelBase                          *_pModel;
    DAGCenter                             *_pDagCenter;
    SFViewBase                           *_pViewModel;

    DAGMerger                           *_pDagMerger;
    DagFanVCache                          *_dagFanVCache;
protected:
    void _vCacheOrder(int clusterId, string cacheDir);
public:
    BufferMaker(ModelBase* pModel, DAGCenter* pDagCenter);
    ~BufferMaker();

    void init(int nSplit, int metric);

    void merge();
    void updateClusters();
    // save assignments
    void saveAssign(string cacheDir) const;
    // vertex cache optimize
    void vCacheOrder(string outDir);

    int getNClusters();
};

#endif // BufferMaker_H
