#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/lights.h"
#include "ExternalLibrary/viewport.cpp"
#include "ExternalLibrary/xmlload.cpp"
#include "ExternalLibrary/lodepng.cpp"
#include "ExternalLibrary/cyPhotonMap.h"
#include "RenderFunctions.cpp"
#include "PixelIterator.h"
#include <thread>

//TODO --------------


//Define & Init Data Stuctures
RenderImage renderImage;
Camera camera;
Sphere theSphere;
Plane thePlane;
Node rootNode;
MaterialList materials;
LightList lights;
ObjFileList objList;
TexturedColor background;
TexturedColor environment;
TextureList textureList;
cy::PhotonMap pMap;

void SpawnRenderThreads() {
    // Generate Photon Map before rendering
    GeneratePhotonMap();
    
    //Multi Thread Rendering
//    PixelIterator i = PixelIterator();
//    int CPUCoreNumber = std::thread::hardware_concurrency();
//
//    if (CPUCoreNumber == 0) {
//        CPUCoreNumber = 1;
//    }
//
////    #if DEBUG
////    DEBUG PURPOSE
////        CPUCoreNumber = 1;
////    #endif
//
//    for (int j = 0; j < CPUCoreNumber; j++) {
//        std::thread(Render, std::ref(i)).detach();
//    }
//
//    while (!i.IterationComplete()) {
//
//    }
    
    //Output Image
    FILE *fp=fopen("photonmap.dat","wb");
    fwrite(pMap.GetPhotons(),sizeof(cyPhotonMap::Photon),pMap.NumPhotons(),fp);
    fclose(fp);
    
//    renderImage.SaveImage("Result.png");
//    renderImage.ComputeZBufferImage();
//    renderImage.SaveZImage("ZBuffer.png");
//    renderImage.ComputeSampleCountImage();
//    renderImage.SaveSampleCountImage("SampleCount.png");
}

void BeginRender() {
    std::thread(SpawnRenderThreads).detach();
}

void StopRender() {
    
}

int main(int argc, const char* argv[]) 
{
    const char* sceneFile;
    
    if (argc == 2) {
        sceneFile = argv[1];
        LoadScene(sceneFile);
    }
    //Load default scene if no sceneFile provided
    else {
        LoadScene("/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project11/scene_glossy_soft.xml");
    }
    
    ShowViewport();
}
