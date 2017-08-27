#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/xmlload.cpp"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/viewport.cpp"
#include "RenderFunctions.cpp"

Camera camera = Camera();
RenderImage renderImage = RenderImage();
Sphere theSphere = Sphere();
Node rootNode = Node();

void BeginRender() {
    
}

void StopRender() {
    
}

int main(int argc, const char* argv[]) 
{
    const char* sceneFile;
    
    if (argc == 2) {
        sceneFile = argv[1];
    }
    
	//Load Scene
    LoadScene(sceneFile);
    
	//Perform Rendering
    Render();
    
	//Output Image
//    renderImage.SaveImage("Result.png");
}
