#include "ofVkRenderer.h"
#include "ofMesh.h"
#include "ofPath.h"
#include "ofBitmapFont.h"
#include "ofImage.h"
#include "of3dPrimitives.h"
#include "ofLight.h"
#include "ofMaterial.h"
#include "ofCamera.h"
#include "ofTrueTypeFont.h"
#include "ofNode.h"
#include "GLFW/glfw3.h"

#include <cstdint>

const string ofVkRenderer::TYPE = "Vulkan";

using InstanceP = shared_ptr<VkInstance>;

// ----------------------------------------------------------------------

// fetched function pointers for debug layer callback creation / destruction
// functions. As these are not directly exposed within the sdk, we have to 
// first query the sdk for the function pointers to these.
PFN_vkCreateDebugReportCallbackEXT  fVkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fVkDestroyDebugReportCallbackEXT = nullptr;

// ----------------------------------------------------------------------

ofVkRenderer::ofVkRenderer(const ofAppBaseWindow * _window, Settings settings )
	: m3dGraphics(this)
	, mSettings(settings)
{
	bBackgroundAuto = true;
	wrongUseLoggedOnce = false;

	currentMaterial = nullptr;
	

	window = _window;

	mPath.setMode(ofPath::POLYLINES);
	mPath.setUseShapeColor(false);

	// vulkan specifics
	setupDebugLayers();

#ifdef OF_TARGET_API_VULKAN
#ifdef TARGET_LINUX
	mInstanceExtensions.push_back( "VK_KHR_xcb_surface" );
#endif
#ifdef TARGET_WIN32
	mInstanceExtensions.push_back( "VK_KHR_win32_surface" );
#endif
	mInstanceExtensions.push_back( "VK_KHR_surface" );
	mDeviceExtensions.push_back( "VK_KHR_swapchain" );
#endif

	createInstance();

	// important to call createDebugLayers() after createInstance, 
	// since otherwise the debug layer create function pointers will not be 
	// correctly resolved, since the main library would not yet have been loaded.
	createDebugLayers();
	
	// createDevice also initialises the device queue, mQueue
	createDevice();

	// up next: create window surface (this happens within glfw)
}

// ----------------------------------------------------------------------

const VkInstance& ofVkRenderer::getInstance() {
	return mInstance;
}

// ----------------------------------------------------------------------

const VkSurfaceKHR& ofVkRenderer::getWindowSurface() {
	return mWindowSurface;
}

// ----------------------------------------------------------------------

ofVkRenderer::~ofVkRenderer()
{
	// Tell GPU to finish whatever it is doing 
	// and to catch up with the CPU waiting right here.
	//
	// This is a sync method so harsh, it should
	// only ever be used for teardown. 
	//
	// Which is what this method is doing.
	auto err= vkDeviceWaitIdle(mDevice);
	assert( !err );


	mDefaultContext.reset();
	mShaderManager.reset();

	// reset command pool and all associated command buffers.
	err = vkResetCommandPool( mDevice, mCommandPool, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
	assert( !err );

	vkDestroyRenderPass( mDevice, mRenderPass, nullptr );

	for ( auto &f : mFrameBuffers ){
		vkDestroyFramebuffer( mDevice, f, nullptr );
	}
	mFrameBuffers.clear();



	vkDestroyImageView( mDevice, mDepthStencil.view, nullptr );
	vkDestroyImage( mDevice, mDepthStencil.image, nullptr );
	vkFreeMemory( mDevice, mDepthStencil.mem, nullptr );

	vkDestroyCommandPool( mDevice, mCommandPool, VK_NULL_HANDLE );
	vkDestroySemaphore( mDevice, mSemaphorePresentComplete, nullptr );
	vkDestroySemaphore( mDevice, mSemaphoreRenderComplete, nullptr );

	mSwapchain.reset();

	destroyDevice();
	destroySurface();
	destroyDebugLayers();
	destroyInstance();
}

// ----------------------------------------------------------------------

void ofVkRenderer::destroySurface(){
	vkDestroySurfaceKHR( mInstance, mWindowSurface, nullptr );
	mWindowSurface = VK_NULL_HANDLE;
}

// ----------------------------------------------------------------------

 void ofVkRenderer::createInstance()
{
	ofLog() << "createInstance";

	
	std::string appName = "openFrameworks" + ofGetVersionInfo();

	VkApplicationInfo applicationInfo{};
	{
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 8);	// !TODO use version info from windowSettings
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		applicationInfo.pApplicationName = appName.c_str();
	}

	VkInstanceCreateInfo instanceCreateInfo{};
	{
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;

		instanceCreateInfo.enabledLayerCount       = uint32_t(mInstanceLayers.size());
		instanceCreateInfo.ppEnabledLayerNames     = mInstanceLayers.data();
		instanceCreateInfo.enabledExtensionCount   = uint32_t(mInstanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = mInstanceExtensions.data();

		// this enables debugging the instance creation.
		// it is slightly weird.
		instanceCreateInfo.pNext                   = &debugCallbackCreateInfo;
	}

	auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);

	if (err != VK_SUCCESS) {
		ofLogError() << "Could not create Instance: " << err;
		std::exit(-1);
	}

	ofLog() << "Successfully created instance.";
}

// ----------------------------------------------------------------------

void ofVkRenderer::destroyInstance()
{
	vkDestroyInstance(mInstance, VK_NULL_HANDLE);
	mInstance = VK_NULL_HANDLE;
}
// ----------------------------------------------------------------------

void ofVkRenderer::createDevice()
{
	// enumerate physical devices list to find 
	// first available device
	{
		uint32_t numDevices = 0;
		// get the count for physical devices 
		// how many GPUs are available?
		auto err = vkEnumeratePhysicalDevices(mInstance, &numDevices, VK_NULL_HANDLE);
		assert( !err );

		std::vector<VkPhysicalDevice> deviceList(numDevices);
		err = vkEnumeratePhysicalDevices(mInstance, &numDevices, deviceList.data());
		assert( !err );

		// TODO: find the best appropriate GPU
		// Select a physical device (GPU) from the above queried list of options.
		// For now, we assume the first one to be the best one.
		mPhysicalDevice = deviceList.front();

		// query the gpu for more info about itself
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);

		std::ostringstream output;
		auto const & d = mPhysicalDeviceProperties;
		output << "GPU Type: " << d.deviceName << std::endl;
		
		{
			ofVkWindowSettings tmpSettings;
			tmpSettings.vkVersion = d.apiVersion;
			output << "Api Version: " << tmpSettings.getVkVersionMajor() << "."
				<< tmpSettings.getVersionMinor() << "." << tmpSettings.getVersionPatch();
		}
		
		ofLog() << output.str();

		// let's find out the devices' memory properties
		vkGetPhysicalDeviceMemoryProperties( mPhysicalDevice, &mPhysicalDeviceMemoryProperties );
	}

	// query queue families for the first queue supporting graphics
	{
		uint32_t numQFP = 0;
		// query number of queue family properties
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQFP, VK_NULL_HANDLE);
		std::vector<VkQueueFamilyProperties> queueFamilyPropertyList(numQFP);
		vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &numQFP, queueFamilyPropertyList.data());

		bool foundGraphics = false;
		for (uint32_t i = 0; i < queueFamilyPropertyList.size(); ++i) {
			// test queue family against flag bitfields
			if (queueFamilyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				foundGraphics = true;
				mVkGraphicsFamilyIndex = i;
				break;
			}
		}
		if (!foundGraphics) {
			ofLogError() << "Vulkan error: did not find queue family that supports graphics";
			ofExit(-1);
		}
	}

	// query debug layers available for instance
	{
		uint32_t layerCount;
		auto err = vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);
		assert( !err );

		vector<VkLayerProperties> layerPropertyList(layerCount);
		err = vkEnumerateInstanceLayerProperties(&layerCount, layerPropertyList.data());
		assert( !err );


		ofLog() << "Instance Layers:";
		for (auto &l : layerPropertyList) {
			ofLog() << "  " << l.layerName << ": \t\t\t\t |" << l.description;
		}

	}

	// query debug layers available for physical device
	{
		uint32_t layerCount;
		auto err = vkEnumerateDeviceLayerProperties(mPhysicalDevice, &layerCount, VK_NULL_HANDLE);
		assert( !err );

		vector<VkLayerProperties> layerPropertyList(layerCount);
		err = vkEnumerateDeviceLayerProperties(mPhysicalDevice, &layerCount, layerPropertyList.data());
		assert( !err );

		ofLog() << "Device Layers:";
		for (auto &l : layerPropertyList) {
			ofLog() << "  " << l.layerName << ": \t\t\t\t |" << l.description;
		}
	}

	
	// create a device
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	{
		float queuePriority[] = { 1.f };
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.queueFamilyIndex = mVkGraphicsFamilyIndex;
		deviceQueueCreateInfo.queueCount = 1;		// tell vulkan how many queues to allocate
		deviceQueueCreateInfo.pQueuePriorities = queuePriority;
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	{
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

		deviceCreateInfo.enabledLayerCount       = uint32_t(mDeviceLayers.size());
		deviceCreateInfo.ppEnabledLayerNames     = mDeviceLayers.data();
		deviceCreateInfo.enabledExtensionCount   = uint32_t(mDeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();
	}

	auto err = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &mDevice);

	if (err == VK_SUCCESS) {
		ofLogNotice() << "Successfully created Vulkan device";
	} else {
		ofLogError() << "error creating vulkan device: " << err;
		ofExit(-1);
	}

	// fetch queue handle into mQueue
	vkGetDeviceQueue(mDevice, mVkGraphicsFamilyIndex, 0, &mQueue);

	// query possible depth formats, find the 
	// first format that supports attachment as a depth stencil 
	//
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for ( auto& format : depthFormats ){
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties( mPhysicalDevice, format, &formatProps );
		// Format must support depth stencil attachment for optimal tiling
		if ( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ){
			mDepthFormat = format;
			break;
		}
	}

}

// ----------------------------------------------------------------------

void ofVkRenderer::destroyDevice()
{
	vkDestroyDevice(mDevice, VK_NULL_HANDLE);
	mDevice = VK_NULL_HANDLE;
}

// ----------------------------------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT		flags,					// what kind of error are we handling
	VkDebugReportObjectTypeEXT 	objType,				// type of object that caused the error
	uint64_t                    srcObj,					// pointer to the object that caused the error
	size_t                      location,				// ? could be source code line ?
	int32_t                     msgCode,				// ? how important this callback is ?
	const char*                 layer_prefix,			// which layer called this callback
	const char*                 msg,					// user readable string
	void *                      userData
) {
	
#ifdef WIN32
	static HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ){
		SetConsoleTextAttribute( hConsole, 12 + 0 * 16 );
	}
#endif // WIN32
	std::ostringstream os;

	static size_t errorNo = 0;

	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		os << "\t\tINFO : " << msg << std::endl;
	} 
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		os << "\t\tWARN : " << msg << std::endl;
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		os << "\t\tPERF : " << msg << std::endl;
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		os << "\tERROR: " << setw(4) << errorNo++ << "\t" << msg << std::endl;
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		os << "\t\tDEBUG: " << msg << std::endl;
	}

	ofLogNotice("vkRender") << "\t LAYER{" << layer_prefix << "}: " << (os.str().substr(0,os.str().length()-1));
#ifdef WIN32
	SetConsoleTextAttribute( hConsole, 7 + 0 * 16 );
#endif
	// if error returns true, this layer will try to bail out and not forward the command
	return false; 
}

// ----------------------------------------------------------------------

void ofVkRenderer::setupDebugLayers() {

	debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugCallbackCreateInfo.pfnCallback = &VulkanDebugCallback;
	debugCallbackCreateInfo.flags = 
		//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
	    | VK_DEBUG_REPORT_DEBUG_BIT_EXT
		| 0;		  // this should enable all flags.


	// remember: all layers that you plan to use in yout app, i.e. in the device,
	// need to be also activated in the instance.

	// layer check happens last element to first element in the layers array.

	mInstanceLayers.push_back( "VK_LAYER_LUNARG_standard_validation" );
	mInstanceLayers.push_back("VK_LAYER_LUNARG_object_tracker");
	mInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	mDeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	
}
// ----------------------------------------------------------------------

void ofVkRenderer::createDebugLayers()
{
	// first get (find) function pointers from sdk for callback [create / destroy] function addresses
	fVkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
	fVkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT");

	// we can't check against nullptr here, since 0x0 is not the same as nullptr and 
	// we would falsely get a positive feedback even if the sdk returns 0x0 as the address
	// for the function pointers.
	if (VK_NULL_HANDLE == fVkCreateDebugReportCallbackEXT || VK_NULL_HANDLE == fVkDestroyDebugReportCallbackEXT) {
		ofLogError() << "error fetching pointers for debug layer callbacks";
		ofExit(-1);
		return;
	}
				   
	// this method is not available by default.
	{
		// note that we execute the function pointers we searched for earlier, 
		// since "vkCreateDebugReportCallbackEXT" is not directly exposed by vulkan-1.lib
		// fVkCreateDebugReportCallbackEXT is the function we want to call.
		fVkCreateDebugReportCallbackEXT(mInstance, &debugCallbackCreateInfo , VK_NULL_HANDLE, &debugReportCallback);
	}
}

// ----------------------------------------------------------------------

void ofVkRenderer::destroyDebugLayers()
{
	// this function pointer was queried from the sdk in 
	// createDebugLayers()
	fVkDestroyDebugReportCallbackEXT(mInstance, debugReportCallback, VK_NULL_HANDLE);
	// let's set our own callback address to 0 just to be on the safe side.
	debugReportCallback = VK_NULL_HANDLE;
}
// ----------------------------------------------------------------------

ofRectangle ofVkRenderer::getCurrentViewport() const
{
	return mViewport;
}

// ----------------------------------------------------------------------

ofRectangle ofVkRenderer::getNativeViewport() const
{
	return mViewport;
}

// ----------------------------------------------------------------------

int ofVkRenderer::getViewportWidth() const
{
	return mWindowWidth;
}

// ----------------------------------------------------------------------

int ofVkRenderer::getViewportHeight() const
{
	return mWindowHeight;
}

// ----------------------------------------------------------------------

bool ofVkRenderer::isVFlipped() const
{
	return false;
}

// ----------------------------------------------------------------------

ofHandednessType ofVkRenderer::getCoordHandedness() const
{
	return ofHandednessType();
}

// ----------------------------------------------------------------------

glm::mat4x4 ofVkRenderer::getCurrentMatrix(ofMatrixMode matrixMode_) const
{
	return ofMatrix4x4();
}

// ----------------------------------------------------------------------

glm::mat4x4 ofVkRenderer::getCurrentOrientationMatrix() const
{
	return ofMatrix4x4();
}

// ----------------------------------------------------------------------

void ofVkRenderer::pushMatrix(){
	if(mDefaultContext)
		mDefaultContext->pushMatrix();
}

// ----------------------------------------------------------------------

void ofVkRenderer::popMatrix(){
	if ( mDefaultContext )
		mDefaultContext->popMatrix();
}

// ----------------------------------------------------------------------

void ofVkRenderer::translate( const glm::vec3 & p ){
	if ( mDefaultContext )
		mDefaultContext->translate( p );
}

// ----------------------------------------------------------------------

void ofVkRenderer::rotateRad( float radians, float axisX, float axisY, float axisZ ){
	if ( mDefaultContext )
		mDefaultContext->rotateRad( radians, { axisX, axisY, axisZ } );
}

// ----------------------------------------------------------------------

void ofVkRenderer::rotateYRad( float radians ){
	rotateRad( radians,  0, 1, 0 );
}

void ofVkRenderer::rotateZRad( float radians ){
	rotateRad( radians, 0, 0, 1 );
}

// ----------------------------------------------------------------------

void ofVkRenderer::rotateXRad(float radians ) {
	rotateRad( radians, 1, 0, 0 );
};

// ----------------------------------------------------------------------

void ofVkRenderer::rotateRad( float radians ){
	rotateZRad( radians );
}

// ----------------------------------------------------------------------

glm::mat4x4 ofVkRenderer::getCurrentViewMatrix() const
{
	if ( mDefaultContext )
		return mDefaultContext->getViewMatrix();
	return glm::mat4x4();
}

// ----------------------------------------------------------------------

glm::mat4x4 ofVkRenderer::getCurrentNormalMatrix() const
{
	if ( mDefaultContext )
		return glm::inverse(glm::transpose(mDefaultContext->getViewMatrix()));
	return glm::mat4x4();
}

// ----------------------------------------------------------------------

ofRectMode ofVkRenderer::getRectMode()
{
	return ofRectMode();
}


// ----------------------------------------------------------------------

void ofVkRenderer::setFillMode( ofFillFlag fill ){
	if ( mDefaultContext ){
		fill == OF_FILLED ? mDefaultContext->setPolyMode( VK_POLYGON_MODE_FILL ) : mDefaultContext->setPolyMode( VK_POLYGON_MODE_LINE );
	}
}

// ----------------------------------------------------------------------

ofFillFlag ofVkRenderer::getFillMode()
{
	return ofFillFlag();
}

// ----------------------------------------------------------------------


ofColor ofVkRenderer::getBackgroundColor()
{
	return ofColor();
}

// ----------------------------------------------------------------------

bool ofVkRenderer::getBackgroundAuto()
{
	return bBackgroundAuto;
}

// ----------------------------------------------------------------------

ofPath & ofVkRenderer::getPath()
{
	return mPath;
}

// ----------------------------------------------------------------------

ofStyle ofVkRenderer::getStyle() const
{
	return ofStyle();
}

// ----------------------------------------------------------------------

const of3dGraphics & ofVkRenderer::get3dGraphics() const
{
	return m3dGraphics;
}

// ----------------------------------------------------------------------

of3dGraphics & ofVkRenderer::get3dGraphics()
{
	return m3dGraphics;
}

// ----------------------------------------------------------------------

void ofVkRenderer::bind( const ofCamera & camera, const ofRectangle & viewport ){
	
	if ( mDefaultContext ){
		mDefaultContext->pushMatrix();
		mDefaultContext->setViewMatrix( camera.getModelViewMatrix() );

		// Clip space transform:

		// Vulkan has inverted y 
		// and half-width z.

		static const glm::mat4x4 clip( 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f );

		mDefaultContext->setProjectionMatrix( clip * camera.getProjectionMatrix( viewport ) );
	}
}

// ----------------------------------------------------------------------

void ofVkRenderer::unbind( const ofCamera& camera ){
	if ( mDefaultContext )
		mDefaultContext->popMatrix();
}


