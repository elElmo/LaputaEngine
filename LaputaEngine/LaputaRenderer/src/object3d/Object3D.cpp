#include "object3d/Object3D.h"
#include "object3d/NullObject3D.h"
#include "object3d/AssimpObject3D.h"

namespace lpta
{
bool LptaMesh::loadSwapZYAxis = false;

#pragma warning( push )
#pragma warning( disable : 4101 )
LptaMesh *LptaMesh::LoadFromFile(const std::string &filename, 
    const LptaRenderDevice &device,
    VERTEX_TYPE vertexType)
{
    try {
        return new LptaAssimpMesh(vertexType, filename, device);
    }
    catch (const LptaMeshLoadFailure &failure) {
        // log error
        return new LptaNullMesh();
    }
}
#pragma warning( pop )

void LptaMesh::SetCached(LptaResource::ID cacheId)
{
    this->cacheId = cacheId;
    this->isCached = true;
}

}