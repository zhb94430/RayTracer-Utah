#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/lights.h"
#include "ExternalLibrary/viewport.cpp"
#include "ExternalLibrary/xmlload.cpp"
#include "ExternalLibrary/lodepng.cpp"
#include "RenderFunctions.cpp"
#include "PixelIterator.h"
#include <thread>

//TODO --------------
// - Write Atomic Pixel Iterator
// - Implement Multi-threading
// - Implement all OpenGL interface

//Define & Init Data Stuctures
RenderImage renderImage;
Camera camera;
Sphere theSphere;
Node rootNode;
MaterialList materials;
LightList lights;

void BeginRender() {
//    std::thread(Render);
}

void StopRender() {
    
}

int main(int argc, const char* argv[]) 
{
//    const char* sceneFile;
//    
//    if (argc == 2) {
//        sceneFile = argv[1];
//    }
    
	//Load Scene
    LoadScene("/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project2.xml");
    
    //Multi Thread Rendering
    PixelIterator i = PixelIterator();
    int CPUCoreNumber = std::thread::hardware_concurrency();
    
    if (CPUCoreNumber == 0) {
        CPUCoreNumber = 1;
    }

    for (int j = 0; j < CPUCoreNumber; j++) {
        std::thread(Render, std::ref(i)).detach();
    }
    
    while (!i.IterationComplete()) {
        
    }
//    ShowViewport();
    
	//Output Image
    renderImage.SaveImage("Result.png");
    renderImage.ComputeZBufferImage();
    renderImage.SaveZImage("ZBuffer.png");
}
