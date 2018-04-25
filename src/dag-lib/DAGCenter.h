#ifndef DAGCENTER_H
#define DAGCENTER_H

#include <vector>
#include <string>
#include "DAG.h"
#include "ViewBase.h"
#include "DLL.h"
#include <unordered_map>
#include <map>
#include <set>
/*
 * Calculate and save DAGs for all views
 *
 * notes: For sampled view, we caculate DAG for viewpoint first
 *        and preclustering to generate DAG for sampled triangle
 */
class TRIDAG_LIB DAGCenter
{
    typedef std::pair<int, int>    Edge;
    int                            _nTris; // num tris of model
    int                            _nPointDags;
    std::vector<DAG*>              _pointDags;
    int                            _nDags;
    std::vector<DAG*>              _dags;

    string                         _cacheFolder; // cache dir
    int                            _numFailed; //DAG with cycle


protected:
    std::string getInitFilePath( std::string cacheFolder ) const;
public:
    DAGCenter( int nTris);
    ~DAGCenter();

    void initPointDags(int nPointDags);
    void initDags(int nDags);

    DAG* getPointDag(int id); //return pointer to pointDAG
    const DAG *getDag(int id) const; // return pointer to sampleTriDAG
    DAG* getDag2(int id);


    // clean
    void cleanPointDags();
    void cleanDag(int id);

    bool load( std::string cacheFolder ); //load DAG from cacheDir
    void save( std::string cacheFolder ); // save DAG to cacheDir
};

#endif // DAGCENTER_H
