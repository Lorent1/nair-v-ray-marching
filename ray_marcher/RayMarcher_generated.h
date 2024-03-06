#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <array>

#include "vk_pipeline.h"
#include "vk_buffers.h"
#include "vk_utils.h"
#include "vk_copy.h"
#include "vk_context.h"


#include "RayMarcher.h"

#include "include/RayMarcher_generated_ubo.h"
class RayMarcher_Generated : public RayMarcher
{
public:

  RayMarcher_Generated() 
  {
  }
  virtual void InitVulkanObjects(VkDevice a_device, VkPhysicalDevice a_physicalDevice, size_t a_maxThreadsCount);
  
  virtual void SetVulkanContext(vk_utils::VulkanContext a_ctx) { m_ctx = a_ctx; }
  virtual void SetVulkanInOutFor_RayMarch(
    VkBuffer out_colorBuffer,
    size_t   out_colorOffset,
    uint32_t dummyArgument = 0)
  {
    RayMarch_local.out_colorBuffer = out_colorBuffer;
    RayMarch_local.out_colorOffset = out_colorOffset;
    InitAllGeneratedDescriptorSets_RayMarch();
  }

  virtual ~RayMarcher_Generated();

  
  virtual void InitMemberBuffers();
  virtual void UpdateAll(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyEngine)
  {
    UpdatePlainMembers(a_pCopyEngine);
    UpdateVectorMembers(a_pCopyEngine);
    UpdateTextureMembers(a_pCopyEngine);
  }
  
  virtual void CommitDeviceData(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyHelper) // you have to define this virtual function in the original imput class
  {
    InitMemberBuffers();
    UpdateAll(a_pCopyHelper);
  }  
  void CommitDeviceData() override { CommitDeviceData(m_ctx.pCopyHelper); }  
  void GetExecutionTime(const char* a_funcName, float a_out[4]) override; 
  void UpdateMembersPlainData() override { UpdatePlainMembers(m_ctx.pCopyHelper); } 
  

  virtual void ReserveEmptyVectors();
  virtual void UpdatePlainMembers(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyEngine);
  virtual void UpdateVectorMembers(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyEngine);
  virtual void UpdateTextureMembers(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyEngine);
  virtual void ReadPlainMembers(std::shared_ptr<vk_utils::ICopyEngine> a_pCopyEngine);
  static VkPhysicalDeviceFeatures2 ListRequiredDeviceFeatures(std::vector<const char*>& deviceExtensions);
  
  virtual void RayMarchCmd(VkCommandBuffer a_commandBuffer, uint32_t* out_color, uint32_t width, uint32_t height);

  void RayMarch(uint32_t* out_color, uint32_t width, uint32_t height) override;

  inline vk_utils::ExecTime GetRayMarchExecutionTime() const { return m_exTimeRayMarch; }

  vk_utils::ExecTime m_exTimeRayMarch;

  virtual void copyKernelFloatCmd(uint32_t length);
  
  virtual void RayMarchCmd(uint32_t* out_color, uint32_t width, uint32_t height);
  
  struct MemLoc
  {
    VkDeviceMemory memObject = VK_NULL_HANDLE;
    size_t         memOffset = 0;
    size_t         allocId   = 0;
  };

  virtual MemLoc AllocAndBind(const std::vector<VkBuffer>& a_buffers); ///< replace this function to apply custom allocator
  virtual MemLoc AllocAndBind(const std::vector<VkImage>& a_image);    ///< replace this function to apply custom allocator
  virtual void   FreeAllAllocations(std::vector<MemLoc>& a_memLoc);    ///< replace this function to apply custom allocator

protected:

  VkPhysicalDevice           physicalDevice = VK_NULL_HANDLE;
  VkDevice                   device         = VK_NULL_HANDLE;
  vk_utils::VulkanContext    m_ctx          = {};
  VkCommandBuffer            m_currCmdBuffer   = VK_NULL_HANDLE;
  uint32_t                   m_currThreadFlags = 0;
  std::vector<MemLoc>        m_allMems;
  VkPhysicalDeviceProperties m_devProps;

  VkBufferMemoryBarrier BarrierForClearFlags(VkBuffer a_buffer);
  VkBufferMemoryBarrier BarrierForSingleBuffer(VkBuffer a_buffer);
  void BarriersForSeveralBuffers(VkBuffer* a_inBuffers, VkBufferMemoryBarrier* a_outBarriers, uint32_t a_buffersNum);

  virtual void InitHelpers();
  virtual void InitBuffers(size_t a_maxThreadsCount, bool a_tempBuffersOverlay = true);
  virtual void InitKernels(const char* a_filePath);
  virtual void AllocateAllDescriptorSets();

  virtual void InitAllGeneratedDescriptorSets_RayMarch();

  virtual void AssignBuffersToMemory(const std::vector<VkBuffer>& a_buffers, VkDeviceMemory a_mem);

  virtual void AllocMemoryForMemberBuffersAndImages(const std::vector<VkBuffer>& a_buffers, const std::vector<VkImage>& a_image);
  virtual std::string AlterShaderPath(const char* in_shaderPath) { return std::string("ray_marcher/") + std::string(in_shaderPath); }

  
  

  struct RayMarch_Data
  {
    VkBuffer out_colorBuffer = VK_NULL_HANDLE;
    size_t   out_colorOffset = 0;
    bool needToClearOutput = false;
  } RayMarch_local;



  struct MembersDataGPU
  {
  } m_vdata;
  
  
  size_t m_maxThreadCount = 0;
  VkBuffer m_classDataBuffer = VK_NULL_HANDLE;

  VkPipelineLayout      RayMarchLayout   = VK_NULL_HANDLE;
  VkPipeline            RayMarchPipeline = VK_NULL_HANDLE; 
  VkDescriptorSetLayout RayMarchDSLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout CreateRayMarchDSLayout();
  virtual void InitKernel_RayMarch(const char* a_filePath);


  virtual VkBufferUsageFlags GetAdditionalFlagsForUBO() const;
  virtual uint32_t           GetDefaultMaxTextures() const;

  VkPipelineLayout      copyKernelFloatLayout   = VK_NULL_HANDLE;
  VkPipeline            copyKernelFloatPipeline = VK_NULL_HANDLE;
  VkDescriptorSetLayout copyKernelFloatDSLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout CreatecopyKernelFloatDSLayout();

  VkDescriptorPool m_dsPool = VK_NULL_HANDLE;
  VkDescriptorSet  m_allGeneratedDS[1];

  RayMarcher_Generated_UBO_Data m_uboData;
  
  constexpr static uint32_t MEMCPY_BLOCK_SIZE = 256;
  constexpr static uint32_t REDUCTION_BLOCK_SIZE = 256;

  virtual void MakeComputePipelineAndLayout(const char* a_shaderPath, const char* a_mainName, const VkSpecializationInfo *a_specInfo, const VkDescriptorSetLayout a_dsLayout, 
                                            VkPipelineLayout* pPipelineLayout, VkPipeline* pPipeline);
  virtual void MakeComputePipelineOnly(const char* a_shaderPath, const char* a_mainName, const VkSpecializationInfo *a_specInfo, const VkDescriptorSetLayout a_dsLayout, VkPipelineLayout pipelineLayout, 
                                       VkPipeline* pPipeline);

  std::vector<VkPipelineLayout> m_allCreatedPipelineLayouts; ///<! remenber them here to delete later
  std::vector<VkPipeline>       m_allCreatedPipelines;       ///<! remenber them here to delete later
public:

  struct MegaKernelIsEnabled
  {
    bool dummy = 0;
  };

  static MegaKernelIsEnabled  m_megaKernelFlags;
  static MegaKernelIsEnabled& EnabledPipelines() { return m_megaKernelFlags; }

};


