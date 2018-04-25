#include "SampledTriangle.h"
#include <fstream>

SampledTriangle::SampledTriangle(int interLevel, const vector<Vector3d> &vertices, const vector<TetFace> &faces, double radius, Vector3d center)
    :_center(center)
    ,_radius(radius)
{
    _vertices.clear();
    _vertices.assign(vertices.begin(),vertices.end());
    _faces.clear();
    _faces.assign(faces.begin(),faces.end());

    initParentTri();
    subdivide(interLevel);
}

void SampledTriangle::initParentTri()
{
    _parentTri.clear();
    for(auto i=0;i<_faces.size();++i){
        _parentTri[i] = i;
        childTris[i]=std::set<int>{i};
    }
}

void SampledTriangle::subdivide(int n)
{
    if(n ==0 )
        return;
    vector<TetFace> newFaces;
    newFaces.reserve(_faces.size()*4);

    std::map<Edge,int> vertexmap;
    //refresh childTris
    std::map<int,int> parTri;
    parTri.clear();
    childTris.clear();

    for(int i=0;i<_faces.size();++i){
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
        // add the corresponding
        int triId = (int)newFaces.size();
        int parentTriId ;
        if(_parentTri.find(i) == _parentTri.end()){
            gLogInfo<<"check error!"<<"face "<<i<<" could not find parent tri";
        }
        parentTriId = _parentTri.at(i);
        parTri[triId] = parentTriId;
        parTri[triId+1] = parentTriId;
        parTri[triId+2] = parentTriId;
        parTri[triId+3] = parentTriId;

        if(childTris.find(parentTriId) == childTris.end()){
           childTris[parentTriId] = std::set<int>{triId,triId+1,triId+2,triId+3};
        }
        else{
           childTris.at(parentTriId).insert(triId);
           childTris.at(parentTriId).insert(triId+1);
           childTris.at(parentTriId).insert(triId+2);
           childTris.at(parentTriId).insert(triId+3);
        }

        //add newfaces
        for ( int j = 0; j < 3; ++j )
        {
            TetFace newface;
            newface[0] = face[j];
            newface[1] = vidx[j];
            newface[2] = vidx[(j+2)%3];
            newFaces.push_back(newface);
        }
        // add center faces
        {
            TetFace newface;
            newface[0] = vidx[0];
            newface[1] = vidx[1];
            newface[2] = vidx[2];
            newFaces.push_back(newface);
        }


    }
    _faces.assign(newFaces.begin(), newFaces.end());
    //replace parTri
    _parentTri.clear();
    _parentTri = parTri;

    subdivide(n-1);
}

void SampledTriangle::getPreClusteringMap()
{
    preClusteringMap.clear();
    for(auto it=childTris.cbegin();it!= childTris.cend();it++){
        auto triId = it->first;
        auto childTriangles = childTris.at(triId);
        std::set<int> verticeIds;
        for(auto triId : childTriangles){
            verticeIds.insert(_faces[triId].d_v[0]);
            verticeIds.insert(_faces[triId].d_v[1]);
            verticeIds.insert(_faces[triId].d_v[2]);
        }
        preClusteringMap[triId]=verticeIds;
    }
}

std::map<int,std::set<int>> SampledTriangle::getMap()
{
    getPreClusteringMap();

    return preClusteringMap;
}
