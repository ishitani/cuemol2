// -*-Mode: C++;-*-
//
//  Backbone spline-trace renderer class
//

#include <common.h>
#include "molvis.hpp"

#include "Spline2Renderer.hpp"

#include <qsys/SceneManager.hpp>
//#include <gfx/Texture.hpp>

#include <modules/molstr/MolCoord.hpp>
#include <modules/molstr/MolChain.hpp>
#include <modules/molstr/MolResidue.hpp>
#include <modules/molstr/ResidIterator.hpp>
#include <modules/molstr/AnimMol.hpp>

using namespace molvis;
using namespace molstr;
using namespace molvis::detail;
using qlib::Matrix3D;

Spline2SS::~Spline2SS()
{
}

detail::DrawSegment *Spline2SS::createDrawSeg(int nstart, int nend)
{
  return MB_NEW Spline2DS(nstart, nend);
}

Spline2DS::~Spline2DS()
{
  if (m_pVBO!=NULL)
    delete m_pVBO;
}

//////////////////////////////////////////////////////////////

Spline2Renderer::Spline2Renderer()
     : super_t()
{
  m_dLineWidth = 1.2;
}

Spline2Renderer::~Spline2Renderer()
{
}

const char *Spline2Renderer::getTypeName() const
{
  return "spline2";
}

void Spline2Renderer::preRender(DisplayContext *pdc)
{
  pdc->setLighting(false);
}

SplineSegment *Spline2Renderer::createSegment()
{
  return MB_NEW Spline2SS();
}

void Spline2Renderer::objectChanged(qsys::ObjectEvent &ev)
{
  if (isVisible() &&
      (ev.getType()==qsys::ObjectEvent::OBE_CHANGED_DYNAMIC||
       ev.getType()==qsys::ObjectEvent::OBE_CHANGED) &&
      ev.getDescr().equals("atomsMoved")) {
    // OBE_CHANGED_DYNAMIC && descr=="atomsMoved"
    if (isUseAnim()) {
      // only update positions
      updateCrdDynamic();
      return;
    }
  }

  super_t::objectChanged(ev);
}

//////////////////////////////////////////////////////////////
// VBO implementation

void Spline2Renderer::setupVBO(detail::SplineSegment *pASeg)
{
  Spline2SS *pSeg = static_cast<Spline2SS *>(pASeg);
  const int nDetail = getAxialDetail();

  BOOST_FOREACH (detail::DrawSegment *pelem, pSeg->m_draws) {
    Spline2DS &elem = *static_cast<Spline2DS*>(pelem);
    const int nsplseg = elem.m_nEnd - elem.m_nStart;
    const int nVA = nDetail * nsplseg + 1;

    Spline2DS::VertArray *pVBO = elem.m_pVBO;
    if (pVBO!=NULL)
      delete pVBO;
    
    elem.m_pVBO = pVBO = MB_NEW Spline2DS::VertArray();
    pVBO->alloc(nVA);

    pVBO->setDrawMode(gfx::DrawElem::DRAW_LINE_STRIP);
    LOG_DPRINTLN("Spline2VBO> %d elems VBO created", nVA);
  }
}

void Spline2Renderer::updateCrdVBO(detail::SplineSegment *pASeg)
{
  Spline2SS *pSeg = static_cast<Spline2SS *>(pASeg);
  CubicSpline *pAxInt = pSeg->getAxisIntpol();

  int i, j;
  float par, fStart;
  Vector3F pos;
  const float fDetail = float(getAxialDetail());

  Spline2DS::VertArray *pVBO;

  BOOST_FOREACH (detail::DrawSegment *pelem, pSeg->m_draws) {
    Spline2DS &elem = *static_cast<Spline2DS*>(pelem);
  
    pVBO = elem.m_pVBO;
    fStart = float(elem.m_nStart);
    const int nVA = pVBO->getSize();

    for (i=0; i<nVA; ++i) {
      par = float(i)/fDetail + fStart;
      pAxInt->interpolate(par, &pos);
      pVBO->vertex3f(i, pos);
    }
    
    pVBO->setUpdated(true);
  }
}

void Spline2Renderer::updateColorVBO(detail::SplineSegment *pASeg)
{
  Spline2SS *pSeg = static_cast<Spline2SS *>(pASeg);
  MolCoordPtr pCMol = getClientMol();

  int i;
  float par;
  float fDetail = float(getAxialDetail());

  Spline2DS::VertArray *pVBO;

  BOOST_FOREACH (detail::DrawSegment *pelem, pSeg->m_draws) {
    Spline2DS &elem = *static_cast<Spline2DS*>(pelem);

    pVBO = elem.m_pVBO;
    float fStart = float(elem.m_nStart);
    const int nVA = pVBO->getSize();

    for (i=0; i<nVA; ++i) {
      par = float(i)/fDetail + fStart;
      pVBO->color(i, pSeg->calcColor(this, pCMol, par));
    }
  }
}

void Spline2Renderer::drawVBO(detail::SplineSegment *pASeg, DisplayContext *pdc)
{
  const float lw = float( getLineWidth() );
  Spline2SS *pSeg = static_cast<Spline2SS *>(pASeg);

  BOOST_FOREACH (detail::DrawSegment *pelem, pSeg->m_draws) {
    Spline2DS &elem = *static_cast<Spline2DS*>(pelem);
    elem.m_pVBO->setLineWidth(lw);
    pdc->drawElem(*elem.m_pVBO);
  }
}
