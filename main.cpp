#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/xmlload.cpp"
#include "RenderFunctions.cpp"

Camera camera = Camera();
RenderImage renderImage = RenderImage();
Sphere theSphere = Sphere();
Node rootNode = Node();

bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    return true;
}


int main(int argc, const char* argv[]) 
{
	//Load Scene
    LoadScene("/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project1.xml");
    
	//Perform Rendering
    Render();
    
	//Output Image
    renderImage.SaveImage(<#const char *filename#>);
}
