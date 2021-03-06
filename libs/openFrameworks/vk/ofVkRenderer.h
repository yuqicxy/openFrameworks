#pragma once

#include <vulkan/vulkan.hpp>

#include "vk/Swapchain.h"
#include "vk/HelperTypes.h"
#include "vk/BufferAllocator.h"
#include "vk/ImageAllocator.h"
#include "vk/DrawCommand.h"
#include "vk/ComputeCommand.h"
#include "vk/RenderBatch.h"
#include "vk/Texture.h"

#include "ofBaseTypes.h"
#include "ofPolyline.h"
#include "ofMatrix4x4.h"
#include "ofMatrixStack.h"
#include "of3dGraphics.h"
#include "ofBitmapFont.h"
#include "ofPath.h"
#include "ofMesh.h"
#include <deque>

#define RENDERER_FUN_NOT_IMPLEMENTED {                                   \
	ofLogVerbose() << __FUNCTION__ << ": not implemented in VkRenderer.";\
}                                                                        \


class ofShapeTessellation;


namespace of{
namespace vk{
	class Context;
	class Shader;
}	// end namespace of::vk
};  // end namespace of


class ofVkRenderer : public ofBaseRenderer
{

	bool bBackgroundAuto;
	bool wrongUseLoggedOnce;

	const ofBaseMaterial * currentMaterial;

	ofStyle currentStyle;
	std::deque <ofStyle> styleHistory;
	of3dGraphics m3dGraphics;
	ofBitmapFont bitmapFont;
	ofPath mPath;
	const ofAppBaseWindow * window;

public:
	static const std::string TYPE;

	const of::vk::RendererSettings mSettings;

	ofVkRenderer( const ofAppBaseWindow * window, of::vk::RendererSettings settings ); 
	
	void setup();
	virtual ~ofVkRenderer() override;

	virtual const std::string & getType() override{
		return TYPE;
	};

	virtual void startRender() override;
	virtual void finishRender() override;

	const uint32_t getSwapChainSize();

	virtual void draw( const ofPolyline & poly )const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofPath & shape ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofMesh & vertexData, ofPolyRenderMode renderType, bool useColors, bool useTextures, bool useNormals ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const of3dPrimitive& model, ofPolyRenderMode renderType ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofNode& model ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofImage & image, float x, float y, float z, float w, float h, float sx, float sy, float sw, float sh ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofFloatImage & image, float x, float y, float z, float w, float h, float sx, float sy, float sw, float sh ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofShortImage & image, float x, float y, float z, float w, float h, float sx, float sy, float sw, float sh ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void draw( const ofBaseVideoDraws & video, float x, float y, float w, float h ) const RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void pushView() RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void popView()  RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void viewport( ofRectangle viewport ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void viewport( float x = 0, float y = 0, float width = -1, float height = -1, bool vflip = true ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setupScreenPerspective( float width = -1, float height = -1, float fov = 60, float nearDist = 0, float farDist = 0 ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setupScreenOrtho( float width = -1, float height = -1, float nearDist = -1, float farDist = 1 ) RENDERER_FUN_NOT_IMPLEMENTED;
	
	virtual void setOrientation( ofOrientation orientation, bool vFlip ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual ofRectangle getCurrentViewport() const override;
	virtual ofRectangle getNativeViewport() const override;
	virtual int getViewportWidth() const override;
	virtual int getViewportHeight() const override;

	virtual bool isVFlipped() const override;
	virtual void setCoordHandedness( ofHandednessType handedness ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual ofHandednessType getCoordHandedness() const override;

	virtual void pushMatrix() override RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void popMatrix() override RENDERER_FUN_NOT_IMPLEMENTED;

	virtual glm::mat4x4 getCurrentMatrix( ofMatrixMode matrixMode_ ) const;
	virtual glm::mat4x4 getCurrentOrientationMatrix() const;

	virtual void translate( float x, float y, float z = 0 ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void translate( const glm::vec3& p ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void scale( float xAmnt, float yAmnt, float zAmnt = 1 ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void rotateRad( float degrees, float axisX, float axisY, float axisZ ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void rotateXRad( float degrees ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void rotateYRad( float degrees ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void rotateZRad( float degrees ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void rotateRad( float degrees ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void matrixMode( ofMatrixMode mode ) RENDERER_FUN_NOT_IMPLEMENTED;


	virtual void loadMatrix( const glm::mat4x4 & m ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void loadMatrix( const float *m ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void loadIdentityMatrix( void ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void loadViewMatrix( const glm::mat4x4 & m )RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void multViewMatrix( const glm::mat4x4 & m )RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void multMatrix( const glm::mat4x4 & m )RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void multMatrix( const float *m )RENDERER_FUN_NOT_IMPLEMENTED;

	virtual glm::mat4x4 getCurrentViewMatrix() const override;
	virtual glm::mat4x4 getCurrentNormalMatrix() const override;

	virtual void bind( const ofCamera & camera, const ofRectangle & viewport ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void unbind( const ofCamera & camera ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setupGraphicDefaults()RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setupScreen()RENDERER_FUN_NOT_IMPLEMENTED;

	void resizeScreen( int w, int h );

	virtual void setRectMode( ofRectMode mode )RENDERER_FUN_NOT_IMPLEMENTED;

	virtual ofRectMode getRectMode() override;

	virtual void setFillMode( ofFillFlag fill ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual ofFillFlag getFillMode() override;

	virtual void setLineWidth( float lineWidth ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setDepthTest( bool depthTest ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setBlendMode( ofBlendMode blendMode )RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setLineSmoothing( bool smooth ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setCircleResolution( int res ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void enableAntiAliasing() RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void disableAntiAliasing() RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setColor( int r, int g, int b ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setColor( int r, int g, int b, int a ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setColor( const ofColor & color ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setColor( const ofColor & color, int _a ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setColor( int gray ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setHexColor( int hexColor ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setBitmapTextMode( ofDrawBitmapMode mode ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual ofColor getBackgroundColor();
	virtual void setBackgroundColor( const ofColor & c ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void background( const ofColor & c ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void background( float brightness ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void background( int hexColor, float _a = 255.0f ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void background( int r, int g, int b, int a = 255 ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void setBackgroundAuto( bool bManual ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual bool getBackgroundAuto() override;

	virtual void clear() RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void clear( float r, float g, float b, float a = 0 ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void clear( float brightness, float a = 0 ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void clearAlpha() RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void drawLine( float x1, float y1, float z1, float x2, float y2, float z2 ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawRectangle( float x, float y, float z, float w, float h ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawTriangle( float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3 ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawCircle( float x, float y, float z, float radius ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawEllipse( float x, float y, float z, float width, float height ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawString( std::string text, float x, float y, float z ) const RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void drawString( const ofTrueTypeFont & font, std::string text, float x, float y ) const RENDERER_FUN_NOT_IMPLEMENTED;

	virtual ofPath & getPath() override;
	virtual ofStyle getStyle() const;
	virtual void setStyle( const ofStyle & style ) RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void pushStyle() RENDERER_FUN_NOT_IMPLEMENTED;
	virtual void popStyle() RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setCurveResolution( int resolution ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual void setPolyMode( ofPolyWindingMode mode ) RENDERER_FUN_NOT_IMPLEMENTED;

	virtual const of3dGraphics & get3dGraphics() const override;
	virtual of3dGraphics & get3dGraphics() override;

private:


	void                                   createInstance();
	void                                   destroyInstance();

	void                                   createDevice();
	void                                   destroyDevice();

	//void                                   destroySurface();

	VkDebugReportCallbackEXT               mDebugReportCallback = nullptr;
	VkDebugReportCallbackCreateInfoEXT     mDebugCallbackCreateInfo = {};

	void                                   requestDebugLayers();
	void                                   createDebugLayers();
	void                                   destroyDebugLayers();

	of::vk::RendererProperties             mRendererProperties;
	
	::vk::Instance                         &mInstance                       = mRendererProperties.instance;
	::vk::Device                           &mDevice                         = mRendererProperties.device;
	::vk::PhysicalDevice                   &mPhysicalDevice                 = mRendererProperties.physicalDevice;
	::vk::PhysicalDeviceProperties         &mPhysicalDeviceProperties       = mRendererProperties.physicalDeviceProperties;
	::vk::PhysicalDeviceMemoryProperties   &mPhysicalDeviceMemoryProperties = mRendererProperties.physicalDeviceMemoryProperties;
	uint32_t                               &mVkGraphicsQueueFamilyIndex     = mRendererProperties.graphicsFamilyIndex;

	std::vector<const char*>               mInstanceLayers;                                // debug layer list for instance
	std::vector<const char*>               mInstanceExtensions;                            // debug layer list for device
	std::vector<const char*>               mDeviceLayers;                                  // debug layer list for device
	std::vector<const char*>               mDeviceExtensions;                              // debug layer list for device

	std::shared_ptr<::vk::PipelineCache>   mPipelineCache;

public:

	// return handle to renderer's vkDevice
	// CONSIDER: error checking for when device has not been aqcuired yet.
	const ::vk::Device& getVkDevice() const;

	const ::vk::PhysicalDeviceProperties& getVkPhysicalDeviceProperties() const;

	const ::vk::PhysicalDeviceMemoryProperties& getVkPhysicalDeviceMemoryProperties() const;

	// Return requested number of virtual frames (n.b. that's not swapchain frames) for this renderer
	// virtual frames are frames that are produced and submitted to the swapchain.
	// Once submitted, they are re-used as soon as their respective fence signals that
	// they have finished rendering.
	const size_t getVirtualFramesCount();

	const std::shared_ptr<::vk::PipelineCache>& getPipelineCache();

	// return default renderpass - a renderpass which has one color buffer and one depth buffer
	const std::shared_ptr<::vk::RenderPass>& getDefaultRenderpass();

private:

	ofRectangle mViewport;

	void postSetup( ofEventArgs & args );
	
	void                     setupSwapChain();
	void                     setupDepthStencil();
	void                     setupDefaultContext();
	void                     setupStagingContext();
	

	// vector of queues - the queue index is based on the index of the queue creation request
	// it is assumed that queue 0 is graphics capable.
	std::vector<::vk::Queue> mQueues;
	std::vector<std::mutex>  mQueueMutex;

	// Depth buffer format
	// Depth format is selected during Vulkan initialization, in createDevice()
	::vk::Format mDepthFormat;

	// our depth stencil: 
	// we only need one since there is only ever one frame in flight.
	struct DepthStencilResource
	{
		::vk::Image image      = nullptr;
		::vk::DeviceMemory mem = nullptr;
		::vk::ImageView view   = nullptr;
	};

	// Depthstencil image used for depth tests with default context.
	std::unique_ptr<DepthStencilResource, std::function<void(DepthStencilResource*)>> mDepthStencil;

	// vulkan swapchain
	std::shared_ptr<of::vk::Swapchain> mSwapchain;

	// The default context
	// This is the context which will render to the swapchain
	std::shared_ptr<of::vk::Context> mDefaultContext;

	// The staging context
	// This Context is used to stage resources such as texture data so that it can be 
	// transferred to device local memory - note that staging context will get submitted before first draw.
	std::shared_ptr<of::vk::Context> mStagingContext;


	// default render pass - 
	std::shared_ptr<::vk::RenderPass> mDefaultRenderPass;

public:

	const ::vk::Instance& getInstance();

	void setSwapchain( std::shared_ptr<of::vk::Swapchain> swapchain_ ){
		mSwapchain = swapchain_;
	};

	std::shared_ptr<::vk::RenderPass> generateDefaultRenderPass(::vk::Format colorFormat_, ::vk::Format depthFormat_) const;

	const std::shared_ptr<of::vk::Context> & getDefaultContext();

	const std::shared_ptr<of::vk::Context> & getStagingContext();

	void setDefaultContext( std::shared_ptr<of::vk::Context> ctx );

	std::shared_ptr<of::vk::Swapchain> & getSwapchain();

	const of::vk::RendererProperties & getVkRendererProperties();

	const ::vk::Format& getVkDepthFormat();

	void submit( size_t queueIndex, ::vk::ArrayProxy<const ::vk::SubmitInfo>&& submits, const ::vk::Fence& fence );

	const ::vk::ImageView& getDepthStencilImageView();

};

// ----------------------------------------------------------------------
// inline methods

inline const ::vk::Device& ofVkRenderer::getVkDevice() const{
	return mDevice;
};

inline const ::vk::PhysicalDeviceProperties& ofVkRenderer::getVkPhysicalDeviceProperties() const{
	return mPhysicalDeviceProperties;
}

inline const::vk::PhysicalDeviceMemoryProperties & ofVkRenderer::getVkPhysicalDeviceMemoryProperties() const{
	return mPhysicalDeviceMemoryProperties;
}


inline const size_t ofVkRenderer::getVirtualFramesCount(){
	return mSettings.numVirtualFrames;
}

inline const std::shared_ptr<of::vk::Context>& ofVkRenderer::getDefaultContext(){
	return mDefaultContext;
}

inline const std::shared_ptr<of::vk::Context>& ofVkRenderer::getStagingContext(){
	return mStagingContext;
}

inline void ofVkRenderer::setDefaultContext( std::shared_ptr<of::vk::Context> ctx ){
	mDefaultContext = ctx;
}

inline std::shared_ptr<of::vk::Swapchain>& ofVkRenderer::getSwapchain(){
	return mSwapchain;
}

inline const of::vk::RendererProperties & ofVkRenderer::getVkRendererProperties(){
	return mRendererProperties;
}

inline const::vk::Format & ofVkRenderer::getVkDepthFormat(){
	return mDepthFormat;
}

inline const std::shared_ptr<::vk::RenderPass>& ofVkRenderer::getDefaultRenderpass(){
	return mDefaultRenderPass;
}

inline const ::vk::ImageView & ofVkRenderer::getDepthStencilImageView(){
	return mDepthStencil->view;
}


// ----------------------------------------------------------------------
// clean up macros
#ifdef RENDERER_FUN_NOT_IMPLEMENTED
	#undef RENDERER_FUN_NOT_IMPLEMENTED
#endif
