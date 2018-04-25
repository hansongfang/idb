#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <string>
#include <dag-lib/SFVector.h>
#include "Camera.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"
#include "dag-lib/ModelBase.h"

using namespace std;

class GLWindow
{
private:
    const ModelBase*          _pModel;
    int                          _width;
    int                          _height;

    GLFWwindow*                  _window;
    Camera                       _camera;
    GLuint                       _programID;
    GLuint                       _VAO;
    GLuint                       _VBO;
    GLuint                       _VBO2;
    GLuint                       _IBO;
    GLuint                       _cubeTBO;


protected:
    void installShaders(GLuint& programID, string verShaderFile, string fragShaderFile);
    string readShaderCode(const char* fileName);
    bool checkShaderStatus(GLuint shaderID);

    void saveFrontBuffer(string suffix, int viewId);
public:
    GLWindow(const ModelBase *pModel, int width, int height);
    void initializeGL();
    void buildShader();

    void setModelObject(int nIndices);
    void subLoadIndices(const vector<int> &piIndexBuffer);
    void setTexture();
    void setCamera(Vector3d cameraPos, Vector3d center);
    void setModelMat(glm::mat4& model);

    void render(int start,int nIndices, int viewId);
    void terminate();
};

#endif // GLWINDOW_H
