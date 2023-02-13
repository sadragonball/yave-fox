/*******************************
Copyright (c) 2016-2022 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/
#ifndef YAVE_UTILS_FORWARD_H
#define YAVE_UTILS_FORWARD_H

// Auto generated: forward definitions for most non template classes
// TODO: Nested classes will not be declared correctly

namespace yave {
class AABB;
class AABBUpdateSystem;
class Animation;
class AnimationChannel;
class AssetDependencies;
class AssetLoader;
class AssetLoaderSystem;
class AssetLoadingContext;
class AssetLoadingThreadPool;
class AssetStore;
class AtmosphereComponent;
class BufferBarrier;
class BufferBase;
class Camera;
class CmdBufferData;
class CmdBufferPool;
class CmdBufferRecorder;
class CmdBufferRegion;
class CmdQueue;
class CmdTimingRecorder;
class ComputeProgram;
class DebugParams;
class DebugUtils;
class DedicatedDeviceMemoryAllocator;
class Descriptor;
class DescriptorSet;
class DescriptorSetAllocator;
class DescriptorSetBase;
class DescriptorSetData;
class DescriptorSetLayout;
class DescriptorSetPool;
class DeviceMemory;
class DeviceMemoryAllocator;
class DeviceMemoryHeap;
class DeviceMemoryHeapBase;
class DeviceMemoryView;
class DeviceResources;
class DirectDraw;
class DirectDrawPrimitive;
class DirectionalLightComponent;
class EventHandler;
class FileSystemModel;
class FolderAssetStore;
class FolderFileSystemModel;
class FrameGraph;
class FrameGraphComputePassBuilder;
class FrameGraphDescriptorBinding;
class FrameGraphFrameResources;
class FrameGraphPass;
class FrameGraphPassBuilder;
class FrameGraphPassBuilderBase;
class FrameGraphRegion;
class FrameGraphResourceId;
class FrameGraphResourcePool;
class Framebuffer;
class Frustum;
class GenericAssetPtr;
class GraphicPipeline;
class IBLProbe;
class ImageBarrier;
class ImageBase;
class ImageData;
class ImageFormat;
class InlineDescriptor;
class Instance;
class KeyCombination;
class Layout;
class LifetimeManager;
class LoaderBase;
class LoadingJob;
class LocalFileSystemModel;
class LocalLightBase;
class Mapping;
class Material;
class MaterialCompiler;
class MaterialTemplate;
class MaterialTemplateData;
class MeshAllocator;
class MeshBufferData;
class MeshData;
class MeshDrawData;
class Octree;
class OctreeData;
class OctreeNode;
class OctreeSystem;
class PhysicalDevice;
class PointLightComponent;
class RenderPass;
class RenderPassRecorder;
class Renderable;
class ResourceFence;
class Sampler;
class SceneView;
class ScriptSystem;
class ScriptWorldComponent;
class SearchableFileSystemModel;
class ShaderModuleBase;
class ShaderProgram;
class SimpleMaterialData;
class Skeleton;
class SkeletonInstance;
class SkyLightComponent;
class SpecializationData;
class SpirVData;
class SpotLightComponent;
class StaticMesh;
class StaticMeshComponent;
class SubBufferBase;
class Swapchain;
class SwapchainImage;
class ThreadLocalDevice;
class ThreadLocalLifetimeManager;
class TimelineFence;
class TimestampQuery;
class TimestampQueryPool;
class TimestampQueryPoolData;
class TransformableComponent;
class TransientBuffer;
class WaitToken;
class Window;
struct AABBTypeInfo;
struct AssetData;
struct AssetDesc;
struct AssetId;
struct AtmospherePass;
struct Attachment;
struct AttachmentData;
struct AttribBuffers;
struct AttribDescriptor;
struct Attribute;
struct BloomPass;
struct BloomSettings;
struct BlurPass;
struct BlurSettings;
struct Bone;
struct BoneKey;
struct BoneTransform;
struct BufferCreateInfo;
struct BufferData;
struct Contants;
struct DefaultRenderer;
struct DeviceProperties;
struct DirectVertex;
struct DownsamplePass;
struct EmptyResource;
struct EnclosingSphere;
struct EntryInfo;
struct Event;
struct ExposurePass;
struct FrameGraphBufferId;
struct FrameGraphImageId;
struct FrameGraphMutableBufferId;
struct FrameGraphMutableImageId;
struct FrameGraphMutableResourceId;
struct FrameSyncObjects;
struct FrameToken;
struct FreeBlock;
struct FullVertex;
struct GBufferPass;
struct ImageCopyInfo;
struct ImageCreateInfo;
struct ImageSampler;
struct InlineBlock;
struct InlineStorage;
struct KeyEqual;
struct KeyHash;
struct LayoutPools;
struct LightingPass;
struct LightingSettings;
struct LoadableComponentTypeInfo;
struct MeshDrawCommand;
struct Mip;
struct Monitor;
struct OneShotScript;
struct PackedVertex;
struct Plane;
struct Region;
struct RendererSettings;
struct ResourceCreateInfo;
struct ResourceUsageInfo;
struct RunOnce;
struct RunOnceResult;
struct SSAOPass;
struct SSAOSettings;
struct SceneData;
struct SceneRenderSubPass;
struct Script;
struct ShadowMapPass;
struct ShadowMapSettings;
struct SkeletonData;
struct SkinWeights;
struct SkinnedVertex;
struct SubMesh;
struct ToneMappingPass;
struct ToneMappingSettings;
struct Viewport;
}


namespace yave::detail {
class AssetPtrDataBase;
struct VkNull;
struct VkStructInitializer;
}


namespace yave::ecs {
class Archetype;
class ComponentBoxBase;
class ComponentContainerBase;
class EntityId;
class EntityIdPool;
class EntityPrefab;
class EntityScene;
class EntityWorld;
class SparseIdSet;
class SparseIdSetBase;
class System;
class WorldComponentContainerBase;
struct ComponentRuntimeInfo;
struct ComponentsReturnPolicy;
struct IdComponents;
struct IdComponentsReturnPolicy;
struct QueryUtils;
struct SetMatch;
}


namespace yave::uniform {
struct Camera;
struct DirectionalLight;
struct ExposureParams;
struct PointLight;
struct RayleighSky;
struct SH;
struct ShadowMapParams;
struct SpotLight;
struct VPL;
}


#endif // YAVE_UTILS_FORWARD_H