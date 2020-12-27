
#include "base/CCRef.h"
#include "base/ccTypes.h"
#include "base/CCDirector.h"

NS_CC_BEGIN

class Texture2D;
class Grabber;
class GLProgram;
class Node;

class CC_DLL GridBase3D : public Ref
{
public:
	/** create one Grid */
	static GridBase3D* create(const Size& gridSize, const int& tWidth, const int& tHeight, const int& distance);
	GridBase3D();
	/**
	Destructor.
	 * @js NA
	 * @lua NA
	 */
	~GridBase3D(void);
	/**@}*/
		/**@{
	 Init and reset the status when render effects by using the grid.
	 */
	void dealEventBeforeDraw(void);
	void dealEventAfterDraw(Node *target);
	/**@}*/
		/** Returns the original (non-transformed) vertex at a given position.
	 * @js NA
	 * @lua NA
	 */
	Vec3 getOriginalVertex(const Vec2& pos) const;
	/** Sets a new vertex at a given position.
	 * @lua NA
	 */
	void setVertex(const Vec2& pos, const Vec3& vertex);

protected:
	Size _gridSize;
	Texture2D *_texture;
	Vec2 _step;
	Grabber *_grabber;
	GLProgram* _shaderProgram;
	Rect _gridRect;
	GLvoid *_texCoordinates;
	GLvoid *_vertices;
	GLvoid *_originalVertices;
	GLushort *_indices;
	int _textureWidth;
	int _textureHeight;
	int _interceptDistance;
	bool initWithSize(const Size& gridSize, const int& tWidth, const int& tHeight, const int& distance);
	void initWithSize(const Size& gridSize, Texture2D *texture, bool flipped, const Rect& rect);

	/**Interface used to blit the texture with grid to screen.*/
	void blit(void);
	/**Interface, Calculate the vertices used for the blit.*/
	void calculateVertexPoints(void);

	/**Change projection to 2D for grabbing.*/
	void set2DProjection(void);
	void set3DProjection(void);
	void beforeDraw(void);
	void afterDraw(Node *target);
};

NS_CC_END

