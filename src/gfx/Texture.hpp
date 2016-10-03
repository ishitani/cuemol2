// -*-Mode: C++;-*-
//
//  Texture abstract class
//

#ifndef GFX_TEXTURE_HPP_INCLUDED
#define GFX_TEXTURE_HPP_INCLUDED

#include "gfx.hpp"

namespace gfx {

  /// Abstract class of Texture implementation REP
  class TextureRep
  {
  public:
    virtual ~TextureRep() {}

    virtual void setup(int iDim, int iPixFmt, int iPixType) =0;

    virtual void setData(int width, int height, int depth, const void *pdata) =0;

    virtual void use(int nUnit) =0;
    virtual void unuse() =0;

    virtual void setLinIntpol(bool b) {}
  };

  /////////////////////////////////////

  /// Texture class
  class Texture
  {
  public:
    Texture()
      : m_pTexRep(NULL)
    {
    }

    virtual ~Texture()
    {
      if (m_pTexRep!=NULL)
	delete m_pTexRep;
    }

  private:
    mutable TextureRep *m_pTexRep;

  public:
    void setRep(TextureRep *pRep) const {
      m_pTexRep = pRep;
    }
    TextureRep *getRep() const {
      return m_pTexRep;
    }

  public:
    /// pixel format
    static const int FMT_R = 0;
    static const int FMT_RG = 1;
    static const int FMT_RGB = 2;
    static const int FMT_RGBA = 3;

    /// pixel type
    static const int TYPE_UINT8 = 0;
    static const int TYPE_UINT16 = 1;
    static const int TYPE_UINT32 = 2;

    static const int TYPE_FLOAT16 = 10;
    static const int TYPE_FLOAT32 = 11;
    static const int TYPE_FLOAT64 = 12;

    /// host: unsigned int (0-MAX) --> gpu: float (0-1)
    static const int TYPE_UINT8_COLOR = 20;

    void setup(int ndim, int iFmt, int iType)
    {
      getRep()->setup(ndim, iFmt, iType);
    }

    void setData(int w, int h, int d, const void *pdata)
    {
      getRep()->setData(w, h, d, pdata);
    }

    void use(int nUnit)
    {
      getRep()->use(nUnit);
    }

    void unuse()
    {
      getRep()->unuse();
    }

    void setLinIntpol(bool b)
    {
      getRep()->setLinIntpol(b);
    }
  }; 

  /////////////////////////////////////

} // namespace gfx

#endif