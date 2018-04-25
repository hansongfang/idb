#ifndef DAGOCCLUDER_H
#define DAGOCCLUDER_H

#include "SFVector.h"
#include "GraphCommon.h"
#include "DLL.h"
#include <unordered_map>

/*
 * Function to compute occlusion relation of two triangles for a given view
 */

class TRIDAG_LIB DagOccluder
{
protected:
    enum RelationV2P{
        //vertex to plane
        VP_NONE = 0,
        VP_UP,
        VP_DOWN,
    };

    enum RelationT2P{
        //triangle to plane
        TP_NONE = 0,
        TP_UP,
        TP_DOWN,
        TP_CROSS
    };
    struct TriRelation{
        Vector<3, RelationV2P> vp;
        RelationT2P tp;
    };

protected:
    double   GE_EPSILON;
    double   Local_EPS;
    double   FACE_COSINE_EPS;
    double   AC_FACE_COSINE_EPS;
    double   _Parameter1;
    const vector<ViewNode *> &_viewNodes;
    const vector<Triangle> &_triangles;
    vector<bool> _cacheFacesAway;
    vector<char> _relations;
    int          _nTris;
    mutable int  _countSum;
    mutable int  _countRel;
    mutable int  _countVE;
    mutable int  _countDir;
    std::unordered_map<int,vector<Vector3d>> _cacheVENormMap;

protected:
    void init(double distance, double Parameter1 = 1000.0f);

    /*
     * utility functions
    */
    // return true if cosine is > 0.999, no chance of occlude
    inline
    bool isCoplanar(const Triangle &A, const Triangle &B) const;
    inline
    bool _isCoplanar(const Vector3d &norm1, const Vector3d &norm2) const;
    // return true if face away
    bool calFaceAway(const ViewNode* view, const Triangle& tri) const;
    inline bool _calFaceAway(const Vector3d& point, const Triangle& tri) const;
    inline bool _calFaceAway(const Triangle& facet, const Triangle& tri) const;

    bool _isOccludeSimple(const Vector3d &point, const Triangle &A, const Triangle &B, bool debug=false) const;
    bool _isOccludeSimple(const Triangle &tri, const Triangle &A, const Triangle &B, bool debug=false) const;
    //Use point and A's 2 vertices to form a cutting plane, if the plane separate B's all vertices, there is no occlude.
    bool _isNoOccludeSimpleVE(const Vector3d &point, const Triangle &A, const Triangle &B,const int& vId = -1,const int& triId = -1) const;

    int occludeDirection(const Vector3d &view, const Triangle &A, const Triangle &B) const;

    // check A occludes B or vice versa ( this is determined after confirming A and B has occlude relations )
    // return 1 if A occludes B, 2 if B occludes A
    // This use a plan to separate
    int _occludeDirection(const Vector3d &view, const Triangle &A, const Triangle &B) const;
    int assignDir(double dist) const;

    // return true if it can't be a cut. v occludes e
    inline bool isNotCutVE(const RelationV2P &v, const RelationV2P &e1, const RelationV2P &e2) const;
    // return true if it can't be a cut. e occludes v
    inline bool isNotCutEV(const RelationV2P &e1, const RelationV2P &e2, const RelationV2P &v) const;
    // v on A, e1,e2 on B. A occludes B, assign the cuttingPlane to outPlane
    //return 0 if no cutting planes
    bool findCuttingPlane(const Triangle &A, const Triangle &B, int v, int e1,
            int e2, Vector4d &outPlane, bool reverse = false, bool debug = false) const;

    // Possible occlude relation
    // 0 for no occlude, 1 for A->B, 2 for B->A, 3 for selfcycle
    inline int getPossibleRel(const TriRelation &relAB, const TriRelation &relBA) const;

    // get relation
    TriRelation calOccludeRelation(const Triangle &A, const Triangle &B, bool debug = false) const;

    // using angle
    inline RelationV2P calRelationByAngle( const Vector3d &v, const Vector3d &norm,
            const Vector3d &center, bool debug=false) const;

    // vertex and plane's angle
    inline double _calAngleSine( const Vector3d &v, const Vector3d &norm,
                          const Vector3d &center) const;

public:
    DagOccluder(const vector<ViewNode *> &viewNodes, const vector<Triangle> &triangles, double distance,
                double Parameter1 = 1000.0f);

    ~DagOccluder();
    void preprocess(int numthreads);
    void preprocessRelation(int numthreads);
    void preprocessVEMap(int vId);
    void clearVEMap(int vId);

    bool isFaceAway(int viewId, int triId) const;
    void occludeSimple(vector<int>& relations,int t1, int t2, bool debug = false) const;// using only vertices
    int occludeSimple(int viewId,int t1, int t2,bool testShareEdge) const;// only for point here
    bool isOccludeSimple(const ViewNode *view, const Triangle &A, const Triangle &B, bool debug=false) const;
    void logCountVE();// log info for VE
};

#endif // DAGOCCLUDER_H
