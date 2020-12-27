#include "grid3d/CCNodeGrid3D.h"
#include "grid3d/CCGrid3D.h"

NS_CC_BEGIN

NodeGrid3D* NodeGrid3D::create()
{
	NodeGrid3D * ret = new (std::nothrow) NodeGrid3D();
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

NodeGrid3D::NodeGrid3D()
	: _nodeGrid3D(nullptr)
{

}
NodeGrid3D::~NodeGrid3D()
{
	CC_SAFE_RELEASE(_nodeGrid3D);
}

void NodeGrid3D::onGridBeginDraw()
{
	if (_nodeGrid3D)
	{
		_nodeGrid3D->dealEventBeforeDraw();
	}
}

void NodeGrid3D::onGridEndDraw()
{
	if (_nodeGrid3D)
	{
		_nodeGrid3D->dealEventAfterDraw(this);
	}
}

void NodeGrid3D::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	// quick return if not visible. children won't be drawn.
	if (!_visible)
	{
		return;
	}

	bool dirty = (parentFlags & FLAGS_TRANSFORM_DIRTY) || _transformUpdated;
	if (dirty)
		_modelViewTransform = this->transform(parentTransform);
	_transformUpdated = false;

	_groupCommand.init(_globalZOrder);
	renderer->addCommand(&_groupCommand);
	renderer->pushGroup(_groupCommand.getRenderQueueID());

	// IMPORTANT:
	// To ease the migration to v3.0, we still support the Mat4 stack,
	// but it is deprecated and your code should not rely on it
	Director* director = Director::getInstance();
	CCASSERT(nullptr != director, "Director is null when setting matrix stack");

	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

	_gridBeginCommand.init(_globalZOrder);
	_gridBeginCommand.func = CC_CALLBACK_0(NodeGrid3D::onGridBeginDraw, this);
	renderer->addCommand(&_gridBeginCommand);

	int i = 0;
	bool visibleByCamera = isVisitableByVisitingCamera();

	if (!_children.empty())
	{
		sortAllChildren();
		// draw children zOrder < 0
		for (auto size = _children.size(); i < size; ++i)
		{
			auto node = _children.at(i);

			if (node && node->getLocalZOrder() < 0)
				node->visit(renderer, _modelViewTransform, dirty);
			else
				break;
		}
		// self draw,currently we have nothing to draw on NodeGrid, so there is no need to add render command
		if (visibleByCamera)
			this->draw(renderer, _modelViewTransform, dirty);

		for (auto it = _children.cbegin() + i, itCend = _children.cend(); it != itCend; ++it) {
			(*it)->visit(renderer, _modelViewTransform, dirty);
		}
	}
	else if (visibleByCamera)
	{
		this->draw(renderer, _modelViewTransform, dirty);
	}

	_gridEndCommand.init(_globalZOrder);
	_gridEndCommand.func = CC_CALLBACK_0(NodeGrid3D::onGridEndDraw, this);
	renderer->addCommand(&_gridEndCommand);
	renderer->popGroup();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void NodeGrid3D::setGrid3D(GridBase3D *grid)
{
	CC_SAFE_RELEASE(_nodeGrid3D);
	CC_SAFE_RETAIN(grid);
	_nodeGrid3D = grid;
}

NS_CC_END
