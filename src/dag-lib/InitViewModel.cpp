#include "InitViewModel.h"
#include "Log.h"

// coordinates of one of the icosahedron vertex
#define X 0.525731112119133696
#define Z 0.850650808352039932

// icosahedron  vertices
static double icoVerts[12][3] = {
    { -X, 0.0, Z }, { X, 0.0, Z }, { -X, 0.0, -Z }, { X, 0.0, -Z },
    { 0.0, Z, X }, { 0.0, Z, -X }, { 0.0, -Z, X }, { 0.0, -Z, -X },
    { Z, X, 0.0 }, { -Z, X, 0.0 }, { Z, -X, 0.0 }, { -Z, -X, 0.0 }
};

// icosehedron faces
static int icoTri[20][3] = {
    { 1, 4, 0 }, { 4, 9, 0 }, { 4, 5, 9 }, { 8, 5, 4 }, { 1, 8, 4 },
    { 1, 10, 8 }, { 10, 3, 8 }, { 8, 3, 5 }, { 3, 2, 5 }, { 3, 7, 2 },
    { 3, 10, 7 }, { 10, 6, 7 }, { 6, 11, 7 }, { 6, 0, 11 }, { 6, 1, 0 },
    { 10, 1, 6 }, { 11, 0, 9 }, { 2, 11, 9 }, { 5, 2, 9 }, { 11, 2, 7 }
};

void InitViewModel::init(const Vector3d &center, double scale, int nSplit)
{
    _center = center;
    _radius = scale;
    _nSub = nSplit;

    _initIcoShape();
    subdivide(nSplit);
}

void InitViewModel::_initIcoShape()
{
    _vertices.resize(12);
    _faces.resize(20);

    // initialize a fixed icosahefron
    double scale = _radius;
    for (auto i = 0; i < 12; i++){
        _vertices[i] = Vector3d(icoVerts[i][0], icoVerts[i][1], icoVerts[i][2]) *scale;
        // move to center of original model
        _vertices[i] += _center;
    }
    for (auto i = 0; i < 20; i++){
        _faces[i][0] = icoTri[i][0];
        _faces[i][1] = icoTri[i][1];
        _faces[i][2] = icoTri[i][2];
    }

    // calculate normal and offset for each face
    for (int i = 0; i < _faces.size(); ++i)
    {
        Vector3d edge1(_vertices[_faces[i][1]] - _vertices[_faces[i][0]]);
        Vector3d edge2(_vertices[_faces[i][2]] - _vertices[_faces[i][0]]);
        _faces[i].d_normal = ((edge1.Cross(edge2)).Normalized());
        _faces[i].d_offset = _faces[i].d_normal.Dot(_vertices[_faces[i][0]]);
    }
}

void InitViewModel::subdivide(int n)
{
    /*
     * Each Triangle will generate 4 faces
     * 1. use each midpoint of the edges to subdivide
     * 2. because each edge belongs to faces, we'll generate new vertices once (vertexmap will check if that vertex is generated or not)
     */
    if (n == 0)
        return;

    vector<TetFace> newFaces;
    newFaces.reserve(_faces.size()*4);

    // return index of the midpoint vertext index given an edge
    std::map<Edge, int> vertexmap; //vertex mapping of new

    for (int i = 0; i < _faces.size(); ++i)
    {
        // new vertices
        Vector3d v[3];
        int vidx[3];
        const TetFace & face = _faces[i];
        for (int j = 0; j < 3; ++j)
        {
            int v1 = face[j];
            int v2 = face[(j+1)%3];
            Edge e = v1<v2?Edge(v1,v2):Edge(v2,v1);
            std::map<Edge, int>::iterator it = vertexmap.find(e);
            if (it != vertexmap.end()){
                // created before
                v[j] = _vertices[it->second];
                vidx[j] = it->second;
            }
            else {
                Vector3d midPoint ( (_vertices[v1] + _vertices[v2]) / 2.0 );
                Vector3d direction ((midPoint - _center).Normalized());
                v[j] = _center + direction * _radius;
                _vertices.push_back(v[j]);
                vidx[j] = (int)_vertices.size() - 1;
                vertexmap[e] = vidx[j];
            }
        }

        // new faces
        for ( int j = 0; j < 3; ++j )
        {
            Vector3d *nv[3];
            nv[0] = &v[j];
            nv[1] = &v[(j+2)%3];
            nv[2] = &_vertices[face[j]];
            Vector3d edge1 (*nv[0] - *nv[2] );
            Vector3d edge2 (*nv[1] - *nv[2] );

            TetFace newface;
            newface.d_normal = (edge1.Cross(edge2)).Normalized();
            newface.d_offset = v[j].Dot(newface.d_normal);
            newface[0] = face[j];
            newface[1] = vidx[j];
            newface[2] = vidx[(j+2)%3];
            newFaces.push_back(newface);
        }
        // add center faces
        {
            Vector3d edge1 (v[1] - v[0] );
            Vector3d edge2 (v[2] - v[0] );

            TetFace newface;
            newface.d_normal = (edge1.Cross(edge2)).Normalized();
            newface.d_offset = (v[0]).Dot(newface.d_normal);
            newface[0] = vidx[0];
            newface[1] = vidx[1];
            newface[2] = vidx[2];
            newFaces.push_back(newface);
        }
    }

    _faces.assign(newFaces.begin(), newFaces.end());

    subdivide(n-1);
}

InitViewModel::InitViewModel(const Vector3d &center, double scale, int nSubdivision)
{
    init(center, scale, nSubdivision);
}

const vector<Vector3d> &InitViewModel::getVertices() const
{
    return _vertices;
}

int InitViewModel::getNumberOfTriangles() const
{
    return (int)_faces.size();
}

vector<Vector3i> InitViewModel::getTriIds() const
{
    auto nTris = _faces.size();
    vector<Vector3i> triIds(nTris);
    for(auto i=0;i<nTris;i++){
        triIds[i][0] = _faces[i][0];
        triIds[i][1] = _faces[i][1];
        triIds[i][2] = _faces[i][2];
    }
    return triIds;
}
