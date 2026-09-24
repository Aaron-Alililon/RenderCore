#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "D3D11/D3D11Device.h"

namespace rcore {

  class RenderTarget {
  public:
    RenderTarget() = default;
    RenderTarget(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor);
    explicit RenderTarget(std::filesystem::path const& path);

  public:
    void clearRTV(float r = 0, float g = 0, float b = 0, float a = 1) const;

    ID3D11Texture2D* getTexture() const;
    ID3D11ShaderResourceView* getSRV() const;
    ID3D11RenderTargetView* getRTV(int face = 0, int mip = 0) const;
    std::vector<ID3D11RenderTargetView*> getRTVs() const;
    bool storeAsDDS(std::filesystem::path const& path) const;
    bool isValid() const;

  private:
    bool validateDescriptors(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor) const;
    bool createTexture(D3D11_TEXTURE2D_DESC const& textureDescriptor, D3D11_SHADER_RESOURCE_VIEW_DESC const& srvDescriptor);
    bool loadFromFile(std::filesystem::path const& path);
    bool createRTVs();

  private:
    bool m_valid = false;
    UINT m_faces = 0;
    UINT m_mipLevels = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_rtvs;
  };

}

#endif