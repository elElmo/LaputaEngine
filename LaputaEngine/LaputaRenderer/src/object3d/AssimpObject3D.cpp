#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "LptaVector.h"
#include "LptaRenderDevice.h"
#include "object3d/AssimpObject3D.h"
#include "vertices/LptaULVertices.h"
#include "vertices/LptaUUVertices.h"

#define SUCCESSFUL
#define IMPORT_FLAGS aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs

namespace lpta
{

Assimp::Importer LptaAssimpMesh::assetImporter;

typedef struct ManagersType
{
    LptaMaterialManager &material;
    LptaTextureManager &texture;
    LptaSkinManager &skin;
} MANAGERS;

// LptaAssimpMesh(filename)
inline bool IsValidScene(const aiScene &scene);
inline LptaVertices *CopyMesh(VERTEX_TYPE vertexType, INDICES &indices, LptaSkin::ID &skinId,
    const aiScene *mesh, MANAGERS &managers);
template <class VType,  class CopyAlgorithm>
inline VType *CopyMesh(INDICES &indices, LptaSkin::ID &skinId, const aiScene *scene, MANAGERS &managers);
struct ULAlgorithm
{
    inline static void Copy(LptaULVertices &vertices, INDICES &indices, const aiMesh &mesh, unsigned int vertexOffset);
};
struct UUAlgorithm
{
    inline static void Copy(LptaUUVertices &vertices, INDICES &indices, const aiMesh &mesh, unsigned int vertexOffset);
};
/*inline LptaULVertices *CopyULMesh(INDICES &indices, const aiScene *scene, MANAGERS &managers);
inline LptaUUVertices *CopyUUMesh(INDICES &indices, const aiScene *scene, MANAGERS &managers);*/


LptaAssimpMesh::LptaAssimpMesh(
    VERTEX_TYPE vertexType, 
    const std::string &filename,
    const LptaRenderDevice &device)
{
    MANAGERS managers = { 
        *device.GetMaterialManager(), 
        *device.GetTextureManager(), 
        *device.GetSkinManager() 
    };
    if (SUCCESSFUL assetImporter.ReadFile(filename, IMPORT_FLAGS)) {
        const aiScene *scene = assetImporter.GetOrphanedScene();
        try {
            if (!scene || !IsValidScene(*scene)) {
                throw LptaMeshLoadFailure("Not a valid mesh file");
            }
            vertices = CopyMesh(vertexType, indices, skinId, scene, managers);
        }
        catch (const LptaMeshLoadFailure &loadFailure) {
            if (scene) {
                aiReleaseImport(scene);
            }
            throw loadFailure;
        }
        aiReleaseImport(scene);     
    }
    else {
        // log error
        throw LptaMeshLoadFailure(assetImporter.GetErrorString());
    }
}
bool IsValidScene(const aiScene &scene)
{
    return true;
}
LptaVertices *CopyMesh(VERTEX_TYPE vertexType, INDICES &indices, LptaSkin::ID &skinId, const aiScene *scene, MANAGERS &managers)
{
    switch (vertexType) {
    case VERTEX_TYPE::VT_UL:
        return CopyMesh<LptaULVertices, ULAlgorithm>(indices, skinId, scene, managers);
    case VERTEX_TYPE::VT_UU:
        return CopyMesh<LptaUUVertices, UUAlgorithm>(indices, skinId, scene, managers);
    default:
        throw LptaMeshLoadFailure("Don't know how to copy vertices for mesh");
    };
}

template <class VType, class CopyAlgorithm>
VType *CopyMesh(INDICES &indices, LptaSkin::ID &skinId, const aiScene *scene, MANAGERS &managers)
{
    VType *vertices = new VType();
    vector<LptaSkin::ID> skinIds;
    for (unsigned int matIdx = 0; matIdx < scene->mNumMaterials; ++matIdx) {
        aiMaterial *mat = scene->mMaterials[matIdx];
        aiColor4D diffuse(0.0f, 0.0f, 0.0f, 0.0f);
        aiColor4D ambient(0.0f, 0.0f, 0.0f, 0.0f);
        aiColor4D specular(0.0f, 0.0f, 0.0f, 0.0f);
        aiColor4D emissive(0.0f, 0.0f, 0.0f, 0.0f);
        float specularPower = 0.0f;

        mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
        mat->Get(AI_MATKEY_SHININESS, specularPower);

        aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &emissive);

        LptaMaterial::ID materialId = managers.material.AddOrRetrieveMaterial(
            LptaColor(diffuse.r, diffuse.g, diffuse.b, diffuse.a),
            LptaColor(ambient.r, ambient.g, ambient.b, diffuse.a),
            LptaColor(specular.r, specular.g, specular.b, specular.a),
            LptaColor(emissive.r, emissive.g, emissive.b, emissive.a),
            specularPower
        );
        
        LptaSkin::TEXTURE_IDS textureIds;
        // only diffuse texture is supported for now
        for (unsigned int texIdx = 0; texIdx < mat->GetTextureCount(aiTextureType_DIFFUSE) && texIdx < LptaSkin::MAX_TEXTURES; ++texIdx) {
            aiString filepath;
            mat->GetTexture(aiTextureType_DIFFUSE, texIdx, &filepath);
            LptaTexture::ID textureId = managers.texture.AddOrRetrieveTexture(filepath.C_Str());
            textureIds.push_back(textureId);
        }
        LptaSkin::ID skinId = managers.skin.AddSkin(materialId, textureIds, false);
        skinIds.push_back(skinId);
    }
    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
        const aiMesh *mesh = scene->mMeshes[meshIdx];

        CopyAlgorithm::Copy(*vertices, indices, *mesh, vertices->GetNumVertices());
        skinId = skinIds.at(mesh->mMaterialIndex);
    }
    return vertices;
}
void ULAlgorithm::Copy(LptaULVertices &vertices, INDICES &indices, const aiMesh &mesh, unsigned int vertexOffset)
{
    for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
        UL_VERTEX vertex;
        auto meshVertex = mesh.mVertices[i];
        // todo use actual values
        vertex.coordinate = LptaMesh::GetLoadSwapZYAxis()?
            lpta_3d::LptaVector(meshVertex.x, meshVertex.z, meshVertex.y) :
            lpta_3d::LptaVector(meshVertex.x, meshVertex.y, meshVertex.z);
        vertex.tu = 0.0f;
        vertex.tv = 0.0f;
        vertices.AddVertex(vertex);
    }
    for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
        auto face = mesh.mFaces[i];
        for (unsigned int vIndex = 0; vIndex < face.mNumIndices; ++vIndex) {
            indices.push_back(vertexOffset + face.mIndices[vIndex]);
        }
    }
}
void UUAlgorithm::Copy(LptaUUVertices &vertices, INDICES &indices, const aiMesh &mesh, unsigned int vertexOffset)
{
        for (unsigned int i = 0; i < mesh.mNumVertices; ++i) {
            UU_VERTEX vertex;
            auto meshVertex = mesh.mVertices[i];
            // todo use actual values
            vertex.coordinate = LptaMesh::GetLoadSwapZYAxis()?
                lpta_3d::LptaVector(meshVertex.x, meshVertex.z, meshVertex.y) :
                lpta_3d::LptaVector(meshVertex.x, meshVertex.y, meshVertex.z);
            if (nullptr != mesh.mTextureCoords) {
                vertex.tu = mesh.mTextureCoords[0][i].x;
                vertex.tv = mesh.mTextureCoords[0][i].y;
            }
            vertex.normal = lpta_3d::LptaVector(
                mesh.mNormals[i].x,
                (LptaMesh::GetLoadSwapZYAxis()? mesh.mNormals[i].z : mesh.mNormals[i].y),
                (LptaMesh::GetLoadSwapZYAxis()? mesh.mNormals[i].y : mesh.mNormals[i].z)
            );
            vertices.AddVertex(vertex);
        }
        for (unsigned int i = 0; i < mesh.mNumFaces; ++i) {
            auto face = mesh.mFaces[i];
            for (unsigned int vIndex = 0; vIndex < face.mNumIndices; ++vIndex) {
                indices.push_back(vertexOffset + face.mIndices[vIndex]);
            }
        }
}

LptaAssimpMesh::~LptaAssimpMesh(void)
{
    delete vertices;
}


const LptaVertices &LptaAssimpMesh::GetVertices(void) const
{
    return *vertices;
}

const INDICES &LptaAssimpMesh::GetIndices(void) const
{
    return indices;
}


unsigned int LptaAssimpMesh::NumVertices(void) const
{
    return vertices->GetNumVertices();
}

// todo fix case for where faces aren't triangles (if needed)
unsigned int LptaAssimpMesh::NumFaces(void) const
{
    return indices.size();
}

}