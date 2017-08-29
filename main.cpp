#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/lights.h"
#include "ExternalLibrary/viewport.cpp"
#include "ExternalLibrary/xmlload.cpp"
#include "ExternalLibrary/lodepng.cpp"
#include "RenderFunctions.cpp"
#include <thread>


//Define & Init Data Stuctures
RenderImage renderImage = RenderImage();
Camera camera = Camera();
Sphere theSphere = Sphere();
Node rootNode = Node();
MaterialList materials = MaterialList();
LightList lights = LightList();

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
    LoadScene("/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project1Example.xml");
    
    Render();
    
//    ShowViewport();
    
	//Output Image
    renderImage.SaveImage("Result.png");
    renderImage.ComputeZBufferImage();
    renderImage.SaveZImage("ZBuffer.png");
}
