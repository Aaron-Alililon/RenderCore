#ifndef HDR_LOADER_H
#define HDR_LOADER_H

#include "Resource/Interface/IHDRLoader.h"

namespace rcore {

  class HDRLoader : public IHDRLoader {
  public:
    PixelComponent* readTexture(std::string const& path, std::pair<UINT, UINT>* outImageSize) override;
  };

}

#endif