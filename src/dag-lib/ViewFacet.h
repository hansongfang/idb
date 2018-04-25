#ifndef SFViewFacet_H
#define SFViewFacet_H

#include "GraphCommon.h"
#include "ViewBase.h"
#include "InitViewModel.h"

/*
 * Class for viewpoints
 *
 */
class TRIDAG_LIB SFViewFacet: public InitViewModel, public SFViewBase
{
protected:
    vector<Triangle> _triangles;
    set<Edge> _adjacencies;

    void calculateTriangles();

    void calculateEdges();

public:
    SFViewFacet(const Vector3d &center, double radius, int nSplit);
    const vector<TetFace>& sfGetTriangles() const;
    const vector<Vector3d>& sfGetVertices() const;

protected:

    virtual set<Edge> getNodeAdjacencies() const;

    virtual void initViewNodes();
};

#endif // SFViewFacet_H
