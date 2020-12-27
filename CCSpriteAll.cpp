#include "grid3d/CCSpriteAll.h"
#include "2d/CCSpriteFrameCache.h"

NS_CC_BEGIN

SpriteAll* SpriteAll::create(const std::string& filename)
{
	SpriteAll *sprite = new (std::nothrow) SpriteAll();
	if (sprite && sprite->initWithFile(filename))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

SpriteAll* SpriteAll::createWithSpriteFrameName(const std::string& spriteFrameName)
{
	SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);

#if COCOS2D_DEBUG > 0
	char msg[256] = { 0 };
	sprintf(msg, "Invalid spriteFrameName: %s", spriteFrameName.c_str());
	CCASSERT(frame != nullptr, msg);
#endif

	return createWithSpriteFrame(frame);
}

SpriteAll* SpriteAll::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
	SpriteAll *sprite = new (std::nothrow) SpriteAll();
	if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

void SpriteAll::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	if (_texture == nullptr)
	{
		return;
	}
	//-- 相对于基类 取消掉各种剔除判断 --
	if (true)
	{
		_trianglesCommand.init(_globalZOrder,
			_texture,
			getGLProgramState(),
			_blendFunc,
			_polyInfo.triangles,
			transform,
			flags);
		renderer->addCommand(&_trianglesCommand);
	}
}



NS_CC_END
