#include "viewSample.h"
#include "dag-lib/Log.h"
#include <dag-lib/ViewFacet.h>
#include <random>
#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <math.h>
#include "GLWindow.h"
#define PI 3.14159265

viewSample::viewSample(string model, string cacheDir)
    :_pModel(0)
    ,_nNodes(0)
{
    _allIndices.clear();
    _bufferOffset.clear();
    _bufferSize.clear();
    _assigns.clear();
    _triPos.clear();
    _triPolars.clear();

    init(model, cacheDir);
}

void viewSample::init(string model, string cacheDir)
{
    _pModel = new ModelBase( model.c_str(), true , true);

    this->loadBuffer(cacheDir);
}

void viewSample::render()
{
    //generate test views
    vector<Vector3d> testViews;
    double radius = _pModel->getRadius() * 3.0;
    for(auto i=0;i<900;i++){
        double theta = ((float) i/900.0) * PI*2;
        double phi = 0.0;
        double z = radius*cos(theta);
        double x = radius*sin(theta)*cos(phi);
        double y = radius*sin(theta)*sin(phi);
        Vector3d pos = Vector3d(x,y,z)+_pModel->getCenter3d();
        testViews.push_back(pos);
    }


    GLWindow renderWindow(_pModel, 800, 600);
    renderWindow.initializeGL();
    renderWindow.buildShader();
    int nIndices = (int)_allIndices.size();
    renderWindow.setModelObject(nIndices);
    renderWindow.setTexture();
    glm::mat4 model;
    renderWindow.setModelMat(model);

    renderWindow.subLoadIndices(_allIndices);
    auto center = _pModel->getCenter3d();
    for(int k=0;k<900;k++){
        auto testview = testViews[k];
        renderWindow.setCamera(testview, center);
        int viewId = chooseBuffer(testview - center);
        int bufferId = _assigns[viewId];
        renderWindow.render(_bufferOffset[bufferId], _bufferSize[bufferId], k);
    }
    renderWindow.terminate();
}

void viewSample::loadBuffer(string cacheDir)
{
    // load assignments
    string filename = cacheDir +"assignments.txt";
    ifstream ifs(filename);
    if(ifs.is_open()){
        double theta, phi;
        int bufferId;
        while (!ifs.eof() ){
            ifs >> bufferId >> theta >> phi;
            _assigns.push_back(bufferId);
            _triPolars.push_back(Vector2d(theta, phi));
        }
    }else{
        gLogError<<" assignment file not found "<<filename;
    }
    ifs.close();
    set<int> bufferIds(_assigns.begin(), _assigns.end());

    _nNodes = (int)_assigns.size();
    _triPos.resize(_nNodes, Vector3d(0,0,0));
    for(auto k=0;k<_nNodes;k++)
        polar2cartesian(_triPolars[k], _triPos[k]);

    // load indice buffers
    int nFaces3 = _pModel->getNumberOfIndices();
    int nBuffers = (int)bufferIds.size();
    _allIndices.reserve(nBuffers * nFaces3);
    int count, offset;
    for(auto bufferId:bufferIds){
        offset =(int) _allIndices.size();
        count =0 ;
        string indiceFile = cacheDir+"/Indices_" + to_string(bufferId) + ".txt";
        ifs.open(indiceFile);
        if(ifs.is_open()){
            int id;
            while(ifs >> id){
                _allIndices.push_back(id);
                count ++;
            }
        }
        else{
            gLogError<<"indice file not fund! "<<indiceFile;
        }
        ifs.close();
        _bufferSize[bufferId]= count;
        _bufferOffset[bufferId] = offset;
    }
}

// iterate all representative viewpoints
int viewSample::chooseBuffer2(Vector3d view)
{
    view.Normalize();
    int id =0;
    float maxdist = -1.0;
    for(int i=0;i<_nNodes;i++){
        double dist = view.Dot(_triPos[i]);
        if(dist > maxdist){
            maxdist = dist;
            id = i;
        }
    }
    return id;
}

// iterate viewpoints with limited theta range
int viewSample::chooseBuffer(Vector3d view)
{
    view.Normalize();
    double theta = acos(view[2]);
    int id =0;
    float maxdist = -1.0;
    for(int i=0;i<_nNodes;i++){
        if(abs(theta - _triPolars[i][0]) > PI/10)
            continue;
        double dist = view.Dot(_triPos[i]);
        if(dist > maxdist){
            maxdist = dist;
            id = i;
        }
    }
    return id;
}

void viewSample::polar2cartesian(const Vector2d& polarcoord, Vector3d& pos)
{
    pos[0] = sin(polarcoord[0])*cos(polarcoord[1]);
    pos[1] = sin(polarcoord[0])*sin(polarcoord[1]);
    pos[2] = cos(polarcoord[0]);
}



