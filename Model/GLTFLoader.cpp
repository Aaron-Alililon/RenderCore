#include "Core/pch.h"
#define CGLTF_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable: 4996)
#include "Lib/cgltf.h"
#pragma warning(pop)
#include "Model/GLTFLoader.h"

namespace rcore {

  static DirectX::XMMATRIX ToXM(const float m[16]) {
    DirectX::XMFLOAT4X4 f;
    memcpy(&f, m, sizeof(float) * 16);
    return DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&f));
  }

  Mesh GLTFLoader::readMesh(std::string const& path) const {
    Mesh mesh;

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success) {
      RCORE_LOG(ERR, "Failed to parse GLTF file: " + path);
      return {};
    }

    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
      RCORE_LOG(ERR, "Failed to load GLTF buffers: " + path);
      cgltf_free(data);
      return {};
    }

    UINT indexOffset = 0;

    for (size_t n = 0; n < data->nodes_count; n++) {
      cgltf_node& node = data->nodes[n];
      if (!node.mesh) continue;

      float worldRaw[16];
      cgltf_node_transform_world(&node, worldRaw);
      DirectX::XMMATRIX world = ToXM(worldRaw);

      DirectX::XMMATRIX normalMat = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));

      for (size_t p = 0; p < node.mesh->primitives_count; p++) {
        cgltf_primitive& prim = node.mesh->primitives[p];

        std::vector<float> positions, normals, tangents, texcoords;

        for (size_t i = 0; i < prim.attributes_count; i++) {
          cgltf_attribute& attr = prim.attributes[i];
          if (attr.type == cgltf_attribute_type_position) positions = readAccessor(attr.data);
          if (attr.type == cgltf_attribute_type_normal)   normals = readAccessor(attr.data);
          if (attr.type == cgltf_attribute_type_tangent)  tangents = readAccessor(attr.data);
          if (attr.type == cgltf_attribute_type_texcoord) texcoords = readAccessor(attr.data);
        }

        size_t vertexCount = positions.size() / 3;
        size_t baseVertex = mesh.vertices.size();

        mesh.vertices.resize(baseVertex + vertexCount);
        mesh.normals.resize(baseVertex + vertexCount);
        mesh.uvs.resize(baseVertex + vertexCount);
        mesh.tangents.resize(baseVertex + vertexCount);
        mesh.binormals.resize(baseVertex + vertexCount);

        for (size_t i = 0; i < vertexCount; i++) {
          DirectX::XMVECTOR pos = DirectX::XMVectorSet(
              positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2], 1.0f);
          pos = DirectX::XMVector3Transform(pos, world);

          DirectX::XMFLOAT4 posOut;
          DirectX::XMStoreFloat4(&posOut, pos);
          mesh.vertices[baseVertex + i] = posOut;

          if (!normals.empty()) {
            DirectX::XMVECTOR nrm = DirectX::XMVectorSet(
                normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2], 0.0f);
            nrm = DirectX::XMVector3TransformNormal(nrm, normalMat);
            nrm = DirectX::XMVector3Normalize(nrm);

            DirectX::XMFLOAT3 nrmOut;
            DirectX::XMStoreFloat3(&nrmOut, nrm);
            mesh.normals[baseVertex + i] = nrmOut;
          } else {
            mesh.normals[baseVertex + i] = { 0.0f, 1.0f, 0.0f };
          }

          if (!texcoords.empty())
            mesh.uvs[baseVertex + i] = { texcoords[i * 2], texcoords[i * 2 + 1] };
          else
            mesh.uvs[baseVertex + i] = { 0.0f, 0.0f };

          if (!tangents.empty()) {
            DirectX::XMVECTOR tan = DirectX::XMVectorSet(
                tangents[i * 4], tangents[i * 4 + 1], tangents[i * 4 + 2], 0.0f);
            tan = DirectX::XMVector3TransformNormal(tan, world);
            tan = DirectX::XMVector3Normalize(tan);

            DirectX::XMFLOAT3 t;
            DirectX::XMStoreFloat3(&t, tan);

            float w = tangents[i * 4 + 3];
            DirectX::XMFLOAT3 nrm = mesh.normals[baseVertex + i];

            mesh.tangents[baseVertex + i] = t;
            mesh.binormals[baseVertex + i] = {
                (nrm.y * t.z - nrm.z * t.y) * w * -1,
                (nrm.z * t.x - nrm.x * t.z) * w * -1,
                (nrm.x * t.y - nrm.y * t.x) * w * -1
            };
          } else {
            mesh.tangents[baseVertex + i] = { 1.0f, 0.0f, 0.0f };
            mesh.binormals[baseVertex + i] = { 0.0f, 0.0f, 1.0f };
          }
        }

        mesh.indices.resize(mesh.indices.size() + prim.indices->count);
        for (size_t i = 0; i < prim.indices->count; i++) {
          UINT idx = static_cast<UINT>(cgltf_accessor_read_index(prim.indices, i));
          mesh.indices[indexOffset + i] = idx + static_cast<UINT>(baseVertex);
        }
        indexOffset += static_cast<UINT>(prim.indices->count);

        mesh.hasTangents = mesh.hasTangents || !tangents.empty();
      }
    }

    cgltf_free(data);
    return mesh;
  }

  std::vector<float> GLTFLoader::readAccessor(cgltf_accessor* accessor) const {
    std::vector<float> out(accessor->count * cgltf_num_components(accessor->type));
    cgltf_accessor_unpack_floats(accessor, out.data(), out.size());
    return out;
  }

}