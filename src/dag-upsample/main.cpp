#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "dag-lib/ModelBase.h"
#include "dag-lib/Log.h"
#include "dag-lib/appcommon.h"
#include "dag-lib/DAGCenter.h"
#include "DAGMergeup.h"

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
            ("fineModel,m", po::value< std::string >(),"fine model path")
            ("coarseModel,n", po::value< std::string >(),"coarse model path")
            ("sub,s", po::value< int >()->default_value(0),"Subdivide level")
            ("innerSub,l",po::value< int >()->default_value(-1),"Defualt inner sub division level -1")
            ("cacheRoot,c", po::value< std::string >()->default_value("./cache"),"Cache root path. ")
            ("coarseCacheRoot,b", po::value< std::string >()->default_value("./cache"),"Cache root path for coarse model. ")
            ("EpsilonPara,e", po::value< double >()->default_value(1000.0), "Epsilon over paramter. default 3000.0")
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


    auto coarseModel = vm["coarseModel"].as<std::string>();
    auto fineModel = vm["fineModel"].as<std::string>();
    auto sub = vm["sub"].as<int>();
    auto innerSub = vm["innerSub"].as<int>();
    auto Parameter1 = vm["EpsilonPara"].as<double>();
    std::string cCacheRoot = vm["coarseCacheRoot"].as<std::string>();
    std::string fCacheRoot = vm["cacheRoot"].as<std::string>();
    double nearScale = 3.0;


    std::string cCacheDir;
    std::string fCacheDir;
    cCacheDir = getCacheDir(cCacheRoot,coarseModel,sub,Parameter1,nearScale, innerSub);
    fCacheDir = getCacheDir(fCacheRoot,fineModel,sub,Parameter1,nearScale, innerSub);

    gLogInfo<<"upsample ";
    gLogInfo<<"coarse model cache dir "<<cCacheDir;
    gLogInfo<<"fine model cache dir "<<fCacheDir;

    DagMergeUp   dagMergeUp;
    dagMergeUp.init(fineModel.c_str(),coarseModel.c_str());
    dagMergeUp.buildTri();//default consider double faces
    dagMergeUp.transIndices(fCacheDir,cCacheDir);//get the dir of the acmrTriOrders

    return 0;
}

