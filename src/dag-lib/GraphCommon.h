#ifndef GRAPHCOMMON_H
#define GRAPHCOMMON_H

#include <vector>
#include <utility>
#include "SFVector.h"
#include "DLL.h"
#include <unordered_set>

typedef std::pair<int, int> Edge;

/*
 * ViewNode, ViewNodePoint, ViewNodeTriangle and Triangle class
 *
 */
class TRIDAG_LIB Triangle
{
public:
    int _id;                // id
    int _vId[3];            // vertex id
    Vector3d _vertices[3];  // references to original vertex

    Vector3d _normal;       // triangle normal
    Vector3d _center;       // center point
    double _dist;           // dist to origin from norml

    void enrich();      // assign normal and offset
    void flip();

    void printTrianlge() const;
    const Vector3d & getVertex( int id ) const;
    const Vector3d getCenter() const;
    bool shareEdge(Triangle tri) const;
};

class TRIDAG_LIB ViewNode
{
    int _id;    // id.
    int _type;  // type. 0 for point, 1 for tri, 2 for region, 3 for voxel
public:
    ViewNode(int type=-1, int id=-1):_id(id),_type(type){}
    int getType() const {return _type;}
    int getId() const {return _id;}
};

class TRIDAG_LIB ViewNodePoint: public ViewNode
{
    Vector3d _vertex;
public:
    ViewNodePoint(const Vector3d &v, int id = -1):ViewNode(0, id), _vertex(v){}
    const Vector3d &getPoint() const { return _vertex; }
};

class TRIDAG_LIB ViewNodeTriangle: public ViewNode
{
    Triangle _tri;
public:
    ViewNodeTriangle(const Triangle &t, int id = -1):ViewNode(1, id), _tri(t){}
    const Triangle &getTriangle() const { return _tri; }
};


#endif // GRAPHCOMMON_H
