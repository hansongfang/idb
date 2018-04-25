#include "ModelBase.h"
#include "Log.h"

ModelBase::ModelBase(const char *filename, bool addBackFaces, bool loadNormal)
{
    this->loadPly(filename, addBackFaces,loadNormal);
}

ModelBase::~ModelBase()
{
    m_indexBuffer.clear();
    m_vertexBuffer.clear();
    m_vertexPosition3d.clear();
    m_triangles.clear();
}

void ModelBase::loadPly( const char* filename, bool addBackFaces /*= false*/,bool addNormal )
{
    strcpy(m_modelFullPath, filename);
    int nVertices, nIndices;
    p_ply ply = ply_open(filename, NULL);
    ply_read_header(ply);
    nVertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, (void*)&m_vertexBuffer, PLY_PX);
    ply_set_read_cb(ply, "vertex", "y", vertex_cb, (void*)&m_vertexBuffer, PLY_PY);
    ply_set_read_cb(ply, "vertex", "z", vertex_cb, (void*)&m_vertexBuffer, PLY_PZ);
    if(addNormal){
        ply_set_read_cb(ply, "vertex", "nx", vertex_cb, (void*)&m_vertexBuffer, PLY_NX);
        ply_set_read_cb(ply, "vertex", "ny", vertex_cb, (void*)&m_vertexBuffer, PLY_NY);
        ply_set_read_cb(ply, "vertex", "nz", vertex_cb, (void*)&m_vertexBuffer, PLY_NZ);
    }
    m_nTriangles = ply_set_read_cb(ply, "face", "vertex_indices", index_cb, (void*)&m_indexBuffer, 0);
    nIndices = 3*m_nTriangles;
    m_vertexBuffer.resize(nVertices);
    m_indexBuffer.reserve(nIndices);
    ply_read(ply);
    ply_close(ply);

    bounds(&m_center, &m_width, &m_height, &m_length, &m_radius, &m_AABB[0]);

    m_enableBackFaces = addBackFaces;
    if (addBackFaces)
    {
        m_indexBuffer.reserve(2*nIndices);
        for (int i = 0; i < m_nTriangles; ++i)
        {
            m_indexBuffer.push_back(m_indexBuffer[3*i]);
            m_indexBuffer.push_back(m_indexBuffer[3*i + 2]);
            m_indexBuffer.push_back(m_indexBuffer[3*i + 1]);
        }
        m_nTriangles *= 2;
    }
    //generateNormals();

    generateVertices3d();

    if(addNormal){
        generateVerticeNormal3d();
    }

    generateTriangles();
}


int ModelBase::vertex_cb( p_ply_argument argument )
{
    long i, j;
    vector<Vertex>* ver;
    ply_get_argument_element(argument, NULL, &i);
    ply_get_argument_user_data(argument, (void**)&ver, &j);
    j -= PLY_PX;
    (*ver)[i].position[j] = (double)ply_get_argument_value(argument);
    return 1;
}

int ModelBase::index_cb( p_ply_argument argument )
{
    long i, j;
    vector<int>* ind;
    ply_get_argument_element(argument, NULL, &i);
    ply_get_argument_property(argument, NULL, NULL, &j);
    ply_get_argument_user_data(argument, (void**)&ind, NULL);
    if(j < 0) return 1;
    ind->push_back((int)ply_get_argument_value(argument));
    return 1;
}

void ModelBase::printSummary()
{
    cout << "nVertices=" << getNumberOfIndices() << " nFaces=" << getNumberOfTriangles() << endl;
}

void ModelBase::bounds(Vector3* center, double* width, double* height,
                     double* length, double* radius, Vector3d* AABB) const
{
    /*float xMax = std::numeric_limits<float>::min();
    float yMax = std::numeric_limits<float>::min();
    float zMax = std::numeric_limits<float>::min();*/
    double xMax = -std::numeric_limits<double>::max();
    double yMax = -std::numeric_limits<double>::max();
    double zMax = -std::numeric_limits<double>::max();

    double xMin = std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double zMin = std::numeric_limits<double>::max();

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    int numVerts = static_cast<int>(m_vertexBuffer.size());

    for (int i = 0; i < numVerts; ++i)
    {
        x = m_vertexBuffer[i].position[0];
        y = m_vertexBuffer[i].position[1];
        z = m_vertexBuffer[i].position[2];

        if (x < xMin)
            xMin = x;

        if (x > xMax)
            xMax = x;

        if (y < yMin)
            yMin = y;

        if (y > yMax)
            yMax = y;

        if (z < zMin)
            zMin = z;

        if (z > zMax)
            zMax = z;
    }

    (*center)[0] = (xMin + xMax) / 2.0;
    (*center)[1] = (yMin + yMax) / 2.0;
    (*center)[2] = (zMin + zMax) / 2.0;

    *width = xMax - xMin;
    *height = yMax - yMin;
    *length = zMax - zMin;

    AABB[0] = Vector3d(xMin, yMin, zMin);
    AABB[1] = Vector3d(xMax, yMax, zMax);
    //*radius = std::max(std::max(*width, *height), *length);
    *radius = sqrt(pow(*width, 2.0)+pow(*height,2.0)+pow(*length,2.0))/2.0;
}

Vector3 ModelBase::getCenter() const
{return m_center;}

Vector3d ModelBase::getCenter3d() const
{return Vector3d(m_center[0],m_center[1],m_center[2]);}

double ModelBase::getWidth() const
{return m_width;}

double ModelBase::getHeight() const
{return m_height;}

double ModelBase::getLength() const
{return m_length;}

double ModelBase::getRadius() const
{return m_radius;}

void ModelBase::generateNormals()
{
    int totalTriangles = getNumberOfTriangles();
    if ( m_enableBackFaces )
        totalTriangles /= 2;
    
    int totalVertices = getNumberOfVertices();
    const int *pTriangle = 0;
    Vertex *pVertex0 = 0;
    Vertex *pVertex1 = 0;
    Vertex *pVertex2 = 0;
    double edge1[3] = {0.0f, 0.0f, 0.0f};
    double edge2[3] = {0.0f, 0.0f, 0.0f};
    double normal[3] = {0.0f, 0.0f, 0.0f};
    double length = 0.0f;

    // Initialize all the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];
        pVertex0->normal[0] = 0.0f;
        pVertex0->normal[1] = 0.0f;
        pVertex0->normal[2] = 0.0f;
    }

    // Calculate the vertex normals.
    for (int i = 0; i < totalTriangles; ++i)
    {
        pTriangle = &m_indexBuffer[i * 3];

        pVertex0 = &m_vertexBuffer[pTriangle[0]];
        pVertex1 = &m_vertexBuffer[pTriangle[1]];
        pVertex2 = &m_vertexBuffer[pTriangle[2]];

        // Calculate triangle face normal.

        edge1[0] = pVertex1->position[0] - pVertex0->position[0];
        edge1[1] = pVertex1->position[1] - pVertex0->position[1];
        edge1[2] = pVertex1->position[2] - pVertex0->position[2];

        edge2[0] = pVertex2->position[0] - pVertex0->position[0];
        edge2[1] = pVertex2->position[1] - pVertex0->position[1];
        edge2[2] = pVertex2->position[2] - pVertex0->position[2];

        normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
        normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
        normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

        // Accumulate the normals.

        pVertex0->normal[0] += normal[0];
        pVertex0->normal[1] += normal[1];
        pVertex0->normal[2] += normal[2];

        pVertex1->normal[0] += normal[0];
        pVertex1->normal[1] += normal[1];
        pVertex1->normal[2] += normal[2];

        pVertex2->normal[0] += normal[0];
        pVertex2->normal[1] += normal[1];
        pVertex2->normal[2] += normal[2];
    }

    // Normalize the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];

        length = 1.0f / sqrtf(pVertex0->normal[0] * pVertex0->normal[0] +
            pVertex0->normal[1] * pVertex0->normal[1] +
            pVertex0->normal[2] * pVertex0->normal[2]);

        pVertex0->normal[0] *= length;
        pVertex0->normal[1] *= length;
        pVertex0->normal[2] *= length;
    }
}

Vector3d* ModelBase::getAABB() const
{
    return (Vector3d*)&m_AABB[0];
}

void ModelBase::generateVertices3d()
{
    m_vertexPosition3d.reserve(m_vertexBuffer.size());
    for (const auto &v: m_vertexBuffer) {
        const auto &pos = v.position;
        m_vertexPosition3d.push_back( Vector3d(pos[0], pos[1], pos[2]) );
    }
}

void ModelBase::generateVerticeNormal3d()
{
    m_vertexNormal3d.reserve(m_vertexBuffer.size());
    for (const auto &v: m_vertexBuffer) {
        const auto &normal = v.normal;
        m_vertexNormal3d.push_back( Vector3d(normal[0], normal[1], normal[2]) );
    }
}

void ModelBase::generateTriangles()
{
    m_triangles.resize(m_nTriangles);
    for (int id = 0; id < m_nTriangles; ++id) {
        Triangle &tri = m_triangles[id];
        tri._id = id;

        for (auto i = 0; i < 3; ++i)
        {
            auto idx = this->getIndex(id * 3 + i);
            tri._vId[i] = idx;

            const auto &v =  this->getVertex(idx).position;
            tri._vertices[i] = Vector3d(v[0], v[1], v[2]);

            tri.enrich();
        }
    }
}

const Triangle &ModelBase::getTriangle(int id) const
{
    return m_triangles[id];
}

const vector<Triangle> &ModelBase::getTriangles() const
{
    return m_triangles;
}
