#ifndef TARGA_32_LOADER_H
#define TARGA_32_LOADER_H

#include "Resource/Interface/ISDRLoader.h"

namespace rcore {
  
  class Targa32Loader : public ISDRLoader {
  private:
    struct TargaHeader {
      unsigned char data1[12];
      unsigned short width;
      unsigned short height;
      unsigned char bpp;
      unsigned char data2;
    };

  public:
    PixelComponent* readTexture(std::string const& path, std::pair<UINT, UINT>* outImageSize) override;
  };

}

#endif