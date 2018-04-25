#include "appcommon.h"

void initLogging(std::string appname) {
    lg::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
    lg::add_console_log(
                std::cout,
                lg::keywords::format = "%TimeStamp% [%Uptime%] (%LineID%) <%Severity%>: %Message%");
    lg::add_file_log(
                lg::keywords::file_name = appname+"_%Y-%m-%d_%H-%M-%S.log",
                lg::keywords::auto_flush = true,
                lg::keywords::format = "%TimeStamp% [%Uptime%] (%LineID%) <%Severity%>: %Message%");
    lg::add_common_attributes();
    lg::core::get()->add_global_attribute("Uptime", lg::attributes::timer());
}

void toggleDebug(bool debug) {
    auto loglevel = lg::trivial::info;
    if ( debug ) {
        loglevel = lg::trivial::trace;
        gLogInfo << "Set log level to DEBUG";
    }
    lg::core::get()->set_filter (
            lg::trivial::severity >= loglevel
        );
}

std::string getCacheDir(
        std::string rootDir, std::string model,
        int sub, double epsi,double nearScale,int interLevel)
{
    fs::path cacheDir( rootDir );
    cacheDir /= fs::path(model).stem();
    if(interLevel < 0){
        cacheDir /= std::to_string(sub);
    }else{
        std::string temp = std::to_string(sub)+"_"+std::to_string(interLevel);
        cacheDir /= temp;
    }

    std::string temp = std::to_string((int)epsi) +"_near"+std::to_string((int)nearScale);
    cacheDir /= temp;



    if (fs::create_directories(cacheDir)) {
        gLogInfo << "Created cache folder for DAG: " << cacheDir;
    }

    return cacheDir.string();
}
