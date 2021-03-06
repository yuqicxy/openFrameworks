#pragma once

#include "ofMain.h"
#include "vk/ofVkRenderer.h"

struct StaticMesh
{
	of::vk::BufferRegion indexBuffer;
	of::vk::BufferRegion posBuffer;
	of::vk::BufferRegion normalBuffer;
	of::vk::BufferRegion texCoordBuffer;
};

class ofApp : public ofBaseApp{

	of::vk::DrawCommand drawPhong;
	of::vk::DrawCommand drawFullScreenQuad;
	of::vk::DrawCommand drawTextured;

	of::vk::ComputeCommand computeCmd;

	ofEasyCam mCam;

	std::shared_ptr<ofMesh> mMeshL;
	std::shared_ptr<ofMesh> mMeshPly;

	of::vk::BufferAllocator mStaticAllocator;
	of::vk::ImageAllocator  mImageAllocator;

	std::shared_ptr<::vk::Image>       mImage;
	of::vk::Texture                    mTexture;

	StaticMesh mStaticMesh;
	StaticMesh mRectangleData;

	of::vk::BufferRegion mStaticColourBuffer;
	of::vk::BufferRegion mParticlesRegion;

	public:
		void setup();
		void setupStaticAllocators();
		void setupDrawCommands();
		void setupMeshL(){
			// Horizontally elongated "L___" shape
			mMeshL = make_shared<ofMesh>();
			vector<glm::vec3> vert{
				{ 0.f,0.f,0.f },
				{ 20.f,20.f,0.f },
				{ 0.f,100.f,0.f },
				{ 20.f,100.f,0.f },
				{ 200.f,0.f,0.f },
				{ 200.f,20.f,0.f }
			};

			vector<ofIndexType> idx{
				0, 1, 2,
				1, 3, 2,
				0, 4, 1,
				1, 4, 5,
			};

			vector<glm::vec3> norm( vert.size(), { 0, 0, 1.f } );

			mMeshL->addVertices( vert );
			mMeshL->addNormals( norm );
			mMeshL->addIndices( idx );
		}
		void update();
		void draw();

		void uploadStaticData( of::vk::Context & stagingContext );

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
};
