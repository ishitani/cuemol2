// -*-Mode: C++;-*-
//
//  PyMOL Session File Reader
//
// $Id: SceneXMLReader.cpp,v 1.14 2011/04/10 10:48:09 rishitani Exp $

#include <common.h>

#include "PSEFileReader.hpp"
#include <qsys/StreamManager.hpp>
#include <qsys/SceneEvent.hpp>
// #include <qsys/SysConfig.hpp>
// #include <pybr/PythonBridge.hpp>

// #include <qsys/ObjReader.hpp>
// #include <qsys/style/AutoStyleCtxt.hpp>
// #include <qsys/RendererFactory.hpp>

// #include <qlib/LDOM2Stream.hpp>
#include <qlib/FileStream.hpp>
// #include <qlib/StringStream.hpp>
// #include <qlib/LByteArray.hpp>

// #include <boost/filesystem/operations.hpp>
// #include <boost/filesystem/path.hpp>
// namespace fs = boost::filesystem;

#include <qlib/RangeSet.hpp>
#include <qsys/Renderer.hpp>
#include <molstr/MolCoord.hpp>
#include <molstr/MolRenderer.hpp>
#include <molstr/SelCommand.hpp>

#include "PickleInStream.hpp"
#include "PSEConsts.hpp"

using namespace pseread;
using qlib::LDom2Node;
using qlib::LDataSrcContainer;

using qsys::RendererPtr;

using molstr::MolCoord;
using molstr::MolCoordPtr;
using molstr::MolAtom;
using molstr::MolAtomPtr;
using molstr::ResidIndex;

PSEFileReader::PSEFileReader()
{
}

PSEFileReader::~PSEFileReader()
{
}

int PSEFileReader::getCatID() const
{
  return IOH_CAT_SCEREADER;
}

/// attach to and lock the target object
void PSEFileReader::attach(ScenePtr pScene)
{
  // TO DO: lock scene
  m_pClient = pScene;
}
    
/// detach from the target object
ScenePtr PSEFileReader::detach()
{
  // TO DO: unlock scene
  ScenePtr p = m_pClient;
  m_pClient = ScenePtr();
  return p;
}

/// Get name of the reader
const char *PSEFileReader::getName() const
{
  return "psefile";
}

/// Get file-type description
const char *PSEFileReader::getTypeDescr() const
{
  return "PyMOL Session (*.pse)";
}

/// Get file extension
const char *PSEFileReader::getFileExt() const
{
  return "*.pse";
}

//////////////////////////////////////////////////

void PSEFileReader::read()
{
  //  LOG_DPRINTLN("PSEFileReader> File loaded: %s.", getPath().c_str());

/*
  qsys::SysConfig *pconf = qsys::SysConfig::getInstance();
  LString filename = pconf->convPathName("%%CONFDIR%%/data/python/pse_reader.py");

  pybr::PythonBridge *pb = pybr::PythonBridge::getInstance();
  LString arguments = LString::format("{\"filename\": \"%s\"}", getPath().c_str()); 
  pb->runFile3(filename, m_pClient->getUID(), 0, arguments);
*/
  
  LString localfile = getPath();
  
  qlib::FileInStream fis;
  fis.open(localfile);
  PickleInStream ois(fis);

  LVariant *pTop = ois.getMap();

  if (!pTop->isDict()) {
    delete pTop;
    return;
  }

  LVarDict *pDict = pTop->getDictPtr();

  int ver = pDict->getInt("version");
  LOG_DPRINTLN("PyMOL version %d", ver);

  LVarList *pNames = pDict->getList("names");
  procNames(pNames);
  
  delete pTop;
  
  //////////
  // fire the scene-loaded event
  {
    qsys::SceneEvent ev;
    ev.setTarget(m_pClient->getUID());
    ev.setType(qsys::SceneEvent::SCE_SCENE_ONLOADED);
    m_pClient->fireSceneEvent(ev);
  }
}

void PSEFileReader::procNames(LVarList *pNames)
{
  LVarList::const_iterator iter = pNames->begin();
  LVarList::const_iterator eiter = pNames->end();

  for (; iter!=eiter; ++iter) {
    LVariant *pElem = *iter;
    if (!pElem->isList()) {
      // ERROR!!
      LOG_DPRINTLN("Invalid elem in names");
      continue;
    }
    LVarList *pList = pElem->getListPtr();

    LString name = pList->getString(0);
    int type = pList->getInt(1);
    int visible = pList->getInt(2);
    LVarList *pRepOn = pList->getList(3);
    int extra_int = pList->getInt(4);
    LVarList *pData = pList->getList(5);
    
    MB_DPRINTLN("Name %s type=%d", name.c_str(), type);
    if (type == ExecObject) {
      if (extra_int == ObjectMolecule) {
        MolCoordPtr pMol(new MolCoord);
        pMol->setName(name);
        // mol = cuemol.createObj("MolCoord")
        // mol.name = name
        // scene.addObject(mol)

        parseObjectMolecule(pData, pMol);
        m_pClient->addObject(pMol);
      }
      else if (extra_int == ObjectMap) {
        //pass
      }
      else if (extra_int == ObjectMesh) {
        // pass
      }
      else if (extra_int == ObjectMeasurement) {
        // pass    
      }
    }
    else if (type == ExecSelection) {
      // pass
    }
  }
}

void PSEFileReader::parseObject(LVarList *pData, qsys::ObjectPtr pObj)
{
}

static const int AT_CHAIN = 1;
static const int AT_RESI = 3;
static const int AT_RESN = 5;
static const int AT_NAME = 6;
static const int AT_ELEM = 7;
static const int AT_BFAC = 14;
static const int AT_OCC = 15;
static const int AT_VISREP = 20;

static const int CT_NINDEX = 0;
static const int CT_NATINDEX = 1;
static const int CT_COORD = 2;
static const int CT_IDXTOATM = 3;
static const int CT_ATOMTOIDX = 4;
static const int CT_NAME = 5;

static const int REP_STICKS = 0;
static const int REP_SPHERES = 1;
static const int REP_SURFACE = 2; // objSurface
static const int REP_LABELS = 3;
static const int REP_NBSPHERES = 4;
static const int REP_CARTOON = 5;
static const int REP_RIBBON = 6;
static const int REP_LINES = 7;
static const int REP_MESH = 8; // objMesh
static const int REP_DOTS = 9; // dots; also used for objMap
static const int REP_DASHES = 10;  // for measurements
static const int REP_NONBONDED = 11;

static const int REP_CELL = 12; // for objMesh, objSurface
static const int REP_CGO = 13; // for sculpt mode, objAlignment, objCGO
static const int REP_CALLBACK = 14; // for objCallback
static const int REP_EXTENT = 15; // for objMap
static const int REP_SLICE = 16; // for objSlice
static const int REP_ANGLES = 17;
static const int REP_DIHEDRALS = 18;
static const int REP_ELLIPSOID = 19;
static const int REP_VOLUME = 20;

static const int REP_MAX = 21;

namespace {
  void createRends(MolCoordPtr pMol,
                   const qlib::RangeSet<int>&rs,
                   const LString &rendname,
                   const LString &styname)
  {
    if (!rs.isEmpty()) {
      LString str = qlib::rangeToString(rs);
      molstr::MolRendererPtr pRend = pMol->createRenderer(rendname);
      pRend->applyStyles(styname);
      
      molstr::SelectionPtr sel(new molstr::SelCommand("aid "+str));
      pRend->setSelection(sel);
      pRend->setDefaultPropFlag("sel", false);
    }
  }
}


void PSEFileReader::parseObjectMolecule(LVarList *pData, MolCoordPtr pMol)
{
  LVarList *pData0 = pData->getList(0);
  parseObject(pData0, pMol);

  int i; 
  int ncset = pData->getInt(1);
  int nbonds = pData->getInt(2);
  int natoms = pData->getInt(3);

  MB_DPRINTLN("ncset=%d, nbonds=%d, natoms=%d", ncset, nbonds, natoms);

  for (i=0; i<ncset; ++i) {
  }

  for (i=0; i<nbonds; ++i) {
  }

  LVarList *pCSet = pData->getList(4)->getList(0);
  LVarList *pCoord = pCSet->getList(CT_COORD);
  //int nIndex = pCSet->getInt(CT_NINDEX);
  //int nAtIndex = pCSet->getInt(CT_NATINDEX);
  LVarList *pIdxToAtm = pCSet->getList(CT_IDXTOATM);
  int nind = pIdxToAtm->size();
  // LVarList *pAtomToIdx = pCSet->getList(CT_ATOMTOIDX);
  // pIdxToAtm->dump();
  // pAtomToIdx->dump();
  // LVarList *pCtName = pCSet->getList(CT_NAME);
  LVarList *pAtmsDat = pData->getList(7);

  qlib::RangeSet<int> rsSticks;
  qlib::RangeSet<int> rsSpheres;
  qlib::RangeSet<int> rsCartoon;
  qlib::RangeSet<int> rsLines;

  for (i=0; i<nind; ++i) {
    int aid = pIdxToAtm->getInt(i);

    MB_DPRINTLN("Loading atom %d...", aid);
    LVarList *pAtmDat = pAtmsDat->getList(aid);
    
    MolAtomPtr pAtom(new MolAtom);

    LString sresi = pAtmDat->getString(AT_RESI);
    ResidIndex resi = ResidIndex::fromString(sresi);
    pAtom->setResIndex(resi);
    pAtom->setChainName(pAtmDat->getString(AT_CHAIN));
    pAtom->setResName(pAtmDat->getString(AT_RESN));

    pAtom->setName(pAtmDat->getString(AT_NAME));
    pAtom->setElementName(pAtmDat->getString(AT_ELEM));
    pAtom->setBfac(pAtmDat->getReal(AT_BFAC));
    pAtom->setOcc(pAtmDat->getReal(AT_OCC));

    double x = pCoord->getReal(i*3);
    double y = pCoord->getReal(i*3+1);
    double z = pCoord->getReal(i*3+2);
    pAtom->setPos(Vector4D(x,y,z));

    int res = pMol->appendAtom(pAtom);
    if (res<0) {
      LString msg = "Append atom failed: " + pAtom->formatMsg();
      LOG_DPRINTLN(msg);
    }

    LVarList *pVisReps = pAtmDat->getList(AT_VISREP);
    if (pVisReps->getInt(REP_STICKS)==1) {
      rsSticks.append(res, res+1);
    }
    if (pVisReps->getInt(REP_SPHERES)==1) {
      rsSpheres.append(res, res+1);
    }
    if (pVisReps->getInt(REP_CARTOON)==1) {
      rsCartoon.append(res, res+1);
    }
    if (pVisReps->getInt(REP_LINES)==1) {
      rsLines.append(res, res+1);
    }
  }

  pMol->applyTopology();
  pMol->calcProt2ndry(-500.0);
  pMol->calcBasePair(3.7, 30);

  createRends(pMol, rsSticks, "ballstick", "DefaultBallStick");
  createRends(pMol, rsSpheres, "cpk", "DefaultCPK");
  createRends(pMol, rsCartoon, "ribbon", "DefaultRibbon");
  createRends(pMol, rsLines, "simple", "DefaultSimple");

}

