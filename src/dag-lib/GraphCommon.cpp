#include "GraphCommon.h"
#include "Log.h"
#include <fstream>

void Triangle::enrich()
{
    Vector3d edge1 = this->getVertex(1) - this->getVertex(0);
    Vector3d edge2 = this->getVertex(2) - this->getVertex(0);

    this->_normal = (edge1.Cross(edge2)).Normalized();
    this->_dist = (this->_vertices[0]).Dot(this->_normal);

    this->_center = Vector3d(0);
    for (int j = 0; j < 3; ++j) {
        this->_center += this->_vertices[j];
    }
    this->_center /= 3.0;
}

void Triangle::flip()
{
    // flip id;
    std::swap(_vId[0], _vId[1]);
    // reverse normal
    _normal = -_normal;
    // reverse offset
    //_offset = -_offset;
    _dist = -_dist;

    //flip vertex position
    std::swap(_vertices[0], _vertices[1]);
}

const Vector3d &Triangle::getVertex(int id) const
{
    return this->_vertices[id];
}

void Triangle::printTrianlge() const
{
    gLogInfo << "Trianlge " << endl;
    gLogInfo << this->_vId[0] << " " << this->_vId[1] << " " << this->_vId[2] << endl;

}

const Vector3d Triangle::getCenter() const
{
    Vector3d center(0,0,0);
    for (auto i = 0; i < 3; i++)
        center += _vertices[i];
    center /= 3.0;
    return center;
}

bool Triangle::shareEdge(Triangle tri) const
{
    std::vector<int> find(3,0);
    for(int i=0;i<3;i++){
        auto id = tri._vId[i];
        auto it = std::find(_vId,_vId+3,id);
        if(it != _vId+3)
            find[i] = 1;
    }
    auto share = find[0]+find[1]+find[2];
    return share == 2;
}
