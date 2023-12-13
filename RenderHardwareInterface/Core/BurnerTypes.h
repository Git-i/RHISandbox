#pragma once
struct RenderingAttachmentDesc_Burner
{
    int   sType;
    void* pNext;
    void* imageView;
    int   imageLayout;
    int   resolveMode;
    void* resolveImageView;
    int   resolveImageLayout;
    int   loadOp;
    int   storeOp;
    float clearValue[4];
};
struct TextureMemoryBarrier_Burner
{
    int  sType;
    void* pNext;
    int   srcAccessMask;
    int   dstAccessMask;
    int   oldLayout;
    int   newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    void*                   image;
    float   subresourceRange[5];
};
struct BufferMemoryBarrier_Burner
{
    void* lol;
};

