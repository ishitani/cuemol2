// -*-Mode: C++;-*-
//
// Light-weight viewer module
//

#include <common.h>

#include "mdtools.hpp"

#include <qsys/RendererFactory.hpp>
#include <qsys/StreamManager.hpp>
#include "NAMDCoorReader.hpp"

extern void mdtools_regClasses();
extern void mdtools_unregClasses();

namespace mdtools {

  using qsys::RendererFactory;
  using qsys::StreamManager;

  bool init()
  {
    mdtools_regClasses();

    // register IO handlers
    StreamManager *pSM = StreamManager::getInstance();
    pSM->registWriter<NAMDCoorReader>();

    MB_DPRINTLN("mdtools init: OK");
    return true;
  }

  void fini()
  {
    mdtools_unregClasses();

    MB_DPRINTLN("mdtools fini: OK");
  }

}

