#include "grid3d/CCGrid3D.h"
#include "base/ccMacros.h"
#include "base/ccUtils.h"
#include "2d/CCNode.h"
#include "2d/CCGrabber.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCGLProgramCache.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCRenderState.h"
#include "renderer/CCTexture2D.h"
#include "platform/CCGL.h"
#include "2d/CCCamera.h"

NS_CC_BEGIN
// implementation of GridBase

GridBase3D* GridBase3D::create(const Size& gridSize, const int& tWidth, const int& tHeight, const int& distance)
{
	GridBase3D *pGridBase = new (std::nothrow) GridBase3D();

	if (pGridBase)
	{
		if (pGridBase->initWithSize(gridSize, tWidth, tHeight, distance))
		{
			pGridBase->autorelease();
		}
		else
		{
			CC_SAFE_RELEASE_NULL(pGridBase);
		}
	}

	return pGridBase;
}

bool GridBase3D::initWithSize(const cocos2d::Size &gridSize, const int& tWidth, const int& tHeight, const int& distance)
{
	_interceptDistance = distance;

	Director *director = Director::getInstance();
	Size s = director->getWinSizeInPixels();

	_textureWidth = tWidth;
	_textureHeight = tHeight;

	// we only use rgba8888
	Texture2D::PixelFormat format = Texture2D::PixelFormat::RGBA8888;

	auto dataLen = _textureWidth * _textureHeight * 4;
	void *data = calloc(dataLen, 1);
	if (!data)
	{
		CCLOG("cocos2d: Grid: not enough memory.");
		this->release();
		return false;
	}

	Texture2D *texture = new (std::nothrow) Texture2D();
	texture->initWithData(data, dataLen, format, _textureWidth, _textureHeight, Size(_textureWidth, _textureHeight));

	free(data);

	if (!texture)
	{
		CCLOG("cocos2d: Grid: error creating texture");
		return false;
	}

	initWithSize(gridSize, texture, false, Rect::ZERO);

	texture->release();

	return true;
}

void GridBase3D::initWithSize(const Size& gridSize, Texture2D *texture, bool flipped, const Rect& rect)
{
	_gridSize = gridSize;
	_texture = texture;
	CC_SAFE_RETAIN(_texture);

	if (rect.equals(Rect::ZERO)) {
		auto size = _texture->getContentSize();
		_gridRect.setRect(0, 0, size.width, size.height);
	}
	else {
		_gridRect = rect;
	}
	_step.x = _gridRect.size.width / _gridSize.width;
	_step.y = _gridRect.size.height / _gridSize.height;

	_grabber = new (std::nothrow) Grabber();
	if (_grabber)
	{
		_grabber->grab(_texture);
	}

	_shaderProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE);
	calculateVertexPoints();
}

GridBase3D::GridBase3D()
	: _texCoordinates(nullptr)
	, _vertices(nullptr)
	, _originalVertices(nullptr)
	, _indices(nullptr)
{

}

GridBase3D::~GridBase3D(void)
{
	CCLOGINFO("deallocing GridBase: %p", this);

	//TODO: ? why 2.0 comments this line:        setActive(false);
	CC_SAFE_RELEASE(_texture);
	CC_SAFE_RELEASE(_grabber);
	CC_SAFE_FREE(_texCoordinates);
	CC_SAFE_FREE(_vertices);
	CC_SAFE_FREE(_indices);
	CC_SAFE_FREE(_originalVertices);
}

void GridBase3D::dealEventBeforeDraw(void)
{
	beforeDraw();
}

void GridBase3D::dealEventAfterDraw(Node * target)
{
	afterDraw(target);
}

void GridBase3D::set2DProjection()
{
	Director *director = Director::getInstance();
	Size    size = director->getWinSizeInPixels();

	director->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

	Mat4 orthoMatrix;
	Mat4::createOrthographicOffCenter(0, _textureWidth, 0, _textureHeight, -1, 1, &orthoMatrix);
	director->multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, orthoMatrix);

	director->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

	GL::setProjectionMatrixDirty();
}

void GridBase3D::set3DProjection()
{
	Director *director = Director::getInstance();
	Size  size = director->getWinSizeInPixels();
	float zeye = director->getZEye();

	Mat4 matrixPerspective, matrixLookup;
	//-- 重新设置透视投影 主要是为了加大远截面可视距离 目前指定 5000 --
	Mat4::createPerspective(60, (GLfloat)size.width / size.height, 10, _interceptDistance, &matrixPerspective);

	Vec3 eye(size.width / 2, size.height / 2, zeye), center(size.width / 2, size.height / 2, 0.0f), up(0.0f, 1.0f, 0.0f);
	Mat4::createLookAt(eye, center, up, &matrixLookup);
	Mat4 proj3d = matrixPerspective * matrixLookup;

	director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, proj3d);
	director->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	GL::setProjectionMatrixDirty();
}


void GridBase3D::beforeDraw(void)
{
	// save projection
	Director *director = Director::getInstance();
	// 2d projection
	set2DProjection();
	Size    size = director->getWinSizeInPixels();
	glViewport(0, 0, (GLsizei)(_textureWidth), (GLsizei)(_textureHeight));
	_grabber->beforeRender(_texture);
}

void GridBase3D::afterDraw(cocos2d::Node * /*target*/)
{
	_grabber->afterRender(_texture);
	set3DProjection();
	// restore projection
	Director *director = Director::getInstance();
	//director->setProjection(_directorProjection);
	director->setViewport();
	const auto& vp = Camera::getDefaultViewport();
	glViewport(vp._left, vp._bottom, vp._width, vp._height);
	GL::bindTexture2D(_texture->getName());
	blit();
}

void GridBase3D::blit(void)
{
	int n = _gridSize.width * _gridSize.height;
	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_TEX_COORD);
	_shaderProgram->use();
	_shaderProgram->setUniformsForBuiltins();
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, _vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, _texCoordinates);
	glDrawElements(GL_TRIANGLES, (GLsizei)n * 6, GL_UNSIGNED_SHORT, _indices);
}

void GridBase3D::calculateVertexPoints(void)
{
	float width = (float)_texture->getPixelsWide();
	float height = (float)_texture->getPixelsHigh();
	float imageH = _texture->getContentSizeInPixels().height;

	int x, y, i;
	CC_SAFE_FREE(_vertices);
	CC_SAFE_FREE(_originalVertices);
	CC_SAFE_FREE(_texCoordinates);
	CC_SAFE_FREE(_indices);

	unsigned int numOfPoints = (_gridSize.width + 1) * (_gridSize.height + 1);
	_vertices = malloc(numOfPoints * sizeof(Vec3));
	_originalVertices = malloc(numOfPoints * sizeof(Vec3));
	_texCoordinates = malloc(numOfPoints * sizeof(Vec2));
	_indices = (GLushort*)malloc(_gridSize.width * _gridSize.height * sizeof(GLushort) * 6);

	GLfloat *vertArray = (GLfloat*)_vertices;
	GLfloat *texArray = (GLfloat*)_texCoordinates;
	GLushort *idxArray = _indices;

	for (x = 0; x < _gridSize.width; ++x)
	{
		for (y = 0; y < _gridSize.height; ++y)
		{
			int idx = (y * _gridSize.width) + x;

			GLfloat x1 = x * _step.x + _gridRect.origin.x;
			GLfloat x2 = x1 + _step.x;
			GLfloat y1 = y * _step.y + _gridRect.origin.y;
			GLfloat y2 = y1 + _step.y;

			GLushort a = (GLushort)(x * (_gridSize.height + 1) + y);
			GLushort b = (GLushort)((x + 1) * (_gridSize.height + 1) + y);
			GLushort c = (GLushort)((x + 1) * (_gridSize.height + 1) + (y + 1));
			GLushort d = (GLushort)(x * (_gridSize.height + 1) + (y + 1));

			GLushort tempidx[6] = { a, b, d, b, c, d };

			memcpy(&idxArray[6 * idx], tempidx, 6 * sizeof(GLushort));

			int l1[4] = { a * 3, b * 3, c * 3, d * 3 };
			Vec3 e(x1, y1, 0);
			Vec3 f(x2, y1, 0);
			Vec3 g(x2, y2, 0);
			Vec3 h(x1, y2, 0);

			Vec3 l2[4] = { e, f, g, h };

			int tex1[4] = { a * 2, b * 2, c * 2, d * 2 };
			Vec2 Tex2F[4] = { Vec2(x1, y1), Vec2(x2, y1), Vec2(x2, y2), Vec2(x1, y2) };

			for (i = 0; i < 4; ++i)
			{
				vertArray[l1[i]] = l2[i].x;
				vertArray[l1[i] + 1] = l2[i].y;
				vertArray[l1[i] + 2] = l2[i].z;

				texArray[tex1[i]] = Tex2F[i].x / width;
				texArray[tex1[i] + 1] = Tex2F[i].y / height;
			}
		}
	}

	memcpy(_originalVertices, _vertices, (_gridSize.width + 1) * (_gridSize.height + 1) * sizeof(Vec3));
}

Vec3 GridBase3D::getOriginalVertex(const Vec2& pos) const
{
	CCASSERT(pos.x == (unsigned int)pos.x && pos.y == (unsigned int)pos.y, "Numbers must be integers");

	int index = (pos.x * (_gridSize.height + 1) + pos.y) * 3;
	float *vertArray = (float*)_originalVertices;

	Vec3 vert(vertArray[index], vertArray[index + 1], vertArray[index + 2]);

	return vert;
}

void GridBase3D::setVertex(const Vec2& pos, const Vec3& vertex)
{
	CCASSERT(pos.x == (unsigned int)pos.x && pos.y == (unsigned int)pos.y, "Numbers must be integers");
	int index = (pos.x * (_gridSize.height + 1) + pos.y) * 3;
	float *vertArray = (float*)_vertices;
	vertArray[index] = vertex.x;
	vertArray[index + 1] = vertex.y;
	vertArray[index + 2] = vertex.z;
}

NS_CC_END
