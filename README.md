# Cocos2dx_3DGrid
cocos2dx利用网格Grid实现3d的透视效果



##### C++ 用例
 
    //创建NodeGrid3D
	auto gridNode = NodeGrid3D::create();
	scene->addChild(gridNode);
	//创建GridBase3D
	auto grid3D = GridBase3D::create(Size(100, 500), 720, 2048, 5000);
	gridNode->setGrid3D(grid3D);
	//创建精灵添加在NodeGrid3D上
	for (int i = 0; i < 5; i++)
	{
		auto sp = SpriteAll::create("Hello.png");
		gridNode->addChild(sp);
		sp->setPosition(Vec2(360,160+322*i));
		sp->runAction(MoveBy::create(8, Vec2(0, -1000)));
	}
	//重新设置网格的顶点信息
	for (int col = 0; col < 100; col++)
	{
		for (int row = 0; row < 500; row++)
		{
			auto vPos = grid3D->getOriginalVertex(Vec2(col, row));
			//纵坐标大于150的网格Z值被修改
			if (row > 150) {
				
				auto zValue = -(row - 150) * 5;
				grid3D->setVertex(Vec2(col, row), Vec3(vPos.x, vPos.y, zValue));
			
			}
		}
	}
