#include <iostream>
#include <dag-lib/appcommon.h>
#include <boost/program_options.hpp>
#include <viewSample.h>


int main(int argc, char *argv[])
{
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
            ("cacheDir,c", po::value< std::string >()->default_value(""),"Cache path. default ./cache")
            ("debug,d", po::bool_switch()->default_value(false), "show debug info")
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

    auto model = vm["model"].as<string>();
    string cacheDir = vm["cacheDir"].as<string>();

    viewSample demo(model, cacheDir);
    demo.render();
}
