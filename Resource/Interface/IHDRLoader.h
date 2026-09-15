#ifndef I_HDR_LOADER_H
#define I_HDR_LOADER_H

#include "Core/pch.h"
#include "Resource/Interface/ITextureLoader.h"

namespace rcore {

  class IHDRLoader : public ITextureLoader {
  public:
    using PixelComponent = float;
    virtual PixelComponent* readTexture(std::string const& path, std::pair<UINT, UINT>* outImageSize) = 0;
  };

}

#endif