// -*-Mode: C++;-*-
//
// MD trajectory object
//

#include <common.h>

#include "Trajectory.hpp"

using namespace mdtools;

Trajectory::Trajectory()
{
  m_bInit = false;
  m_nBlkInd = -1;
  m_nFrmInd = -1;
  m_nTotalFrms = 0;
  m_nCurFrm = 0;
}

Trajectory::~Trajectory()
{
}

qfloat32 *Trajectory::getCrdArrayImpl()
{
  TrajBlockPtr pBlk = m_blocks[m_nBlkInd];
  return pBlk->getCrdArray(m_nFrmInd);
}

void Trajectory::invalidateCrdArray()
{
}

void Trajectory::append(TrajBlockPtr pBlk)
{
  int nAtoms = getAtomSize();
  if (nAtoms*3 != pBlk->getCrdSize()) {
    // non-compatible trajectory block
    MB_THROW(qlib::RuntimeException, "non compatible atom coord size");
    return;
  }
  
  int nnext = 0;
  if (!m_blocks.empty()) {
    TrajBlockPtr pLast = m_blocks.back();
    nnext = pLast->getStartIndex() + pLast->getSize();
  }
  pBlk->setStartIndex(nnext);
  m_blocks.push_back(pBlk);

  if (!m_bInit) {
    update(0);
    createLinearMap();
    applyTopology();
    m_bInit = true;
  }

  m_nTotalFrms += pBlk->getSize();
  
  LOG_DPRINTLN("Traj> append blk start=%d, size=%d", nnext, pBlk->getSize());
}

void Trajectory::update(int iframe)
{
  int ind1 = 0;
  int ind2 = -1;

  TrajBlockPtr pBlk;
  BOOST_FOREACH (TrajBlockPtr pelem, m_blocks) {
    int istart = pelem->getStartIndex();
    int iend = istart + pelem->getSize() -1;
    if (istart<=iframe && iframe<=iend) {
      ind2 = iframe - istart;
      pBlk = pelem;
      break;
    }
    ++ind1;
  }

  if (ind2<0) {
    // ERROR: iframe out of range
    MB_THROW(qlib::RuntimeException, "update(): iframe out of range");
    return;
  }

  //qfloat32 *pcrd = pBlk->getCrdArray(ind2);
  m_nCurFrm = iframe;
  m_nBlkInd = ind1;
  m_nFrmInd = ind2;

  crdArrayChanged();
  
  // broadcast modification event
  fireAtomsMoved();
}

int Trajectory::getFrame() const
{
  return m_nCurFrm;
}

void Trajectory::setFrame(int ifrm)
{
  update(ifrm);
}

int Trajectory::getFrameSize() const
{
  return m_nTotalFrms;
}

void Trajectory::writeTo2(qlib::LDom2Node *pNode) const
{
}

void Trajectory::readFrom2(qlib::LDom2Node *pNode)
{
}

void Trajectory::readFromStream(qlib::InStream &ins)
{
}

