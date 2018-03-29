#include "stdafx.h"
#include "RwVulkanEngine.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBufferMgr.h"
#include "VulkanIm2DPipeline.h"
#include "VulkanDevice.h"
CRwVulkanEngine::CRwVulkanEngine(shared_ptr<CDebug> d) :CIRwRenderEngine{ d }
{}

bool CRwVulkanEngine::BaseEventHandler(int State, int* a2, void* a3, int a4)
{
	return RwD3DSystem(State, a2, a3, a4);
}

bool CRwVulkanEngine::Open(HWND windowHandle)
{
	m_pRenderer = new CVulkanRenderer(windowHandle);
	return true;
}

bool CRwVulkanEngine::Close()
{
	
	delete m_pRenderer;
	return true;
}
std::vector<VkSampler> m_rastersamplers;
bool CRwVulkanEngine::Start()
{
	m_pRenderer->InitDevice();
	m_pIm2DPipeline = new CVulkanIm2DPipeline(m_pRenderer);
	return true;
}

bool CRwVulkanEngine::Stop()
{
	delete m_pIm2DPipeline;
	m_pRenderer->DeInitDevice();
	return true;
}

bool CRwVulkanEngine::Standards(int*fnPtrArray, int)
{
	bool(*pDefstd)(void *, void *, int) = &mDefStd;
	bool(*pRasterCreate)(void *, void *, int) = &mRasterCreate;
	bool(*pNativeTextureRead)(void *, void *, int) = &mNativeTextureRead;
	bool(*pRasterLock)(void *, void *, int) = &mRasterLock;
	bool(*pRasterUnlock)(void *, void *, int) = &mRasterUnlock;
	bool(*pClear)(void *, void *, int) = &mCamClear;
	bool(*pBeginUpdate)(void *, void *, int) = &mCamBU;
	bool(*pEndUpdate)(void *, void *, int) = &mCameraEndUpdate;
	bool(*pRasterShowRaster)(void *, void *, int) = &mRasterShowRaster;
	bool(*pRasterDestroy)(void *, void *, int) = &mRasterDestroy;

	fnPtrArray[0] = (int)(void*&)pDefstd;
	fnPtrArray[1] = (int)(void*&)pBeginUpdate;//0x7F8F20;//CameraBeginUpdate !!must do!!
	fnPtrArray[2] = 0x7FEE20;//RGBToPixel not used in menu
	fnPtrArray[3] = 0x7FF070;//PixelToRGB not used in menu
	fnPtrArray[4] = (int)(void*&)pRasterCreate;//0x4CCE60;//RasterCreate !!must do!!
	fnPtrArray[5] = (int)(void*&)pRasterDestroy;//0x4CBB00;//RasterDestroy !!must do!!
	fnPtrArray[6] = 0x7FF270;//ImageGetFromRaster not used in menu
	fnPtrArray[7] = 0x8001E0;//RasterSetFromImage not used in menu
	fnPtrArray[8] = 0x4CBD40;//TextureSetRaster not used in menu
	fnPtrArray[9] = 0x7FFF00;//ImageFindRasterFormat not used in menu
	fnPtrArray[10] = (int)(void*&)pEndUpdate;//0x7F98D0;//CameraEndUpdate !!must do!!
	fnPtrArray[11] = 0x4CB524;//SetRasterContext not used in menu
	fnPtrArray[12] = 0x4CBD50;//RasterSubRaster not used in menu
	fnPtrArray[13] = 0x4CB4C0;//RasterClearRect not used in menu
	fnPtrArray[14] = 0x4CB4E0;//RasterClear not used in menu
	fnPtrArray[15] = (int)(void*&)pRasterLock;//0x4C9F90;//RasterLock !!must do!!
	fnPtrArray[16] = (int)(void*&)pRasterUnlock;//0x4CA290;//RasterUnlock !!must do!!
	fnPtrArray[17] = 0x4CAE40;//RasterRender not used in menu
	fnPtrArray[18] = 0x4CAE80;//RasterRenderScaled not used in menu
	fnPtrArray[19] = 0x4CAE60;//RasterRenderFast not used in menu
	fnPtrArray[20] = (int)(void*&)pRasterShowRaster;//0x7F99B0;//RasterShowRaster !!must do!!
	fnPtrArray[21] = (int)(void*&)pClear;//0x4CB4E0;//CameraClear !!must do!!
	fnPtrArray[22] = (int)(void*&)pDefstd;
	fnPtrArray[23] = 0x4CA4E0;//RasterLockPalette not used in menu
	fnPtrArray[24] = 0x4CA540;//RasterUnlockPalette not used in menu
	fnPtrArray[25] = 0x4CD360;//NativeTextureGetSize not used in menu
	fnPtrArray[26] = (int)(void*&)pNativeTextureRead;//0x4CD820;//NativeTextureRead !!must do!!
	fnPtrArray[27] = 0x4CD4D0;//NativeTextureWrite not used in menu
	fnPtrArray[28] = 0x4CBCB0;//RasterGetMipLevels not used in menu
	return true;
}

bool CRwVulkanEngine::GetNumSubSystems(int& n)
{
	n = m_pRenderer->getGPUlist().size();
	return true;
}

bool CRwVulkanEngine::GetCurrentSubSystem(int& n)
{
	n = m_pRenderer->GPU_ID();
	return true;
}

bool CRwVulkanEngine::SetSubSystem(int n)
{
	m_pRenderer->GPU_ID(n);
	return true;
}

bool CRwVulkanEngine::GetSubSystemInfo(RwSubSystemInfo&info, int n)
{
	strncpy_s(info.name, m_pRenderer->getGPUProperties(n).deviceName, 80);
	return true;
}

bool CRwVulkanEngine::GetNumModes(int& n)
{
	n = 1;
	return true;
}

bool CRwVulkanEngine::GetModeInfo(RwVideoMode& mode, int n)
{
	UNREFERENCED_PARAMETER(n);
	mode.width = 640;
	mode.height = 480;
	mode.refRate = 0;
	mode.depth = 32;
	mode.flags = rwVIDEOMODEEXCLUSIVE;
	mode.format = rwRASTERFORMATDEFAULT;
	return true;
}

bool CRwVulkanEngine::GetMode(int&n)
{
	n = 0;
	return true;
}

bool CRwVulkanEngine::UseMode(int)
{
	return true;
}

bool CRwVulkanEngine::Focus(bool)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::GetTexMemSize(int&)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::GetMaxTextureSize(int&)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int CRwVulkanEngine::GetMaxMultiSamplingLevels()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void CRwVulkanEngine::SetMultiSamplingLevels(int)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::RenderStateSet(RwRenderState rs, UINT val)
{
	if (!m_insideScene)
		return true;
	if (rs == rwRENDERSTATETEXTURERASTER) {
		int bUseTexture=0;
		if (val != NULL)
		{
			RwVulkanRaster* vkRaster = GetVKRaster(val);

			VkDescriptorImageInfo descriptorImageInfo = {};
			descriptorImageInfo.sampler = vkRaster->texture->vkSampler;
			descriptorImageInfo.imageView = vkRaster->texture->vkImgView;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

			VkWriteDescriptorSet writeDescriptor = {};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.dstSet = m_pIm2DPipeline->getDescriptorSet();
			writeDescriptor.dstBinding = 0;
			writeDescriptor.dstArrayElement = 0;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptor.pImageInfo = &descriptorImageInfo;
			writeDescriptor.pBufferInfo = NULL;
			writeDescriptor.pTexelBufferView = NULL;

			vkUpdateDescriptorSets(m_pRenderer->getDevice()->getDevice(), 1, &writeDescriptor, 0, NULL);
			bUseTexture = 1;
		}
		else {
			bUseTexture = 0;
		}
		vkCmdPushConstants(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), m_pIm2DPipeline->getFanPipeLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(bUseTexture), &bUseTexture);
		vkCmdPushConstants(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), m_pIm2DPipeline->getListPipeLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(bUseTexture), &bUseTexture);
	}
	return true;
	//throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::RenderStateGet(RwRenderState, UINT&)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices)
{
	if (!m_insideScene)
		return true;
	m_pIm2DPipeline->Draw(primType,vertices, numVertices);
	return true;
}

bool CRwVulkanEngine::Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
{
	if (!m_insideScene)
		return true;
	m_pIm2DPipeline->DrawIndexed(primType,vertices, numVertices, indices, numIndices);
	return true;
}

bool CRwVulkanEngine::RasterCreate(RwRaster *raster, UINT flags)
{
	RwVulkanRaster* vkRaster = GetVKRaster(raster);
	raster->cpPixels = 0;
	raster->palette = 0;
	raster->cType = flags & rwRASTERTYPEMASK;
	raster->cFlags = flags & 0xF8;
	vkRaster->texture = 0;
	vkRaster->palette = 0;
	vkRaster->alpha = 0;
	vkRaster->textureFlags = 0;
	vkRaster->cubeTextureFlags = 0;
	vkRaster->lockFlags = 0;
	vkRaster->lockedSurface = 0;
	vkRaster->format = VK_FORMAT_UNDEFINED;
	vkRaster->texture = nullptr;
	int rasterPixelFmt = flags & rwRASTERFORMATPIXELFORMATMASK;
	raster->cFormat = rasterPixelFmt >> 8;
	if (raster->cType == rwRASTERTYPETEXTURE)
	{
		if (flags&rwRASTERDONTALLOCATE)
		{
			if (rasterPixelFmt ==rwRASTERFORMAT1555)
			{
				vkRaster->format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			}
			else if(rasterPixelFmt == rwRASTERFORMAT565)
			{
				vkRaster->format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
			}
			else if (rasterPixelFmt == rwRASTERFORMAT4444)
			{
				vkRaster->format = VK_FORMAT_BC2_UNORM_BLOCK;
			}
		}else{
			switch (rasterPixelFmt)
			{
			case rwRASTERFORMAT1555:
				vkRaster->format = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
				break;
			case rwRASTERFORMAT4444:
				vkRaster->format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				break;
			case rwRASTERFORMAT565:
				vkRaster->format = VK_FORMAT_R5G6B5_UNORM_PACK16;
				break;
			case rwRASTERFORMAT8888:
				vkRaster->format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case rwRASTERFORMAT888:
				vkRaster->format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}
		}
		if (vkRaster->format == VK_FORMAT_UNDEFINED)
			return false;
		vkRaster->texture = new RwVulkanRasterData;
		VkImageCreateInfo textureCreateInfo = {};
		textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		textureCreateInfo.format = vkRaster->format;
		textureCreateInfo.extent = { (uint32_t)raster->width, (uint32_t)raster->height, 1 };
		textureCreateInfo.mipLevels = 1;
		textureCreateInfo.arrayLayers = 1;
		textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		textureCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		
		vkCreateImage(m_pRenderer->getDevice()->getDevice(), &textureCreateInfo, NULL, &vkRaster->texture->vkImg);

		{
			VkMemoryRequirements textureMemoryRequirements = {};
			vkGetImageMemoryRequirements(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->vkImg, &textureMemoryRequirements);

			VkPhysicalDeviceMemoryProperties memory_properties;
			vkGetPhysicalDeviceMemoryProperties(m_pRenderer->getGPUlist()[m_pRenderer->GPU_ID()], &memory_properties);

			VkMemoryAllocateInfo textureImageAllocateInfo = {};
			textureImageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			textureImageAllocateInfo.allocationSize = textureMemoryRequirements.size;

			uint32_t textureMemoryTypeBits = textureMemoryRequirements.memoryTypeBits;
			VkMemoryPropertyFlags tDesiredMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			for (uint32_t i = 0; i < 32; ++i) {
				VkMemoryType memoryType = memory_properties.memoryTypes[i];
				if (textureMemoryTypeBits & 1) {
					if ((memoryType.propertyFlags & tDesiredMemoryFlags) == tDesiredMemoryFlags) {
						textureImageAllocateInfo.memoryTypeIndex = i;
						break;
					}
				}
				textureMemoryTypeBits = textureMemoryTypeBits >> 1;
			}
			vkRaster->texture->memory = {};
			vkAllocateMemory(m_pRenderer->getDevice()->getDevice(), &textureImageAllocateInfo, NULL, &vkRaster->texture->memory);
		}

		vkBindImageMemory(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->vkImg, vkRaster->texture->memory, 0);

		VkImageViewCreateInfo textureImageViewCreateInfo = {};
		textureImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		textureImageViewCreateInfo.image = vkRaster->texture->vkImg;
		textureImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		textureImageViewCreateInfo.format = vkRaster->format;
		textureImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A };
		textureImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		textureImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		textureImageViewCreateInfo.subresourceRange.levelCount = 1;
		textureImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		textureImageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(m_pRenderer->getDevice()->getDevice(), &textureImageViewCreateInfo, NULL, &vkRaster->texture->vkImgView);
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.minLod = 0;
		samplerCreateInfo.maxLod = 1;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		vkCreateSampler(m_pRenderer->getDevice()->getDevice(), &samplerCreateInfo, NULL, &vkRaster->texture->vkSampler);
	}
	return true;
}

bool CRwVulkanEngine::RasterDestroy(RwRaster *raster)
{
	if (raster->cType == rwRASTERTYPETEXTURE)
	{
		RwVulkanRaster* vkRaster = GetVKRaster(raster);
		if(vkRaster->texture->vkSampler)
			vkDestroySampler(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->vkSampler, nullptr);
		vkDestroyImageView(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->vkImgView, nullptr);
		vkFreeMemory(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->memory, nullptr);
		vkDestroyImage(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->vkImg, nullptr);
	}
	return true;
}

bool CRwVulkanEngine::AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params)
{
	throw std::logic_error("The method or operation is not implemented.");
}

RwBool CRwVulkanEngine::Im3DSubmitNode()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void CRwVulkanEngine::DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{
	throw std::logic_error("The method or operation is not implemented.");
}

RwBool CRwVulkanEngine::DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::NativeTextureRead(RwStream *stream, RwTexture** tex)
{
	TextureFormat textureInfo; RasterFormat rasterInfo;
	unsigned int lengthOut, versionOut; unsigned char savedFormat;
	RwRaster *raster; RwTexture *texture;

	if (!RwStreamFindChunk(stream, rwID_STRUCT, &lengthOut, &versionOut) || versionOut < 0x34000 || versionOut > rwLIBRARYVERSION36003 ||
		RwStreamRead(stream, &textureInfo, sizeof(TextureFormat)) != sizeof(TextureFormat) || textureInfo.platformId != rwID_PCD3D9 ||
		RwStreamRead(stream, &rasterInfo, sizeof(RasterFormat)) != sizeof(RasterFormat))
		return false;
	if (rasterInfo.compressed)
	{
		/*if (m_checkValidTextureFormat(rasterInfo.d3dFormat))
		{*/
		raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType | rwRASTERDONTALLOCATE);
		if (!raster)
			return false;
		/*}
		else
			return false;*/
		/*d3dRaster = GetD3D9Raster(raster);
		numLevels = (raster->cFormat & FORMATMIPMAP) ? rasterInfo.numLevels : 1;
		if (!rasterInfo.cubeTexture)
		{
			//if ((rasterInfo.rasterFormat & rwRASTERFORMATMIPMAP) && (rasterInfo.rasterFormat & rwRASTERFORMATAUTOMIPMAP) &&
			//	rwD3D9CheckAutoMipmapGenCubeTextureFormat(rasterInfo.d3dFormat))
			d3dRaster->textureFlags |= 1;
			if (pD3DDevice->CreateTexture(raster->width, raster->height, numLevels, D3DUSAGE_AUTOGENMIPMAP, rasterInfo.d3dFormat,
				D3DPOOL_MANAGED, &d3dRaster->texture, NULL) != D3D_OK)
			{
				RwRasterDestroy(raster);
				return false;
			}
		}
		d3dRaster->textureFlags |= 0x10;
		d3dRaster->format = rasterInfo.d3dFormat;*/
	}
	else
	{
		if (!rasterInfo.cubeTexture)
		{
			/*if ((rasterInfo.rasterFormat & rwRASTERFORMATMIPMAP) && (rasterInfo.rasterFormat & rwRASTERFORMATAUTOMIPMAP))
			{
			raster = RwD3D9RasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.d3dFormat, rasterInfo.rasterType |
			(rasterInfo.rasterFormat & (rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP)));
			if (!raster)
			return false;
			}*/
			//else
			//{
			raster = RwRasterCreate(rasterInfo.width, rasterInfo.height, rasterInfo.depth, rasterInfo.rasterFormat | rasterInfo.rasterType);
			if (!raster)
				return false;
			//d3dRaster = GetD3D9Raster(raster);
			//raster->d3dRaster.hasCompression = 0; // Зачем??
			//}
		}
		else
			throw std::logic_error("The method or operation is not implemented.");
	}
	raster->cFlags ^= rwRASTERDONTALLOCATE;
	//d3dRaster->alpha = rasterInfo.alpha;

	/*if (rasterInfo.autoMipMaps && !raster->d3dRaster.autoMipMapping || rasterInfo.rasterFormat != ((raster->cFormat) << 8) ||
	rasterInfo.d3dFormat != raster->d3dRaster.format || rasterInfo.width != raster->width || rasterInfo.height != raster->height)
	{
	RwRasterDestroy(raster);
	return false;
	}*/

	/*// Читаем палитру, если надо
	if (rasterInfo.rasterFormat & rwRASTERFORMATPAL4)
	{
	if (RwStreamRead(stream, RwRasterLockPalette(raster, rwRASTERLOCKWRITE), 128) != 128)
	{
	RwRasterUnlockPalette(raster);
	RwRasterDestroy(raster);
	return false;
	}
	}
	else if (rasterInfo.rasterFormat & rwRASTERFORMATPAL8)
	{
	if (RwStreamRead(stream, RwRasterLockPalette(raster, rwRASTERLOCKWRITE), 1024) != 1024)
	{
	RwRasterUnlockPalette(raster);
	RwRasterDestroy(raster);
	return false;
	}
	}
	RwRasterUnlockPalette(raster);*/

	savedFormat = raster->cFormat;
	//raster->cFormat ^= FORMATAUTOMIPMAP;
	
	for (int i = 0; i < 1; i++)
	{
		//d3dRaster->cubeTextureFlags = 0;
		for (RwUInt8 j = 0; j < 1; j++)
		{
			if (RwStreamRead(stream, &lengthOut, 4) == 4)
			{
				if (RwStreamRead(stream, RwRasterLock(raster, j, rwRASTERLOCKWRITE), lengthOut) == lengthOut)
				{
					RwRasterUnlock(raster);
					continue;
				}
			}
			RwRasterUnlock(raster);
			RwRasterDestroy(raster);
			return false;
		}
	}
	
	raster->cFormat = savedFormat;

	texture = RwTextureCreate(raster);
	if (!texture)
	{
		RwRasterDestroy(raster);
		return false;
	}
	RwTextureSetFilterModeMacro(texture, textureInfo.filterMode);
	RwTextureSetAddressingUMacro(texture, textureInfo.uAddressing);
	RwTextureSetAddressingVMacro(texture, textureInfo.vAddressing);
	RwTextureSetName(texture, textureInfo.name);
	RwTextureSetMaskName(texture, textureInfo.maskName);
	*tex = texture;
	return true;
	//throw std::logic_error("The method or operation is not implemented.");
}

bool CRwVulkanEngine::RasterLock(RwRaster *raster, UINT flags, void** data)
{
	UNREFERENCED_PARAMETER(flags);
	RwVulkanRaster* vkRaster = GetVKRaster(raster);
	if (raster->cpPixels!=nullptr||vkRaster->texture==nullptr)
		return false;
	vkMapMemory(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->memory, 0, VK_WHOLE_SIZE, 0, (void**)&raster->cpPixels);

	*data = (void*)raster->cpPixels;

	return true;
}

bool CRwVulkanEngine::RasterUnlock(RwRaster *raster)
{
	RwVulkanRaster* vkRaster = GetVKRaster(raster);
	if (raster->cpPixels==nullptr||vkRaster->texture == nullptr)
		return false;
	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = vkRaster->texture->memory;
	memoryRange.offset = 0;
	memoryRange.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(m_pRenderer->getDevice()->getDevice(), 1, &memoryRange);

	vkUnmapMemory(m_pRenderer->getDevice()->getDevice(), vkRaster->texture->memory);
	raster->cpPixels = nullptr;
	return true;
}

bool CRwVulkanEngine::CameraBeginUpdate(RwCamera *camera)
{
	g_pDebug->printMsg("begin update");
	if (m_insideScene)
		return true;
	dgGGlobals = camera;

	RwMatrixInvert(&RwD3D9D3D9ViewTransform, RwFrameGetLTM((RwFrame*)camera->object.object.parent));
	RwD3D9D3D9ViewTransform.right.x = -RwD3D9D3D9ViewTransform.right.x;
	RwD3D9D3D9ViewTransform.up.x = -RwD3D9D3D9ViewTransform.up.x;
	RwD3D9D3D9ViewTransform.at.x = -RwD3D9D3D9ViewTransform.at.x;
	RwD3D9D3D9ViewTransform.pos.x = -RwD3D9D3D9ViewTransform.pos.x;
	RwD3D9D3D9ViewTransform.flags = 0;
	RwD3D9D3D9ViewTransform.pad1 = 0;
	RwD3D9D3D9ViewTransform.pad2 = 0;
	RwD3D9D3D9ViewTransform.pad3 = 0x3F800000;
	//SetTransform(D3DTS_VIEW, &RwD3D9D3D9ViewTransform);

	RwD3D9D3D9ProjTransform.right.x = camera->recipViewWindow.x;
	RwD3D9D3D9ProjTransform.up.y = camera->recipViewWindow.y;
	RwD3D9D3D9ProjTransform.at.x = camera->viewOffset.x * camera->recipViewWindow.x;
	RwD3D9D3D9ProjTransform.at.y = camera->viewOffset.y * camera->recipViewWindow.y;
	RwD3D9D3D9ProjTransform.pos.x = -(camera->viewOffset.x * camera->recipViewWindow.x);
	RwD3D9D3D9ProjTransform.pos.y = -(camera->viewOffset.y * camera->recipViewWindow.y);
	if (camera->projectionType == rwPARALLEL)
	{
		RwD3D9D3D9ProjTransform.at.z = 1.0f / (camera->farPlane - camera->nearPlane);
		RwD3D9D3D9ProjTransform.pad2 = 0;
		RwD3D9D3D9ProjTransform.pad3 = 0x3F800000;
	}
	else
	{
		RwD3D9D3D9ProjTransform.at.z = camera->farPlane / (camera->farPlane - camera->nearPlane);
		RwD3D9D3D9ProjTransform.pad2 = 0x3F800000;
		RwD3D9D3D9ProjTransform.pad3 = 0;
	}
	RwD3D9D3D9ProjTransform.pos.z = -(RwD3D9D3D9ProjTransform.at.z * camera->nearPlane);
	//SetTransform(D3DTS_PROJECTION, &RwD3D9D3D9ProjTransform);
	RwD3D9ActiveViewProjTransform = 0;

	
	m_pRenderer->BeginScene();
	VkClearValue clear_value = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },                     // VkClearColorValue              color
	};
	VkRenderPassBeginInfo render_pass_begin_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,     // VkStructureType                sType
		nullptr,                                      // const void                    *pNext
		m_pRenderer->getRenderPass(),                            // VkRenderPass                   renderPass
		m_pRenderer->getCurrentFrameBuffer(),          // VkFramebuffer                  framebuffer
		{                                             // VkRect2D                       renderArea
			{                                           // VkOffset2D                     offset
				0,                                          // int32_t                        x
				0                                           // int32_t                        y
			},
			{                                           // VkExtent2D                     extent
				m_pRenderer->getWindowWidth(),                                        // int32_t                        width
				m_pRenderer->getWindowHeight(),                                        // int32_t                        height
			}
		},
		1,                                            // uint32_t                       clearValueCount
		&clear_value                                  // const VkClearValue            *pClearValues
	};

	vkCmdBeginRenderPass(m_pRenderer->getBufferMgr()->getRenderCommandBuffer(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	m_pIm2DPipeline->UpdateViewProj(RwD3D9D3D9ViewTransform, RwD3D9D3D9ProjTransform);
	m_insideScene = !m_insideScene;
	return true;
}

bool CRwVulkanEngine::CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags)
{
	UNREFERENCED_PARAMETER(camera);
	UNREFERENCED_PARAMETER(flags);
	VkClearColorValue clear_color = {
		{ color->red/255.0f, color->green / 255.0f, color->blue / 255.0f, color->alpha / 255.0f }
	};
	m_pRenderer->ClearScene(clear_color);
	return true;
}

bool CRwVulkanEngine::CameraEndUpdate(RwCamera *camera)
{
	g_pDebug->printMsg("end update");
	if (!m_insideScene)
		return true;
	UNREFERENCED_PARAMETER(camera);
	
	vkCmdEndRenderPass(m_pRenderer->getBufferMgr()->getRenderCommandBuffer());
	m_pRenderer->EndScene();
	m_pIm2DPipeline->EndScene();
	m_insideScene = !m_insideScene;
	return true;
}

bool CRwVulkanEngine::RasterShowRaster(RwRaster *raster, UINT flags)
{
	UNREFERENCED_PARAMETER(raster);
	UNREFERENCED_PARAMETER(flags);
	m_pRenderer->Present();
	return true;
}
