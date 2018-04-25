#ifndef SFViewPoint_H
#define SFViewPoint_H

#include "ViewBase.h"
#include "InitViewModel.h"

/*
 * Class for view triangle
 *
 */
class TRIDAG_LIB SFViewPoint:public InitViewModel, public SFViewBase
{
public:
    SFViewPoint(const Vector3d &center, double radius, int nSplit);

protected:

    virtual set<Edge> getNodeAdjacencies() const;

    virtual void initViewNodes();
};

#endif // SFViewPoint_H
