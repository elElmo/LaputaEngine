#include <list>
#include <set>
#include "vertices/LptaD3DVertex.h"
#include "LptaD3DVertexCache.h"

namespace lpta_d3d
{
const unsigned int DEFAULT_DYNAMIC_VERTICES_SIZE = 5000;
const unsigned int DEFAULT_DYNAMIC_INDICES_SIZE = 1000;

inline D3DMATERIAL9 ToDXMaterial(const lpta::LptaMaterial &material);

LptaD3DVertexCache::LptaD3DVertexCache(LPDIRECT3DDEVICE9 d3ddev) : 
    d3ddev(d3ddev), staticBuffers(LptaD3DStaticBufferManager(d3ddev)),
    materialManager(unique_ptr<lpta::LptaMaterialManager>(new lpta::LptaMaterialManager())),
    skinManager(unique_ptr<lpta::LptaSkinManager>(new lpta::LptaSkinManager(*materialManager))),
    textureManager(unique_ptr<LptaD3DTextureManager>(new LptaD3DTextureManager(d3ddev)))
{
}

LptaD3DVertexCache::~LptaD3DVertexCache(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Static buffers
/////////////////////////////////////////////////////////////////////
LptaD3DStaticBufferResource::ID LptaD3DVertexCache::CreateStaticBuffer(
    const lpta::LptaVertices &vertices, const lpta::INDICES &indices, 
    lpta::LptaSkin::SKIN_ID skinId)
{
    lpta::LptaResource::ID id = staticBuffers.AddBuffer(
        new LptaD3DStaticBuffer(d3ddev, vertices, indices, skinId)
    );
    return id;
}

HRESULT LptaD3DVertexCache::FlushStaticBuffer(LptaD3DStaticBufferResource::ID id)
{
    using namespace  lpta;
    // todo different shade mode support
    // todo book uses active, would defensive coding's runtime cost be significant 
    // enough to warrant it?
    // assume solid shade for now

    try {
        
        LptaD3DStaticBuffer *buffer = staticBuffers.GetStaticBuffer(id);
        d3ddev->SetIndices(buffer->GetIndexBuffer());
        d3ddev->SetStreamSource(0, buffer->GetVertexBuffer(), 0, ToStride(buffer->GetVertexType()));
        const LptaSkin &skin = skinManager->RetrieveSkin(buffer->GetSkinId());
        const LptaMaterial &material = materialManager->RetrieveResource(skin.GetMaterialId());
        D3DMATERIAL9 dxMaterial = ToDXMaterial(material);
        d3ddev->SetMaterial(&dxMaterial);

        for (unsigned int i = 0; i < skin.MAX_TEXTURES; ++i) {
            if (skin.INVALID_TEXTURE_ID != skin.GetTextureIds().at(i)) {
                const LptaTexture &texture = textureManager->RetrieveTexture(
                    skin.GetTextureIds().at(i)
                );
                // todo this indicing seems odd
                d3ddev->SetTexture(i, static_cast<LPDIRECT3DTEXTURE9>(texture.GetData()));
            }
        }
        d3ddev->SetFVF(ToFVF(buffer->GetVertexType()));
        unsigned int foo = static_cast<unsigned int>(buffer->GetNumIndices() / 3.0f);
        // assume triangles for now
        return d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
            0, buffer->GetNumVertices(), 
            0, static_cast<unsigned int>(buffer->GetNumIndices() / 3.0f)
        );
    }
    catch (NoSuchBuffer) {
        // log error
        return E_FAIL;
    }
}
D3DMATERIAL9 ToDXMaterial(const lpta::LptaMaterial &material)
{
    D3DMATERIAL9 dxMat = {
        material.GetDiffuse().GetRed(), material.GetDiffuse().GetGreen(),
        material.GetDiffuse().GetBlue(), material.GetDiffuse().GetAlpha(),

        material.GetAmbient().GetRed(), material.GetAmbient().GetGreen(),
        material.GetAmbient().GetBlue(), material.GetAmbient().GetAlpha(),

        material.GetSpecular().GetRed(), material.GetSpecular().GetGreen(),
        material.GetSpecular().GetBlue(), material.GetSpecular().GetAlpha(),

        material.GetEmissive().GetRed(), material.GetEmissive().GetGreen(),
        material.GetEmissive().GetBlue(), material.GetEmissive().GetAlpha(),

        material.GetSpecularPower()
    };
    return dxMat;
}

///////////////////////////////////////////////////////////////////////////////
// Dynamic buffers
/////////////////////////////////////////////////////////////////////
HRESULT LptaD3DVertexCache::Render(const lpta::LptaVertices &vertices, const lpta::INDICES &indices, 
    lpta::LptaSkin::SKIN_ID skinId)
{
    LptaD3DDynamicBuffer *buffer = nullptr;
    for (DYNAMIC_BUFFER &candidate : dynamicBuffers) {
        if (candidate->CanFit(vertices, indices) && skinId == candidate->GetSkinId()) {
            buffer = candidate.get();
            break;
        }
    }
    // todo prevent buffer allocate overruns
    if (nullptr == buffer) {
        DYNAMIC_BUFFER newBuffer(
            new LptaD3DDynamicBuffer(
                d3ddev, 
                vertices.GetType(), 
                std::max(DEFAULT_DYNAMIC_VERTICES_SIZE, vertices.GetNumVertices()), 
                std::max(DEFAULT_DYNAMIC_INDICES_SIZE, indices.size()), 
                skinId));
        dynamicBuffers.push_back(newBuffer);
        buffer = newBuffer.get();
    }
    buffer->Add(vertices, indices);
    return S_OK;
}

HRESULT LptaD3DVertexCache::ForcedFlushAll(void)
{
    std::set<lpta::VERTEX_TYPE> processedTypes;
    for (DYNAMIC_BUFFER &buffer : dynamicBuffers) {
        if (processedTypes.count(buffer->GetVertexType()) == 0) {
            ForcedFlush(buffer->GetVertexType());
            processedTypes.insert(buffer->GetVertexType());
        }
    }
    return S_OK;
}

// todo eliminate code dup with static
HRESULT LptaD3DVertexCache::ForcedFlush(lpta::VERTEX_TYPE vertexType)
{
    //std::list<DYNAMIC_BUFFER> deadBuffers;
    bool failed = false;
    for (DYNAMIC_BUFFER &buffer : dynamicBuffers) {
        if (vertexType == buffer->GetVertexType()) {
            using namespace lpta;
            d3ddev->SetIndices(buffer->GetIndexBuffer());
            d3ddev->SetStreamSource(0, buffer->GetVertexBuffer(), 0, ToStride(buffer->GetVertexType()));
            const LptaSkin &skin = skinManager->RetrieveSkin(buffer->GetSkinId());
            const LptaMaterial &material = materialManager->RetrieveResource(skin.GetMaterialId());
            D3DMATERIAL9 dxMaterial = ToDXMaterial(material);
            d3ddev->SetMaterial(&dxMaterial);

            for (unsigned int i = 0; i < skin.MAX_TEXTURES; ++i) {
                if (skin.INVALID_TEXTURE_ID != skin.GetTextureIds().at(i)) {
                    const LptaTexture &texture = textureManager->RetrieveTexture(
                        skin.GetTextureIds().at(i)
                    );
                    // todo this indicing seems odd
                    d3ddev->SetTexture(i, static_cast<LPDIRECT3DTEXTURE9>(texture.GetData()));
                }
            }
            d3ddev->SetFVF(ToFVF(buffer->GetVertexType()));
            unsigned int foo = static_cast<unsigned int>(buffer->GetNumIndices() / 3.0f);
            // assume triangles for now
            HRESULT result = d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
                0, buffer->GetNumVertices(), 
                0, static_cast<unsigned int>(buffer->GetNumIndices() / 3.0f));
            if (S_OK != result) {
                failed = true;
            }
            buffer->Clear();
            //deadBuffers.push_back(buffer);
        }
    }
    // todo reclaim buffers as necessary, for now, simply empty
    return !failed? S_OK : E_FAIL;
}

}