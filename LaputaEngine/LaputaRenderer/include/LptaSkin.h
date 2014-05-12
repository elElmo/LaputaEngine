#ifndef _LPTASKIN_H_
#define _LPTASKIN_H_

#ifdef max
#undef max
#endif

#include <limits>
#include <list>
#include "LptaResource.h"
#include "LptaMaterial.h"
#include "LptaTexture.h"

namespace lpta
{
using std::list;

class LptaSkin : public LptaResource
{
public:
    static const LptaResource::ID INVALID_TEXTURE_ID;
    typedef ID SKIN_ID;
    static const unsigned int MAX_TEXTURES = 8;
    typedef list<LptaResource::ID> TEXTURE_IDS;

    static const bool SKIN_TRANSPARENT = true;
    static const bool SKIN_NON_TRANSPARENT = false;

public:
    LptaSkin(SKIN_ID id, LptaMaterial::MATERIAL_ID materialId, bool transparent);
    LptaSkin(SKIN_ID id, LptaMaterial::MATERIAL_ID materialId, const TEXTURE_IDS &textureIds, bool transparent);
    ~LptaSkin(void);

    LptaMaterial::MATERIAL_ID GetMaterialId(void) const { return materialId; }
    const TEXTURE_IDS &GetTextureIds(void) const { return textureIds; }

    unsigned int NumTextures(void) const { return textureIds.size(); }

    unsigned int AddTexture(LptaTexture::ID textureId);

private:
    bool transparent;
    LptaMaterial::MATERIAL_ID materialId;
    TEXTURE_IDS textureIds;

};

}

#endif