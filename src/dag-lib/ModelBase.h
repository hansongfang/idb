#pragma once

#include "rply.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "SFVector.h"
#include "DLL.h"
#include "GraphCommon.h"

/*
 * Load ply model
 *
 * notes: each face is duplicated with a back face
 *
 */
class TRIDAG_LIB ModelBase
{
protected:
    struct Vertex
    {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoord;
        Vector3 tangent;
        Vector3 bitangent;
    };

public:
    ModelBase(const char* filename, bool addBackFaces = false, bool loadNormal = false);
    virtual ~ModelBase(void);

    void loadPly(const char* filename, bool addBackFaces = false, bool addNormal = false);
    void generateNormals();
    virtual void printSummary();

public:
    int getNumberOfIndices() const;
    int getNumberOfVertices() const;
    int getNumberOfTriangles() const;
    const Vertex& getVertex(int i) const;
    const Vector3d& getVertexPosition3d(int i) const;
    const vector<Vector3d>& getVertexPosition() const;
    const vector<Vector3d>& getVertexNormal() const;
    const int& getIndex(int i) const;
    const vector<int>& getIndices() const;
    bool enableBackFaces() const;

    Vector3 getCenter() const;
    Vector3d getCenter3d() const;
    double getWidth() const;
    double getHeight() const;
    double getLength() const;
    double getRadius() const;
    Vector3d* getAABB() const;

    char m_modelFullPath[256];

public:
    void generateVertices3d();
    void generateVerticeNormal3d();
    void generateTriangles();
    const Triangle &getTriangle(int id) const;
    const vector<Triangle> &getTriangles() const;

private:
    enum {
        PLY_PX, PLY_PY, PLY_PZ,PLY_NX,PLY_NY,PLY_NZ,
    };

    // call back functions
    static int vertex_cb(p_ply_argument argument); 

    static int index_cb(p_ply_argument argument); 

    void bounds(Vector3* center, double* width, double* height,
    double* length, double* radius, Vector3d* AABB) const;

private:
    int               m_nTriangles;
    vector<Vertex>    m_vertexBuffer;
    vector<Vector3d>  m_vertexPosition3d;
    vector<Vector3d>  m_vertexNormal3d;
    vector<int>       m_indexBuffer;
    Vector3           m_center;
    double            m_width;
    double            m_height;
    double            m_length;
    double            m_radius;
    Vector3d          m_AABB[2];
    bool              m_enableBackFaces;
    vector<Triangle>  m_triangles;
};



inline int ModelBase::getNumberOfIndices() const
{ return (int)m_indexBuffer.size(); }

inline int ModelBase::getNumberOfVertices() const
{ return (int)m_vertexBuffer.size(); }

inline int ModelBase::getNumberOfTriangles() const
{ return m_nTriangles; }

inline const ModelBase::Vertex &ModelBase::getVertex(int i) const
{ return m_vertexBuffer[i]; }

inline const Vector3d &ModelBase::getVertexPosition3d(int i) const
{ return m_vertexPosition3d[i]; }

inline const vector<Vector3d> & ModelBase::getVertexPosition() const
{
    return m_vertexPosition3d;
}
inline const vector<Vector3d>& ModelBase::getVertexNormal() const
{
    return m_vertexNormal3d;
}
inline const int& ModelBase::getIndex(int i) const
{ return m_indexBuffer[i]; }

inline bool ModelBase::enableBackFaces() const
{ return  m_enableBackFaces; }
inline const vector<int>& ModelBase::getIndices() const
{
    return m_indexBuffer;
}
