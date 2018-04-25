#include "GLWindow.h"
#include "FreeImage.h"
#include "dag-lib/Log.h"

const int VERTEX_BYTE_SIZE = sizeof(Vector3d);

GLWindow::GLWindow(const ModelBase *pModel, int width, int height)
    :_pModel(pModel)
    , _width(width)
    ,_height(height)
{
}

void GLWindow::initializeGL()
{
    /* initialize window */
    if (!glfwInit())
        return;
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    _window = glfwCreateWindow(_width, _height, "Initial window", NULL, NULL);
    if (!_window)
    {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.3?");
    }
    glfwMakeContextCurrent(_window);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;

    /* initialize glew */
    glewInit();
}

void GLWindow::buildShader()
{
    string verShaderFile =  "../src/dag-viewer/shader/vertexShaderCode.glsl";
    string fragShaderFile = "../src/dag-viewer/shader/fragmentShaderCode.glsl";
    installShaders(_programID, verShaderFile, fragShaderFile);
}

void GLWindow::installShaders(GLuint &programID, string verShaderFile, string fragShaderFile)
{
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    const char* adpater[1];
    auto cFileName = verShaderFile.c_str();
    std::string temp = readShaderCode(cFileName);
    adpater[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adpater, 0);

    cFileName = fragShaderFile.c_str();
    temp = readShaderCode(cFileName);
    adpater[0] = temp.c_str();
    glShaderSource(fragmentShaderID, 1, adpater, 0);

    /*get error message*/
    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);
    if (!checkShaderStatus(vertexShaderID)) {
        std::cout << "vertex shader problem" << std::endl;
    }
    if (!checkShaderStatus(fragmentShaderID)) {
        std::cout << "fragment shader problem" << std::endl;
    }
    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
        return;

    /*link as program*/
    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(_programID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
}

string GLWindow::readShaderCode(const char *fileName)
{
    ifstream meInput(fileName);
    if (!meInput.good()) {
        gLogInfo << "could not find file";
        exit(-1);
    }
    return string(
        std::istreambuf_iterator<char>(meInput),
        std::istreambuf_iterator<char>());
}

bool GLWindow::checkShaderStatus(GLuint shaderID)
{
    GLint compileStatus;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        GLint infoLogLens;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLens);
        GLchar* buffer = new char[infoLogLens];
        GLsizei bufferSize;
        glGetShaderInfoLog(shaderID, infoLogLens, &bufferSize, buffer);
        std::cout << buffer << std::endl;

        delete[] buffer;
        return false;
    }
    return true;
}

void GLWindow::setModelObject(int nIndices)
{
    int numVertices = _pModel->getNumberOfVertices();
    int numFaces = _pModel->getNumberOfTriangles();
    gLogInfo << "# " << numVertices << " vertices " << " # " << numFaces << " faces";

    const auto& posBuffer = _pModel->getVertexPosition();
    const auto& normBuffer = _pModel->getVertexNormal();

    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);
    /* vertex object */
    glGenBuffers(1, &_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * VERTEX_BYTE_SIZE, &(posBuffer[0][0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, VERTEX_BYTE_SIZE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1,&_VBO2);
    glBindBuffer(GL_ARRAY_BUFFER,_VBO2);
    glBufferData(GL_ARRAY_BUFFER, numVertices * VERTEX_BYTE_SIZE,(&normBuffer[0][0]),GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, VERTEX_BYTE_SIZE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_VERTEX_ARRAY, 0);
    glBindVertexArray(0);

    /* index buffer object */
    glGenBuffers(1, &_IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices* sizeof(int), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLWindow::subLoadIndices(const vector<int> &piIndexBuffer)
{
    int numFaces = (int)piIndexBuffer.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _IBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numFaces *sizeof(int), &piIndexBuffer[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLWindow::setTexture()
{
    glGenTextures(1, &_cubeTBO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeTBO);

    vector<string> fileNames;
    fileNames.push_back("../src/dag-viewer/shader/texture/right.jpg");
    fileNames.push_back("../src/dag-viewer/shader/texture/left.jpg");
    fileNames.push_back("../src/dag-viewer/shader/texture/bottom.jpg");
    fileNames.push_back("../src/dag-viewer/shader/texture/top.jpg");
    fileNames.push_back("../src/dag-viewer/shader/texture/back.jpg");
    fileNames.push_back("../src/dag-viewer/shader/texture/front.jpg");

    for (int i = 0; i < 6; i++) {
        const char* cfilename = fileNames[i].c_str();
        FREE_IMAGE_FORMAT format = FreeImage_GetFileType(cfilename, 0);
        FIBITMAP* bitmap = FreeImage_Load(format, cfilename);
        GLubyte* textureData = FreeImage_GetBits(bitmap);
        GLsizei width = FreeImage_GetWidth(bitmap);
        GLsizei height = FreeImage_GetHeight(bitmap);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData);
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTextureParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTextureParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTextureParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    glTextureParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLWindow::setCamera(Vector3d cameraPos, Vector3d center)
{
    glUseProgram(_programID);

    _camera.setViewportAspectRatio(1.0f*_width / _height);
    _camera.setFieldOfView(30.0f);
    _camera.setNearAndFarPlanes(0.1f, 100.0f);

    _camera.setPosition(glm::vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    _camera.lookAt(glm::vec3(center[0], center[1], center[2]));

    glm::mat4 view = _camera.view();
    glm::mat4 projection = _camera.projection();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view)));

    auto location = glGetUniformLocation(_programID, "modelView");
    glUniformMatrix4fv(location, 1, GL_FALSE, &view[0][0]);
    location = glGetUniformLocation(_programID, "projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
    location = glGetUniformLocation(_programID, "normalMatrix");
    glUniformMatrix3fv(location, 1, GL_FALSE, &normalMatrix[0][0]);
}

void GLWindow::setModelMat(glm::mat4 &model)
{
    glUseProgram(_programID);
    auto location = glGetUniformLocation(_programID,"model");
    glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);
}

void GLWindow::render(int start, int nIndices, int viewId)
{
    glUseProgram(_programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeTBO);
    glUniform1i(glGetUniformLocation(_programID, "ourTexture"), 0);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_IBO);
    glClearColor(1.0, 1.0, 1.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, (GLvoid*)(start*sizeof(GLuint)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glfwSwapBuffers(_window);

    string suffix = "./visual";
    saveFrontBuffer(suffix, viewId);

}

void GLWindow::terminate()
{
    /* finish drawing*/
    glfwTerminate();
    printf("terminate rendering window!\n ");
}

void GLWindow::saveFrontBuffer(string suffix, int viewId)
{
    string filename = suffix+"/view" + std::to_string(viewId) + ".png";
    GLubyte* pixelBuffer = (GLubyte*)malloc(_width*_height*3);
    glReadBuffer(GL_FRONT);
    glReadPixels(0,0,_width,_height,GL_BGR,GL_UNSIGNED_BYTE,pixelBuffer);
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixelBuffer, _width, _height, 3 * _width,
        24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    FreeImage_Save(FIF_PNG, image, filename.c_str(), 0);
    FreeImage_Unload(image);
    delete [] pixelBuffer;
}
