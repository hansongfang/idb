#ifndef InitViewModel_H
#define InitViewModel_H

#include <vector>
#include <math.h>
#include <set>
#include <map>
#include "SFVector.h"
#include "GraphCommon.h"
#include "DLL.h"

/*
 * Class to construct representative views
 *
 */
struct TetFace
{
    int& operator[](int i) { return d_v[i]; }
    const int& operator[](int i) const { return d_v[i]; }

    int d_v[3];   // vertex
    Vector3d d_normal; // normal
    double d_offset;    // distance to (0,0,0)
};

class TRIDAG_LIB InitViewModel
{
protected:
    Vector3d            _center;
    double              _radius;
    int                 _nSub;

    vector<Vector3d>    _vertices;
    vector<TetFace>     _faces;

protected:
    void init(const Vector3d& center, double scale, int nSplit);
    void _initIcoShape();
    void subdivide(int n);

    InitViewModel() {}// not to call this

public:
    InitViewModel(const Vector3d& center, double scale, int nSubdivision);

    const vector<Vector3d> &getVertices() const;
    int getNumberOfTriangles() const;
    vector<Vector3i> getTriIds() const;

};

#endif // InitViewModel_H
