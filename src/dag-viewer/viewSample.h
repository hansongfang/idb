#ifndef VIEWSAMPLE_H
#define VIEWSAMPLE_H

#include <string>
#include <dag-lib/ModelBase.h>
#include <dag-lib/ViewBase.h>
using namespace std;

class viewSample
{
private:
    ModelBase*            _pModel;

    // representative views
    vector<Vector3d>         _triPos;
    vector<Vector2d>         _triPolars;
    int                      _nNodes;

    //indice buffers
    vector<int>              _allIndices;
    map<int, int>            _bufferOffset;
    map<int, int>            _bufferSize;
    vector<int>              _assigns;

public:
    viewSample(string model, string cacheDir);
    // render demo
    void render();

private:
    void init(string model, string cacheDir);
    // load precomputed in-depth buffers
    void loadBuffer(string cacheDir);

    // choosing buffer
    // both two function is fine to use
    int chooseBuffer2(Vector3d view);
    int chooseBuffer(Vector3d view);

    void polar2cartesian(const Vector2d& polarcoord, Vector3d& pos);

};

#endif // VIEWSAMPLE_H
