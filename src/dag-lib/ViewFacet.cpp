#include "ViewFacet.h"
#include "Log.h"

void SFViewFacet::calculateTriangles()
{
    _triangles.resize(_faces.size());
    for (int i = 0; i < _faces.size(); ++i) {
        const TetFace & face = _faces[i];
        Triangle &tri = _triangles[i];
        tri._id = i;

        std::copy(std::begin(face.d_v),std::end(face.d_v),std::begin(tri._vId));
        tri._normal = face.d_normal;
        for (int j = 0; j < 3; ++j) {
            tri._vertices[j] = this->_vertices[face.d_v[j]];
        }

        tri.enrich();
    }
}

void SFViewFacet::calculateEdges()
{
    _adjacencies.clear();
    map<Edge, int> edges;
    for (int i = 0; i < _faces.size(); ++i) {
        const TetFace & face = _faces[i];
        for (int j = 0; j < 3; ++j)
        {
            int v1 = face[j];
            int v2 = face[(j+1)%3];
            Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
            if (edges.count(e)){
                int u1 = edges[e];
                int u2 = i;
                Edge adj = u1<u2?Edge(u1,u2):Edge(u2,u1);
                _adjacencies.insert(adj);
            }
            else
                edges[e] = i;
        }
    }
}

SFViewFacet::SFViewFacet(const Vector3d &center, double radius, int nSplit)
    :InitViewModel(center, radius, nSplit)
{
}

set<Edge> SFViewFacet::getNodeAdjacencies() const
{
    return _adjacencies;
}

void SFViewFacet::initViewNodes()
{
    gLogInfo << "SFViewFacet initViewNodes";
    calculateTriangles();
    calculateEdges();

    _viewNodes.resize(_triangles.size());
    for (auto i = 0; i < _triangles.size(); ++i) {
        _viewNodes[i] = new ViewNodeTriangle(_triangles[i], i);
    }

    initGraph();
}

const vector<TetFace>& SFViewFacet::sfGetTriangles() const
{
    return _faces;
}

const vector<Vector3d>& SFViewFacet::sfGetVertices() const
{
    return _vertices;
}
