#ifndef LOG_H
#define LOG_H

#ifndef Q_MOC_RUN
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#endif // Q_MOC_RUN

#define gLogDebug BOOST_LOG_TRIVIAL(debug)
#define gLogInfo BOOST_LOG_TRIVIAL(info)
#define gLogWarn BOOST_LOG_TRIVIAL(warning)
#define gLogError BOOST_LOG_TRIVIAL(error)
#define gLogFatal BOOST_LOG_TRIVIAL(fatal)
#define gLogTrace BOOST_LOG_TRIVIAL(trace)


#endif // LOG_H
