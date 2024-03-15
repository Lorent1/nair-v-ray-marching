

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>  // for shared pointers
#include <iomanip> // for std::fixed/std::setprecision
#include <sstream>

#include "files/file_input.h"

#include "ray_marcher/RayMarcher.h"
#include "Image2d.h"

#ifdef USE_VULKAN
#include "vk_context.h"
std::shared_ptr<RayMarcher> CreateRayMarcher_Generated(vk_utils::VulkanContext a_ctx, size_t a_maxThreadsGenerated);
#endif


int main(int argc, const char** argv){
#ifndef NDEBUG
    bool enableValidationLayers = true;
#else
    bool enableValidationLayers = false;
#endif
    uint WIN_WIDTH = 512;
    uint WIN_HEIGHT = 512;

    std::shared_ptr<RayMarcher> pImpl = nullptr;
#ifdef USE_VULKAN
    bool onGPU = true; // TODO: you can read it from command line
    if (onGPU)
    {
        auto ctx = vk_utils::globalContextGet(enableValidationLayers, 0);
        pImpl = CreateRayMarcher_Generated(ctx, WIN_WIDTH * WIN_HEIGHT);
    }
    else
#else
    bool onGPU = false;
#endif
    pImpl = std::make_shared<RayMarcher>();

    pImpl->CommitDeviceData();

    float3 ro;
    float3 lookAt;
    float FOV;
    InputJson::read_camera("./jsons/camera.json", &ro, &lookAt, &FOV);
    pImpl->setCamera(ro, lookAt, FOV);
  
    std::vector<uint> pixelData(WIN_WIDTH * WIN_HEIGHT);

    pImpl->UpdateMembersPlainData();                                            // copy all POD members from CPU to GPU in GPU implementation
    pImpl->RayMarch(pixelData.data(), WIN_WIDTH, WIN_HEIGHT, 4);

    float timings[4] = { 0,0,0,0 };
    pImpl->GetExecutionTime("RayMarch", timings);

    std::stringstream strOut;
    if (onGPU)
        strOut << std::fixed << std::setprecision(2) << "out_gpu_" << 0 << ".bmp";
    else
        strOut << std::fixed << std::setprecision(2) << "out_cpu_" << 0 << ".bmp";
    std::string fileName = strOut.str();

    LiteImage::SaveBMP(fileName.c_str(), pixelData.data(), WIN_WIDTH, WIN_HEIGHT);

    std::cout << "angl = " << 0 << ", timeRender = " << timings[0] << " ms, timeCopy = " << timings[1] + timings[2] << " ms " << std::endl;

    pImpl = nullptr;
    return 0;
}
