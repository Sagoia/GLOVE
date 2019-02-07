#include "uniformBufferObject.h"

UniformBufferObject::UniformBufferObject(const vulkanAPI::vkContext_t *vkContext, const VkBufferUsageFlags vkBufferUsageFlags, const VkSharingMode vkSharingMode, const VkFlags vkFlags)
: mVkContext(vkContext), mUsage(GL_STATIC_DRAW), mAllocated(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mBuffer = new vulkanAPI::Buffer(vkContext, vkBufferUsageFlags, vkSharingMode);
    mMemory = new vulkanAPI::UniformMemory(vkContext, vkFlags);
}

UniformBufferObject::~UniformBufferObject()
{
    FUN_ENTRY(GL_LOG_TRACE);

    delete mBuffer;
    delete mMemory;
}

void
UniformBufferObject::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mBuffer->Release();
    mMemory->Release();
    mAllocated = false;
}

bool
UniformBufferObject::Allocate(size_t size, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mBuffer->SetSize(size);

    mAllocated = mBuffer->Create()                                            &&
                 mMemory->GetBufferMemoryRequirements(mBuffer->GetVkBuffer()) &&
                 mMemory->Create()                                            &&
                 UpdateData(size, 0, data)                                    &&
                 mMemory->BindBufferMemory(mBuffer->GetVkBuffer());

    if(mAllocated) {
        AllocateVkDescriptorBufferInfo();
    }
    return mAllocated;
}

bool
UniformBufferObject::UpdateData(size_t size, size_t offset, const void *data)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint8_t *pData = nullptr;
    if (mMemory->Lock((void **)&pData)) {
        if (data) {
            memcpy(pData + offset, data, size);
        } else {
            memset(pData + offset, 0, size);
        }
        mMemory->Unlock();
        return true;
    }

    return false;
}
