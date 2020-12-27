/****************************************************************************
time:2020/12/15
dec:·ÂÕÕCCNodeGrid ÔöÌí3DGridNode
 ****************************************************************************/


#include "2d/CCNode.h"
#include "renderer/CCGroupCommand.h"
#include "renderer/CCCustomCommand.h"
#include "renderer/CCRenderer.h"
#include "renderer/ccGLStateCache.h"

NS_CC_BEGIN

class GridBase3D;

class CC_DLL NodeGrid3D : public Node
{
public:

	static NodeGrid3D* create();
	void setGrid3D(GridBase3D *grid3d);
	// overrides
	virtual void visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags) override;

CC_CONSTRUCTOR_ACCESS:
	NodeGrid3D();
	virtual ~NodeGrid3D();

protected:
	void onGridBeginDraw();
	void onGridEndDraw();
	GridBase3D* _nodeGrid3D;
	GroupCommand _groupCommand;
	CustomCommand _gridBeginCommand;
	CustomCommand _gridEndCommand;

	Rect _grid3DRect;

private:
	CC_DISALLOW_COPY_AND_ASSIGN(NodeGrid3D);
};
/** @} */
NS_CC_END

//#endif
