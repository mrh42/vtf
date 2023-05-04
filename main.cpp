
#include <vulkan/vulkan.h>

#include <vector>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <cmath>
#include <sys/time.h>


const int np = 8192*64;  // total threads to start, check shader, need atleast 4620 to initialize.
const int NK = 300; // must match shader
struct Stuff {
	uint64_t    P;
	uint64_t    K;
	uint64_t    Found;
	uint64_t    Err;
	uint        Init;
	uint        sp[100];
	uint        k4620[4620][4620];
};

// small primes for sieve()
const uint csp[561] = {7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = false;
#endif

//
// mrh: this is all lifted right from: https://github.com/Erkaman/vulkan_minimal_compute


// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

/*
The application launches a compute shader that renders the mandelbrot set,
by rendering it into a storage buffer.
The storage buffer is then read from the GPU, and saved as .png. 
*/
class ComputeApplication {
private:
    
    /*
    In order to use Vulkan, you must create an instance. 
    */
    VkInstance instance;

    VkDebugReportCallbackEXT debugReportCallback;
    /*
    The physical device is some device on the system that supports usage of Vulkan.
    Often, it is simply a graphics card that supports Vulkan. 
    */
    VkPhysicalDevice physicalDevice;
    /*
    Then we have the logical device VkDevice, which basically allows 
    us to interact with the physical device. 
    */
    VkDevice device;

    /*
    The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.

    We will be creating a simple compute pipeline in this application. 
    */
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShaderModule;

    /*
    The command buffer is used to record commands, that will be submitted to a queue.

    To allocate such command buffers, we use a command pool.
    */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    /*

    Descriptors represent resources in shaders. They allow us to use things like
    uniform buffers, storage buffers and images in GLSL. 

    A single descriptor represents a single resource, and several descriptors are organized
    into descriptor sets, which are basically just collections of descriptors.
    */
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    /*
    The mandelbrot set will be rendered to this buffer.

    The memory that backs the buffer is bufferMemory. 
    */
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
        
    uint32_t bufferSize; // size of `buffer` in bytes.

    std::vector<const char *> enabledLayers;

    /*
    In order to execute commands on a device(GPU), the commands must be submitted
    to a queue. The commands are stored in a command buffer, and this command buffer
    is given to the queue. 

    There will be different kinds of queues on the device. Not all queues support
    graphics operations, for instance. For this application, we at least want a queue
    that supports compute operations. 
    */
    VkQueue queue; // a queue supporting compute operations.

    /*
    Groups of queues that have the same capabilities(for instance, they all supports graphics and computer operations),
    are grouped into queue families. 
    
    When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
    This variable keeps track of the index of that queue in its family. 
    */
    uint32_t queueFamilyIndex;

	uint32_t mrhDone;
public:
    void run() {
	bufferSize = sizeof(struct Stuff);

        // Initialize vulkan:
        createInstance();
        findPhysicalDevice();
        createDevice();
        createBuffer();
        createDescriptorSetLayout();
        createDescriptorSet();
        createComputePipeline();

	createCommandBuffer();

	mrhInit();


	for (int i = 0; i < 937480369; i++) {
		struct timeval t1, t2;
		gettimeofday(&t1, NULL);

		runCommandBuffer();

		gettimeofday(&t2, NULL);
		double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
		fprintf(stdout, "gpu: %d in %f ms\n", i, elapsedTime);
		mrhNext();
    
		//if (mrhDone) { break; }
	}

        // Clean up all vulkan resources.
        cleanup();
    }

	void mrhNext() {
		void* mappedMemory = NULL;
		vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &mappedMemory);
		struct Stuff *p = (struct Stuff *) mappedMemory;
		printf("K: %ld P: %ld Err %ld\n", p->K, p->P, p->Err);
		if (p->Found) {
			printf("M%ld has factor with K=%ld E: %ld\n", p->P, p->Found, p->Err);
			p->Found = 0;
			p->Err = 0;
			mrhDone = 1;
		}
		p->Err = 0;
		p->K += NK * np;
		vkUnmapMemory(device, bufferMemory);
	}
	void mrhInit() {
		void* mappedMemory = NULL;
		vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &mappedMemory);
		struct Stuff *p = (struct Stuff *) mappedMemory;
		mrhDone = 0;
		p->P = 999983;
		p->K = 1;
		p->P = 133330459;
		p->K = 43473036040;
		p->P = 133331333;
		p->K = 606233611363280;
		//p->P = 262359187;
		//p->K = 36929909828640;
		p->Found = 0;
		p->Err = 0;
		p->Init = 0;

		// for (int k = 0; k < 4620; k++) {
		// 	for (uint n = 0; n < 4620; n++) {
		// 		uint q = 2 * n * k + 1;
		// 		if (((q&7) == 3) || ((q&7) == 5) || (q%3 == 0) || (q%5 == 0) || (q%7 == 0) || (q%11 == 0)) {
		// 			p->k4620[n][k] = 0;
		// 		} else {
		// 			p->k4620[n][k] = 1;
		// 		}
		// 	}
		// }
		
		for (int i = 0; i < 100; i++) {
			p->sp[i] = csp[i];
		}
		runCommandBuffer();  // initialize some shader stuff
		p->Init = 1;         // now we are done.
		vkUnmapMemory(device, bufferMemory);
	}


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData) {

        printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

        return VK_FALSE;
     }

    void createInstance() {
        std::vector<const char *> enabledExtensions;

        /*
        By enabling validation layers, Vulkan will emit warnings if the API
        is used incorrectly. We shall enable the layer VK_LAYER_LUNARG_standard_validation,
        which is basically a collection of several useful validation layers.
        */
        if (enableValidationLayers) {
            /*
            We get all supported layers with vkEnumerateInstanceLayerProperties.
            */
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, NULL);

            std::vector<VkLayerProperties> layerProperties(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

            /*
            And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
            */
            bool foundLayer = false;
            for (VkLayerProperties prop : layerProperties) {
                
                if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
                    foundLayer = true;
                    break;
                }

            }
            
            if (!foundLayer) {
                throw std::runtime_error("Layer VK_LAYER_LUNARG_standard_validation not supported\n");
            }
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation"); // Alright, we can use this layer.

            /*
            We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            in order to be able to print the warnings emitted by the validation layer.

            So again, we just check if the extension is among the supported extensions.
            */
            
            uint32_t extensionCount;
            
            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
            std::vector<VkExtensionProperties> extensionProperties(extensionCount);
            vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

            bool foundExtension = false;
            for (VkExtensionProperties prop : extensionProperties) {
                if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
                    foundExtension = true;
                    break;
                }

            }

            if (!foundExtension) {
                throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
            }
            enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }		

        /*
        Next, we actually create the instance.
        
        */
        
        /*
        Contains application info. This is actually not that important.
        The only real important field is apiVersion.
        */
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Hello world app";
        applicationInfo.applicationVersion = 0;
        applicationInfo.pEngineName = "awesomeengine";
        applicationInfo.engineVersion = 0;
        applicationInfo.apiVersion = VK_API_VERSION_1_0;;
        
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &applicationInfo;
        
        // Give our desired layers and extensions to vulkan.
        createInfo.enabledLayerCount = enabledLayers.size();
        createInfo.ppEnabledLayerNames = enabledLayers.data();
        createInfo.enabledExtensionCount = enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    
        /*
        Actually create the instance.
        Having created the instance, we can actually start using vulkan.
        */
        VK_CHECK_RESULT(vkCreateInstance(
            &createInfo,
            NULL,
            &instance));

        /*
        Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
        layer are actually printed.
        */
        if (enableValidationLayers) {
            VkDebugReportCallbackCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            createInfo.pfnCallback = &debugReportCallbackFn;

            // We have to explicitly load this function.
            auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
            if (vkCreateDebugReportCallbackEXT == nullptr) {
                throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
            }

            // Create and register callback.
            VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &createInfo, NULL, &debugReportCallback));
        }
    
    }

    void findPhysicalDevice() {
        /*
        In this function, we find a physical device that can be used with Vulkan.
        */

        /*
        So, first we will list all physical devices on the system with vkEnumeratePhysicalDevices .
        */
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
        if (deviceCount == 0) {
            throw std::runtime_error("could not find a device with vulkan support");
        }

	fprintf(stderr, "mrh: %d devices found\n", deviceCount);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        /*
        Next, we choose a device that can be used for our purposes. 

        With VkPhysicalDeviceFeatures(), we can retrieve a fine-grained list of physical features supported by the device.
        However, in this demo, we are simply launching a simple compute shader, and there are no 
        special physical features demanded for this task.

        With VkPhysicalDeviceProperties(), we can obtain a list of physical device properties. Most importantly,
        we obtain a list of physical device limitations. For this application, we launch a compute shader,
        and the maximum size of the workgroups and total number of compute shader invocations is limited by the physical device,
        and we should ensure that the limitations named maxComputeWorkGroupCount, maxComputeWorkGroupInvocations and 
        maxComputeWorkGroupSize are not exceeded by our application.  Moreover, we are using a storage buffer in the compute shader,
        and we should ensure that it is not larger than the device can handle, by checking the limitation maxStorageBufferRange. 

        However, in our application, the workgroup size and total number of shader invocations is relatively small, and the storage buffer is
        not that large, and thus a vast majority of devices will be able to handle it. This can be verified by looking at some devices at_
        http://vulkan.gpuinfo.org/

        Therefore, to keep things simple and clean, we will not perform any such checks here, and just pick the first physical
        device in the list. But in a real and serious application, those limitations should certainly be taken into account.

        */
	// mrh
	int n = 0;
        for (VkPhysicalDevice device : devices) {
            if (n == 0) { // As above stated, we do no feature checks, so just accept.
                physicalDevice = device;
                break;
            }
	    n++;
        }
    }

    // Returns the index of a queue family that supports compute operations. 
    uint32_t getComputeQueueFamilyIndex() {
        uint32_t queueFamilyCount;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

        // Retrieve all queue families.
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        // Now find a family that supports compute.
        uint32_t i = 0;
        for (; i < queueFamilies.size(); ++i) {
            VkQueueFamilyProperties props = queueFamilies[i];

            if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                // found a queue with compute. We're done!
                break;
            }
        }

        if (i == queueFamilies.size()) {
            throw std::runtime_error("could not find a queue family that supports operations");
        }

        return i;
    }

    void createDevice() {
        /*
        We create the logical device in this function.
        */

        /*
        When creating the device, we also specify what queues it has.
        */
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
        float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant. 
        queueCreateInfo.pQueuePriorities = &queuePriorities;

        /*
        Now we create the logical device. The logical device allows us to interact with the physical
        device.
        */
        VkDeviceCreateInfo deviceCreateInfo = {};

        // Specify any desired device features here. We do not need any for this application, though.
        VkPhysicalDeviceFeatures deviceFeatures = {};

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledLayerCount = enabledLayers.size();  // need to specify validation layers here as well.
        deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)); // create logical device.

        // Get a handle to the only member of the queue family.
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    }

    // find memory type with desired properties.
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memoryProperties;

        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        /*
        How does this search work?
        See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description. 
        */
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            if ((memoryTypeBits & (1 << i)) &&
                ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;
        }
        return -1;
    }

    void createBuffer() {
        /*
        We will now create a buffer. We will render the mandelbrot set into this buffer
        in a computer shade later. 
        */
        
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = bufferSize; // buffer size in bytes. 
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time. 

        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer)); // create buffer.

        /*
        But the buffer doesn't allocate memory for itself, so we must do that manually.
        */
    
        /*
        First, we find the memory requirements for the buffer.
        */
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
        
        /*
        Now use obtained memory requirements info to allocate the memory for the buffer.
        */
        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
        /*
        There are several types of memory that can be allocated, and we must choose a memory type that:

        1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits). 
        2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
           with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT. 
        Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily 
        visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
        this flag.
        */
        allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        //allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &bufferMemory)); // allocate memory on device.
        
        // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
        VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));
    }

    void createDescriptorSetLayout() {
        /*
        Here we specify a descriptor set layout. This allows us to bind our descriptors to 
        resources in the shader. 

        */

        /*
        Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
        0. This binds to 

          layout(std140, binding = 0) buffer buf

        in the compute shader.
        */
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.binding = 0; // binding = 0
        descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout. 
        descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding; 

        // Create the descriptor set layout. 
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
    }

    void createDescriptorSet() {
        /*
        So we will allocate a descriptor set here.
        But we need to first create a descriptor pool to do that. 
        */

        /*
        Our descriptor pool can only allocate a single storage buffer.
        */
        VkDescriptorPoolSize descriptorPoolSize = {};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorPoolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

        // create descriptor pool.
        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

        /*
        With the pool allocated, we can now allocate the descriptor set. 
        */
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
        descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
        descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

        // allocate descriptor set.
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        /*
        Next, we need to connect our actual storage buffer with the descrptor. 
        We use vkUpdateDescriptorSets() to update the descriptor set.
        */

        // Specify the buffer to bind to the descriptor.
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = buffer;
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = bufferSize;

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
        writeDescriptorSet.dstBinding = 0; // write to the first, and only binding.
        writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        // perform the update of the descriptor set.
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
    }

    // Read file into array of bytes, and cast to uint32_t*, then return.
    // The data has been padded, so that it fits into an array uint32_t.
    uint32_t* readFile(uint32_t& length, const char* filename) {

        FILE* fp = fopen(filename, "rb");
        if (fp == NULL) {
            printf("Could not find or open file: %s\n", filename);
        }

        // get file size.
        fseek(fp, 0, SEEK_END);
        long filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        long filesizepadded = long(ceil(filesize / 4.0)) * 4;

        // read file contents.
        char *str = new char[filesizepadded];
        size_t n = fread(str, filesize, sizeof(char), fp);
	fprintf(stderr, "read %ld bytes\n", n * filesize);
        fclose(fp);

        // data padding. 
        for (int i = filesize; i < filesizepadded; i++) {
            str[i] = 0;
        }

        length = filesizepadded;
        return (uint32_t *)str;
    }

    void createComputePipeline() {
        /*
        We create a compute pipeline here. 
        */

        /*
        Create a shader module. A shader module basically just encapsulates some shader code.
        */
        uint32_t filelength;
        // the code in comp.spv was created by running the command:
        // glslangValidator.exe -V shader.comp
        uint32_t* code = readFile(filelength, "comp.spv");
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pCode = code;
        createInfo.codeSize = filelength;
        
        VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));
        delete[] code;

        /*
        Now let us actually create the compute pipeline.
        A compute pipeline is very simple compared to a graphics pipeline.
        It only consists of a single stage with a compute shader. 

        So first we specify the compute shader stage, and it's entry point(main).
        */
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = computeShaderModule;
        shaderStageCreateInfo.pName = "main";

        /*
        The pipeline layout allows the pipeline to access descriptor sets. 
        So we just specify the descriptor set layout we created earlier.
        */
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout; 
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage = shaderStageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;

        /*
        Now, we finally create the compute pipeline. 
        */
	fprintf(stderr, "mrh about to create\n");
        VK_CHECK_RESULT(vkCreateComputePipelines(
            device, VK_NULL_HANDLE,
            1, &pipelineCreateInfo,
            NULL, &pipeline));
    }

    void createCommandBuffer() {
        /*
        We are getting closer to the end. In order to send commands to the device(GPU),
        we must first record commands into a command buffer.
        To allocate a command buffer, we must first create a command pool. So let us do that.
        */
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = 0;
        // the queue family of this command pool. All command buffers allocated from this command pool,
        // must be submitted to queues of this family ONLY. 
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));

        /*
        Now allocate a command buffer from the command pool. 
        */
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from. 
        // if the command buffer is primary, it can be directly submitted to queues. 
        // A secondary buffer has to be called from some primary command buffer, and cannot be directly 
        // submitted to a queue. To keep things simple, we use a primary command buffer. 
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer. 
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.

        /*
        Now we shall start recording commands into the newly allocated command buffer. 
        */
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        /*
        We need to bind a pipeline, AND a descriptor set before we dispatch.

        The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
        */
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

        /*
        Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
        The number of workgroups is specified in the arguments.
        If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
        */
	// mrh: match shader...
        vkCmdDispatch(commandBuffer, np/64, 1, 1);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
    }

    void runCommandBuffer() {
        /*
        Now we shall finally submit the recorded command buffer to a queue.
        */

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1; // submit a single command buffer
        submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

        /*
          We create a fence.
        */
        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

        /*
        We submit the command buffer on the queue, at the same time giving a fence.
        */
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
       /*
        The command will not have finished executing until the fence is signalled.
        So we wait here.
        We will directly after this read our buffer from the GPU,
        and we will not be sure that the command has finished executing unless we wait for the fence.
        Hence, we use a fence here.
        */

        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 50000000000L));

        vkDestroyFence(device, fence, NULL);
    }

    void cleanup() {
        /*
        Clean up all Vulkan Resources. 
        */

        if (enableValidationLayers) {
            // destroy callback.
            auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
            if (func == nullptr) {
                throw std::runtime_error("Could not load vkDestroyDebugReportCallbackEXT");
            }
            func(instance, debugReportCallback, NULL);
        }

        vkFreeMemory(device, bufferMemory, NULL);
        vkDestroyBuffer(device, buffer, NULL);	
        vkDestroyShaderModule(device, computeShaderModule, NULL);
        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyCommandPool(device, commandPool, NULL);	
        vkDestroyDevice(device, NULL);
        vkDestroyInstance(instance, NULL);		
    }
};

int main() {
    ComputeApplication app;

    try {
        app.run();
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
