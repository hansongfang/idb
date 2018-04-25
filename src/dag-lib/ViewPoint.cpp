#include "ViewPoint.h"

SFViewPoint::SFViewPoint(const Vector3d &center, double radius, int nSplit)
    :InitViewModel(center, radius, nSplit)
{
}

set<Edge> SFViewPoint::getNodeAdjacencies() const
{
    set<Edge> edges;
    for (int i = 0; i < _faces.size(); ++i) {
        const TetFace & face = _faces[i];
        for (int j = 0; j < 3; ++j)
        {
            int v1 = face[j];
            int v2 = face[(j+1)%3];
            Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
            edges.insert(e);
        }
    }
    return edges;
}

void SFViewPoint::initViewNodes()
{
    //generate view nodes
    _viewNodes.resize(_vertices.size());
    for (auto i = 0; i < _vertices.size(); ++i) {
        _viewNodes[i] = new ViewNodePoint(_vertices[i], i);
    }
    initGraph();
}
