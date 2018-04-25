#include "Occluder.h"
#include "Log.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <omp.h>
#include <stdexcept>

namespace fs = boost::filesystem;

DagOccluder::DagOccluder(
        const vector<ViewNode *> &viewNodes, const vector<Triangle> &triangles,
        double distance, double Parameter1)
    :_viewNodes(viewNodes)
    ,_triangles(triangles)
    ,_countSum(0)
    ,_countVE(0)
    ,_countDir(0)
    ,_countRel(0)
{
    init(distance, Parameter1);
    _nTris = (int)_triangles.size();
    auto nViewNodes = _viewNodes.size();
    _cacheVENormMap.clear();
    for(auto vId=0;vId<nViewNodes;vId++){
        _cacheVENormMap[vId] = vector<Vector3d>{};
    }
}

DagOccluder::~DagOccluder()
{
}

void DagOccluder::init(double distance, double Parameter1)
{
    gLogInfo << "distance " << distance;
    gLogInfo << "_Parameter1 " << Parameter1;

    _Parameter1 = Parameter1;
    GE_EPSILON = distance / Parameter1;
    Local_EPS = 1e-5;
    FACE_COSINE_EPS = 0.999;
    AC_FACE_COSINE_EPS = 0.9995;

    gLogInfo << "GE_EPSILON: " << GE_EPSILON << endl;
    gLogInfo << "Local_EPS: " << Local_EPS << endl;
    gLogInfo << "FACE_COSINE_EPS: " << FACE_COSINE_EPS << endl;
    gLogInfo<<"AC_FACE_COSINE_EPS: "<<AC_FACE_COSINE_EPS;
}

void DagOccluder::preprocess(int numthreads)
{
    gLogInfo << "Calculating face away stats";
    // Save relations of view to triangle
    auto nViewNodes = _viewNodes.size();
    auto nTriangles = _triangles.size();

    //TODO: allocated size may have limit here
    _cacheFacesAway.clear();
    _cacheFacesAway.resize( nTriangles*nViewNodes, false );
    gLogInfo<<"cacheFaceAway Size "<<_cacheFacesAway.size()<< nTriangles*nViewNodes;

    omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(numthreads-1);
    #pragma omp parallel for
    for (auto i = 0; i < nViewNodes; ++i) {
        const auto &view = _viewNodes[i];
        for (auto j = 0; j < nTriangles; ++j) {
            const auto &tri = _triangles[j];
            unsigned id = i*nTriangles+j;
            _cacheFacesAway[id] = calFaceAway(view, tri);
        }
    }
    gLogInfo << "finish calculating face away stats";
}


void DagOccluder::clearVEMap(int vId)
{
    _cacheVENormMap.at(vId).clear();
    _cacheVENormMap.at(vId) = vector<Vector3d>{};
}

void DagOccluder::preprocessVEMap(int vId)
{
    const auto &view = _viewNodes[vId];
    auto& point = static_cast<const ViewNodePoint*>(view)->getPoint();

    unsigned cap = _nTris*3;
    _cacheVENormMap.at(vId).resize(cap,Vector3d(0,0,0));
    for (auto j = 0; j < _nTris; ++j) {
        const auto &A = _triangles[j];
        for(auto id =0;id<3;++id){
            Vector3d v1 = A.getVertex(id);
            Vector3d v2 = A.getVertex((id+1)%3);
            Vector3d dir1 = v1 - point;       // direction from view to v1
            Vector3d dir2 = v2 - point;       // direction from view to v2
            // plane norm
            auto norm = (dir1.Cross(dir2)).Normalized();
            unsigned int uId = j*3+id;
            _cacheVENormMap.at(vId).at(uId) = norm;
        }
    }
}


void DagOccluder::preprocessRelation(int numthreads)
{
    gLogInfo << "Calculating triangle pair relation";
    auto nTriangles = _triangles.size();
    _relations.clear();
    try{
         _relations.resize(nTriangles*nTriangles+1,'4');
    }
    catch(std::bad_alloc const&)
    {
        gLogError<<"memory allocation failed!";
        gLogInfo<<"max_size: "<<_relations.max_size();
    }

    omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(numthreads-1);
#pragma omp parallel for
    for(int t1 =0;t1<nTriangles;t1++){
        const Triangle &A = _triangles[t1];
        for(int t2 =t1+1;t2<nTriangles;t2++){
            const Triangle &B = _triangles[t2];
            unsigned id = t1*nTriangles + t2;
            if(isCoplanar(A,B)){
                _relations[id] = '0';
            }
            TriRelation relAB = calOccludeRelation(A, B);
            TriRelation relBA = calOccludeRelation(B, A);
            auto possibleRel = getPossibleRel(relAB, relBA);
            if(possibleRel == 0)
                _relations[id] = '0';
            else if(possibleRel == 1)
                _relations[id] = '1';
            else if(possibleRel == 2)
                _relations[id] = '2';
            else if(possibleRel == 3)
                _relations[id] = '3';
        }
    }
    gLogInfo<<"finish calculating triangle relaiton";
}

bool DagOccluder::calFaceAway(const ViewNode *view, const Triangle &tri) const
{
    if (view->getType() == 1)
        return _calFaceAway(static_cast<const ViewNodeTriangle*>(view)->getTriangle(), tri);
    else if (view->getType() == 0)
        return _calFaceAway(static_cast<const ViewNodePoint*>(view)->getPoint(), tri);
    return false;
}

bool DagOccluder::_calFaceAway(const Vector3d &point, const Triangle &tri) const
{
    return (point - tri.getVertex(0) ).Dot(tri._normal)< GE_EPSILON;
}

bool DagOccluder::_calFaceAway(const Triangle &facet, const Triangle &tri) const
{
    for (const auto& v:facet._vertices) {
        if (!this->_calFaceAway(v, tri))
            return false;
    }
    return true;
}

bool DagOccluder::isFaceAway(int viewId, int triId) const
{
    unsigned int id = viewId*_nTris + triId;
    return _cacheFacesAway[id];
}

bool DagOccluder::isOccludeSimple(const ViewNode *view, const Triangle &A, const Triangle &B, bool debug) const
{
    if (view->getType() == 1)
        return _isOccludeSimple(static_cast<const ViewNodeTriangle*>(view)->getTriangle(), A, B, debug);
    else if (view->getType() == 0)
        return _isOccludeSimple(static_cast<const ViewNodePoint*>(view)->getPoint(), A, B, debug);
    return 0;
}

bool DagOccluder::_isOccludeSimple(const Vector3d &point, const Triangle &A, const Triangle &B, bool debug) const
{
    // vertices might not seeing both two triangles
    // add check faceaway for ViewNodePoint
	if (_calFaceAway(point,A) || _calFaceAway(point,B))
		return false;
    // Assumption: proved later in the paper
    // If 2 triangles are not occluded wrt view point,
    // true if and only if there exists a cutting plane,
    // formed with view point and 2 vertices from A or 2 vertices from B which can separate A and B.
    bool ab = _isNoOccludeSimpleVE(point, A, B);
    bool ba = _isNoOccludeSimpleVE(point, B, A);
    if (debug)
        gLogDebug << "viewPlaneTest "<<ab << " " << ba;
    if ( ab || ba ) {
        return false;
    }

    // has occlusion, A may occlude B or B may occlude A.
    auto dir = occludeDirection(point, A, B);
    // if occludeDirection return 0
    gLogDebug<<"occludeDirection "<<dir;
    return (dir==1);
}

bool DagOccluder::_isOccludeSimple(const Triangle &tri, const Triangle &A, const Triangle &B, bool debug) const
{
    //TODO: implement
    for (const auto& v : tri._vertices) {
        auto r = this->_isOccludeSimple(v, A, B, debug);
        if (debug)
            gLogError << "HEHE " << r;
        if (r) {
            if (debug)
                gLogDebug << "view: " << v;
            return r;
        }
    }
    return false;
}

bool DagOccluder::_isNoOccludeSimpleVE(const Vector3d &point, const Triangle &A, const Triangle &B,const int& vId,const int& triId) const
{
    for (auto id = 0; id < 3; ++id) {
//        Vector3d v1 = A.getVertex(id);
//        Vector3d v2 = A.getVertex((id+1)%3);
//        Vector3d dir1 = v1 - point;       // direction from view to v1
//        Vector3d dir2 = v2 - point;       // direction from view to v2

//        // plane norm
//        auto norm = (dir1.Cross(dir2)).Normalized();

        unsigned int uId = triId*3+id;
        const auto& norm = _cacheVENormMap.at(vId).at(uId);
        bool test = true;
        for (const auto &v: B._vertices) {
            if ( ( v - point ).Dot(norm) < -GE_EPSILON ) {
                test = false;
                break;
            }
        }
        if (test)
            return true;
    }
    return false;
}

int DagOccluder::occludeDirection(const Vector3d &view, const Triangle &A, const Triangle &B) const
{
    auto dir = _occludeDirection(view, A, B);
    if ( dir != 0 )
        return dir;

    dir = _occludeDirection(view, B, A);
    if (dir != 0)
        return 3-dir;

    return 0;
}

int DagOccluder::_occludeDirection(const Vector3d &view, const Triangle &A, const Triangle &B) const
{
    //TODO: Below codes can be optimized
    // find a plane that can separate A and B, check view's side
    // use edge in A and vertice in B for the plane
    for (int id = 0; id < 3; ++id) {
        // if one vertex in A exists in B, continue
        bool shared = false;
        for(int i = 0; i < 3; ++i){
            if (B._vId[i] == A._vId[id]) {
                shared = true;
                break;
            }
        }
        if (shared)
            continue;

        const Vector3d &v0 = A._vertices[id];
        const Vector3d &v1 = A._vertices[(id+1)%3];
        const Vector3d &v2 = A._vertices[(id+2)%3];

        for (int i = 0; i < 3; ++i) {
            //plane normal
            const Vector3d &u0 = B._vertices[i];
            const Vector3d &u1 = B._vertices[(i+1)%3];
            const Vector3d &u2 = B._vertices[(i+2)%3];

            // use A as the plane
            bool useA = false;
            for (int j = 0; j < 3; ++j) {
                // if vertice exists in A, use special check, plane is the same as A
                if (B._vId[i] == A._vId[j] ) {
                    useA = true;
                    break;
                }
            }

            auto normal = A._normal;
            if (!useA) {
                //plane use u0, v1, v2 to calculate
                Vector3d dir1 = (v1-u0);
                Vector3d dir2 = (v2-u0);
                normal = ( dir1.Cross(dir2) ).Normalized();
            }

            auto dirView = assignDir((view-u0).Dot(normal));
            if (dirView == 0)
                continue;

            auto dirA = assignDir((v0-u0).Dot(normal));
            auto dirB1 = assignDir((u1-u0).Dot(normal));
            auto dirB2 = assignDir((u2-u0).Dot(normal));

            if( dirB1==0 && dirB2==0) {
                if (dirA==0) {
                    gLogError << "2 triangles are too close!";
                    return 0;
                }
                else {
                    // if A and view are one the same side, A occludes B
                    return dirA==dirView?1:2;
                }
            }
            else if (dirB1 == dirB2) {//dirB1== dirB2
                if (dirA==0) {
                    // if A and view are one the same side, A occludes B
                    return dirB1!=dirView?1:2;
                }
                else if (dirA!=dirB1){
                    return dirA==dirView?1:2;
                }else {
                    continue;
                }
            }
            else if (dirB1==0 || dirB2==0){
                auto dirB = dirB1 + dirB2;
                if (dirA==0) {
                    return dirB!=dirView?1:2;
                }
                else if (dirA != dirB) {
                    return dirA==dirView?1:2;
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    return 0;
}

int DagOccluder::assignDir(double dist) const {
    if (dist > Local_EPS) {
        return 1;
    }else if (dist < -Local_EPS){
        return -1;
    }else
        return 0;
}

bool DagOccluder::isCoplanar(const Triangle &A, const Triangle &B) const
{
    return _isCoplanar( A._normal, B._normal);
}

bool DagOccluder::_isCoplanar(const Vector3d &norm1, const Vector3d &norm2) const
{
    //return norm1.Dot(norm2) < -AC_FACE_COSINE_EPS;
    return norm1.Dot(norm2) < -FACE_COSINE_EPS;
}

DagOccluder::TriRelation DagOccluder::calOccludeRelation(const Triangle &A, const Triangle &B, bool debug) const
{
    TriRelation res;
    bool noUp = true, noDown = true;
    for (auto i = 0; i < 3; ++i) {
        res.vp[i] = calRelationByAngle( A.getVertex(i), B._normal, B._center, debug );
        if ( res.vp[i] == VP_UP )
            noUp = false;
        if ( res.vp[i] == VP_DOWN )
            noDown = false;
    }

    if (noUp && noDown) // A and B on the same plane
        res.tp = TP_NONE;
    else if (noUp)
        res.tp = TP_DOWN;
    else if (noDown)
        res.tp = TP_UP;
    else
        res.tp = TP_CROSS;
    return res;
}

double DagOccluder::_calAngleSine(const Vector3d &v, const Vector3d &norm, const Vector3d &center) const
{
    auto dir = v - center;
    if (fabs( dir.Len() )<0.0001)
        return 0;
    return ((v-center).Normalized()).Dot(norm);
}

bool DagOccluder::isNotCutVE(const DagOccluder::RelationV2P &v, const DagOccluder::RelationV2P &e1, const DagOccluder::RelationV2P &e2) const
{
    // if v is not up, can't be a cutting plane
    if (v != VP_UP )
        return true;

    // if no e1/2 is down, can't be a cutting plane
    if ( e1 != VP_DOWN && e2 != VP_DOWN )
        return true;

    return false;
}

bool DagOccluder::isNotCutEV(const DagOccluder::RelationV2P &e1, const DagOccluder::RelationV2P &e2, const DagOccluder::RelationV2P &v) const
{
    // if no e1/2 is up, can't be a cutting plane
    if ( e1 != VP_UP && e2 != VP_UP )
        return true;

    // if v is not down, can't be a cutting plane
    if (v != VP_DOWN)
        return true;

    return false;
}

bool DagOccluder::findCuttingPlane(const Triangle &A, const Triangle &B, int v, int e1, int e2, Vector4d &outPlane, bool reverse, bool debug) const
{
    if (debug) {
        gLogDebug << "v: " << v << ". e: " << e1 << ", " << e2;
    }
    //find cutting plane
    const auto &v0 = A.getVertex(v);
    const auto &u1 = B.getVertex(e1);
    const auto &u2 = B.getVertex(e2);

    // plane
    Vector3d dir1 = (u1-v0);
    Vector3d dir2 = (u2-v0);
    auto norm = ( dir1.Cross(dir2) ).Normalized();
    auto dist = v0.Dot(norm);


    // check if other vertices are split on different side
    // encode 3 relations in to 3 digits
    int signCode = 0;
    for (int i = 0; i < 3; ++i) {
        if ( (i != e1) && (i != e2) ) {
            signCode += 100 * calRelationByAngle( B.getVertex(i), norm, v0, debug );
            break;
        }
    }
    auto a1 = (v + 1) % 3;
    signCode += 10 * calRelationByAngle( A.getVertex(a1), norm, u1, debug ); // or use (u1+u2)/2 for u1?
    auto a2 = (v + 2) % 3;
    signCode += calRelationByAngle( A.getVertex(a2), norm, u1, debug );

    bool flip = false;
    //27 cases here
    switch(signCode) {
    case 0: // 0 0 0
    case 12: // 0 1 2
    case 21: // 0 2 1
    case 110: // 1 1 0
    case 101: // 1 0 1
    case 112: // 1 1 2
    case 121: // 1 2 1
    case 111: // 1 1 1
    case 220: // 2 2 0
    case 202: // 2 0 2
    case 221: // 2 2 1
    case 212: // 2 1 2
    case 222: // 2 2 2
        return false;
    case 100: // 1 0 0
    case 120: // 1 2 0
    case 102: // 1 0 2
    case 122: // 1 2 2
    case 2: // 0 0 2
    case 20: // 0 2 0
    case 22: // 0 2 2
        flip = true;
        break;
    case 200: // 2 0 0
    case 201: // 2 0 1
    case 210: // 2 1 0
    case 211: // 2 1 1
    case 1: // 0 0 1
    case 10: // 0 1 0
    case 11: // 0 1 1
        break;
    default:
        gLogFatal << "No such case";
        //exit(-1);
    }

    outPlane = Vector4d(norm, dist);
    if ((reverse && !flip) || (!reverse && flip)) {
        outPlane = -outPlane;
        norm = -norm;
    }

    if (debug) {
        gLogDebug << "Added plane: " << outPlane << ". flip: " << flip;
        gLogDebug << "Angle: A cutPlane:" << norm.Dot(A._normal) ;
        gLogDebug << "Angle: B cutPlane:" << norm.Dot(B._normal) ;
    }
    return true;
}

DagOccluder::RelationV2P DagOccluder::calRelationByAngle(const Vector3d &v, const Vector3d &norm, const Vector3d &center, bool debug) const
{
    auto angle = _calAngleSine(v, norm, center);
    if (debug) {
        gLogDebug << "calRelationByAngle: " << angle;
    }
    if (angle > 1-FACE_COSINE_EPS)
        return VP_UP;
    else if (angle < -(1-FACE_COSINE_EPS))
        return VP_DOWN;
    else
        return VP_NONE;
}

int DagOccluder::getPossibleRel(
        const TriRelation &relAB,
        const TriRelation &relBA) const
{
    int code = relAB.tp*10 + relBA.tp;
    switch(code) {
    // If A and B are on the same plane
    case 0: //0 0
    case 1:   //0 1
    case 10: //1 0
    case 2:  //0 2
    case 20: //2 0
    case 3:  //0 3
    case 30: //3 0

    // If A up B and B up A, no occlude
    case 11:
    // If A down B and B down A, no occlude
    case 22:
        return 0;

    // If A up B and B down/cross A, A *could* occludes B
    case 12:
    case 13:
    // If B below A and A up/cross B, A *could* occludes B
    case 32:
        return 1;

    // If A below B and B above/cross A, B *could* occludes A
    case 21:
    case 23:
    // If B up A and A down/cross B, B *could* occludes A
    case 31:
        return 2;

    // if A cross B and B cross A
    case 33:
        return 3;
    }

    gLogFatal << "Logic fault! Please check";
    //exit(-1);
    return 4;
}

void DagOccluder::occludeSimple(vector<int> &res,int t1, int t2, bool debug) const
{
    std::fill(res.begin(), res.end(), 0);
    const Triangle &A = _triangles[t1];
    const Triangle &B = _triangles[t2];
    auto nViewNodes = _viewNodes.size();

    if (isCoplanar(A, B)) {
        if (debug)
            gLogDebug << "Coplanar: " << t1 << ", " << t2;
        return ;
    }

    TriRelation relAB = calOccludeRelation(A, B);
    TriRelation relBA = calOccludeRelation(B, A);

    auto possibleRel = getPossibleRel(relAB, relBA);
    if (possibleRel == 0) {
        if (debug){
            gLogDebug << "possibleRel: " << possibleRel << ". " << t1 << ", " << t2;
            gLogDebug << relAB.vp << " " << relAB.tp;
            gLogDebug << relBA.vp << " " << relBA.tp;
        }
        return ;
    }

    for (auto i = 0; i < nViewNodes; ++i) {
        if(isFaceAway(i, t1) || isFaceAway(i, t2)) {
            if(debug)
                gLogDebug << i << ": " << t1 << ", " << t2 << ". " << isFaceAway(i, t1) << "|" << isFaceAway(i, t2);
            continue;
        }

        const auto &view = _viewNodes[i];
        if (possibleRel == 1){
            if ( isOccludeSimple(view, A, B) )
                res[i] = 1;
            // else res[i] = 0;
        }
        else if (possibleRel == 2){
            if ( isOccludeSimple(view, B, A) )
                res[i] = 2;
            // else res[i] = 0;
        }
        else { // 3
            auto ab = isOccludeSimple(view, A, B);
            auto ba = isOccludeSimple(view, B, A);
            if (ab && ba) {
                gLogError << "Error with self cycle, please check! " << view->getId() << ", " << A._id << ", " << B._id;
                //debugOcclude(view, A, B);
                res[i] = 3; // ERROR, should not happen
            }
            else if (ab)
                res[i] = 1;
            else if (ba)
                res[i] = 2;
            //else res[i] = 0;
        }
    }

    return ;
}

int DagOccluder::occludeSimple(int viewId,int t1, int t2, bool testShareEdge) const
{
    const auto &view = _viewNodes[viewId];
//    if(view->getType() != 0){
//        gLogError<<"This function only support point only! ";
//        exit(-1);
//    }
    auto& point = static_cast<const ViewNodePoint*>(view)->getPoint();

    const Triangle &A = _triangles[t1];
    const Triangle &B = _triangles[t2];

    unsigned int uId = t1*_nTris + t2;
    auto possibleRel = _relations[uId];

    if(possibleRel == '0')
        return 0;
    bool ab = _isNoOccludeSimpleVE(point, A, B,viewId,t1);
    if(ab)
        return 0;

    bool ba = _isNoOccludeSimpleVE(point, B, A,viewId,t2);

    if (ba) {
        return 0;
    }

    if(possibleRel == '1')
        return 1;
    else if(possibleRel == '2')
        return 2;

    // has occlusion, A may occlude B or B may occlude A.
    auto dir = occludeDirection(point, A, B);
    return dir;
}

void DagOccluder::logCountVE()
{
    gLogInfo<<"count Sum "<<_countSum;
    gLogInfo<<"count Rel "<<_countSum;
    gLogInfo<<"count VE "<<_countVE;
    gLogInfo<<"count Dir "<<_countDir;
    gLogInfo<<"percentige Sum"<<(float)_countSum*2 / (_nTris*_nTris);
    gLogInfo<<"percentige VE"<<(float)_countVE*2 / (_nTris*_nTris);
    gLogInfo<<"percentige Dir"<<(float)_countDir*2 / (_nTris*_nTris);
    _countSum = 0;
    _countRel = 0;
    _countVE = 0;
    _countDir = 0;
}
