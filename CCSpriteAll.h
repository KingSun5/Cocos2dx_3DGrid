/****************************************************************************
time:2020/12/15
dec:·ÂÕÕCCNodeGrid ÔöÌí3DGridNode
 ****************************************************************************/


#include "2d/CCSprite.h"
#include "renderer/CCRenderer.h"
#include "base/CCDirector.h"

NS_CC_BEGIN

class CC_DLL SpriteAll : public Sprite
{
public:
	static SpriteAll* create(const std::string& filename);
	virtual void draw(Renderer * renderer, const Mat4 & transform, uint32_t flags) override;
	static SpriteAll* createWithSpriteFrame(SpriteFrame *spriteFrame);
	static SpriteAll* createWithSpriteFrameName(const std::string& spriteFrameName);
};
/** @} */
NS_CC_END

//#endif
