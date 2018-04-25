#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "dag-lib/ModelBase.h"
#include "dag-lib/Log.h"
#include "dag-lib/appcommon.h"
#include "BufferMaker.h"
#include "time.h"
#include "DAGMaker.h"
#include "dag-lib/DAGCenter.h"

int main(int argc, char *argv[])
{
    //initLogging("merger");

    po::variables_map vm;
    try {
        po::options_description app_desc("General.");
        app_desc.add_options()
            ("help,h", "produce help message")
            ("gui,g", po::bool_switch()->default_value(false), "start up gui")
        ;

        po::options_description psb_desc("trianglediag");
        psb_desc.add_options()
            ("model,m", po::value< std::string >(),"input model path")
            ("sub,s", po::value< int >()->default_value(0),"Subdivide level")
            ("debug,d", po::bool_switch()->default_value(false), "show debug info")
            ("cacheRoot,c", po::value< std::string >()->default_value("./cache"),"Cache root path. default ./cache")
            ("EpsilonPara,e", po::value< double >()->default_value(1000.0), "Epsilon over paramter. default 3000.0")
            ("priorityMetric,p",po::value< int >()->default_value(1),"Defualt 1. 1 for max share Edges")
            ("iteration,i",po::value< int >()->default_value(10),"Defualt 10 for iteration")
            ("innerSub,l",po::value< int >()->default_value(-1),"Defualt inner sub division level -1")
        ;

        po::options_description cmd_desc("Command arguments");
        cmd_desc.add(app_desc).add(psb_desc);

        po::store(po::parse_command_line(argc, argv, cmd_desc), vm);
        po::notify(vm);

        if (vm.count("gui") && vm["gui"].as<bool>()) {
            gLogInfo << "Start gui. " << vm["gui"].as<bool>();
        }
    }
    catch(std::exception& e) {
        gLogError << "error: " << e.what();
        return 1;
    }
    catch(...) {
        gLogError << "Exception of unknown type!";
    }

    toggleDebug( vm["debug"].as<bool>() );

    auto model = vm["model"].as<std::string>();
    auto sub = vm["sub"].as<int>();
    auto innerSub = vm["innerSub"].as<int>();
    auto pointSub = sub+innerSub;
    auto Parameter1 = vm["EpsilonPara"].as<double>();

    std::string cacheRoot = vm["cacheRoot"].as<std::string>();

    auto metric = vm["priorityMetric"].as<int>();
    auto iterTimes = vm["iteration"].as<int>();

    double nearScale = 3.0;
    std::string cacheDir;
    if (!cacheRoot.empty()) {
        cacheDir = getCacheDir(cacheRoot, model, sub, Parameter1,nearScale,innerSub);
    }

    // init model
    ModelBase* pModel=new ModelBase( model.c_str(), true );
    int numTris = pModel->getNumberOfTriangles();

    // init DagCenter
    DAGCenter* pDagCenter = new DAGCenter(numTris);

    // generate Dags
    gLogInfo<<"sublevel "<<pointSub;
    DagMaker maker(pModel, pDagCenter);
    maker.init(sub, pointSub, Parameter1);
    maker.genDags();
    maker.preClustering(innerSub);

    gLogInfo<<"prepare merging";
    // merge Dags
    BufferMaker merger(pModel, pDagCenter);
    merger.init(sub, metric);
    merger.merge();
    int oldNClusters = merger.getNClusters();
    for( int i=0;i<iterTimes;i++){
        merger.updateClusters();
    }
    int nClusters = merger.getNClusters();
    gLogInfo<<"relax cluster from "<< oldNClusters<<" -- " <<nClusters;
    merger.saveAssign(cacheDir);
    merger.vCacheOrder(cacheDir);

    return 0;
}
