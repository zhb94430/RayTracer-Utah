#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/viewport.cpp"
#include "ExternalLibrary/xmlload.cpp"
#include "ExternalLibrary/lodepng.cpp"
#include "RenderFunctions.cpp"
#include <thread>

Camera camera = Camera();
RenderImage renderImage = RenderImage();
Sphere theSphere = Sphere();
Node rootNode = Node();

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
}
