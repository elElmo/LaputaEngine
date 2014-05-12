#ifndef _LPTAMESH_H_
#define _LPTAMESH_H_

#include <string>
#include "LptaResource.h"
#include "LptaMaterialManager.h"
#include "LptaTextureManager.h"
#include "LptaSkinManager.h"
#include "vertices/LptaVertices.h"
#include "vertices/LptaIndices.h"

namespace lpta
{
class LptaRenderDevice;

class LptaMeshLoadFailure
{
public:
    LptaMeshLoadFailure(const std::string &reason) : reason(reason) {}
    ~LptaMeshLoadFailure(void) {}

    const std::string reason;
};

// todo rename to something more sensible
class LptaMesh
{
public:
    LptaMesh(void) {}
    virtual ~LptaMesh(void) {}

    virtual const LptaVertices &GetVertices(void) const = 0;
    virtual const INDICES &GetIndices(void) const = 0;

    virtual unsigned int NumVertices(void) const = 0;
    virtual unsigned int NumFaces(void) const = 0;

    bool IsCached(void) const { return isCached; }
    void SetCached(LptaResource::ID cacheId);
    LptaResource::ID GetCacheId(void) const { return cacheId; }

    LptaSkin::ID GetSkinId(void) const { return skinId; }

protected:
    LptaSkin::ID skinId = 0;

private:
    bool isCached = false;
    LptaResource::ID cacheId;

public:
    static void SetLoadSwapZYAxis(bool swap) { loadSwapZYAxis = swap; }
    static bool GetLoadSwapZYAxis(void) { return loadSwapZYAxis; }

    static LptaMesh *LoadFromFile(const std::string &filename, 
        const LptaRenderDevice &device,
        VERTEX_TYPE vertexType=VERTEX_TYPE::VT_UL);

protected:
    static bool loadSwapZYAxis;
};

}

#endif