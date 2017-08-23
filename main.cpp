#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/xmlload.cpp"

Camera camera = Camera();
RenderImage renderImage = RenderImage();
Sphere theSphere = Sphere();
Node rootNode = Node();



int main(int argc, const char* argv[]) 
{
	//Load Scene
    LoadScene("/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project1.xml");
    
	//Perform Rendering

	//Output Image
}
