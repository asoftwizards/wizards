#ifndef _GC_H_
#define _GC_H_

#include "gpn_auto.h"
#include "aparser/aparser.h"
#include "acommon/utf8.h"
#include "acommon/acommon.h"
#include "/usr/local/effi/include/Services/Authorizer/authorizer_pnames_auto.h"

#include "datalist/dblist.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/conversion.hpp"
#include "acommon/atranslator.h"


using namespace boost::posix_time;
using namespace boost::gregorian;

int AfterRegisterCallback(Effi::MTContainer* container);
#endif
