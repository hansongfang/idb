#ifndef APPCOMMON_H
#define APPCOMMON_H
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "Log.h"
#include "DLL.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace lg = boost::log;

TRIDAG_LIB void initLogging(std::string appname);

TRIDAG_LIB void toggleDebug(bool debug);

TRIDAG_LIB std::string getCacheDir(std::string rootDir, std::string model, int sub, double epsi,double nearScale,int interLevel);

#endif // APPCOMMON_H
