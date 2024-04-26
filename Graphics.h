#pragma once
class Graphics
{
public: 
	Graphics();
	~Graphics();

	void init();
	void render();
	void renderScene();
	void uploadLightning();

	void uploadCamera();

	void initVoxelization();
	void voxelize();
	void initVoxelVisualization();
	void renderVoxelVisualization();



};

