#include <limits>
#include "LptaSkin.h"

namespace lpta
{

const LptaResource::ID LptaSkin::INVALID_TEXTURE_ID = std::numeric_limits<LptaResource::ID>::max();

LptaSkin::LptaSkin(SKIN_ID id, LptaMaterial::MATERIAL_ID materialId, bool transparent) :
    LptaSkin(id, materialId, TEXTURE_IDS(), transparent)
{}

LptaSkin::LptaSkin(SKIN_ID id, LptaMaterial::MATERIAL_ID materialId, const TEXTURE_IDS &textureIds, bool transparent) : 
    LptaResource(id), materialId(materialId), transparent(transparent)
{
    TEXTURE_IDS idsCopy = textureIds;
    // todo catch texture overflow case
    for (unsigned int i = 0; i < textureIds.size() && i < MAX_TEXTURES; ++i) {
        this->textureIds.push_back(idsCopy.front());
        idsCopy.pop_front();
    }
}

LptaSkin::~LptaSkin(void)
{}


unsigned int LptaSkin::AddTexture(LptaTexture::ID textureId)
{
    if (textureIds.size() < MAX_TEXTURES) {
        textureIds.push_back(textureId);
    }
    return NumTextures();
}

}