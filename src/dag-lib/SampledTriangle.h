#ifndef SampledTriangle_H
#define SampledTriangle_H

#include <vector>
#include <map>
#include <set>
#include "Log.h"
#include "SFVector.h"
#include "InitViewModel.h"
#include "DAG.h"

/*
 * Class to generate preclustering map<sampleTri, pointsIds>
 */
class TRIDAG_LIB SampledTriangle
{
protected:
    typedef std::pair<int, int> Edge;

    Vector3d                    _center;
    double                      _radius;
    vector<Vector3d>            _vertices;
    vector<TetFace>             _faces;
    std::map<int,int>           _parentTri;//map children tri to parent tri
    std::map<int,std::set<int>> childTris;//map parent tri to children tris
    std::map<int,std::set<int>> preClusteringMap;//map trianlge to points inside it

    void subdivide(int n);
    void initParentTri();
    void getPreClusteringMap();

public:
    SampledTriangle(int interLevel,const vector<Vector3d>& vertices,
                          const vector<TetFace>& faces, double radius, Vector3d center);
    std::map<int,std::set<int>> getMap() ;
};

#endif // SampledTriangle_H
