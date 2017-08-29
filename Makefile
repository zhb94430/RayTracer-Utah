RayTracer:
	gcc -o RayTracer main.cpp -framework OpenGL -framework GLUT ExternalLibrary/tinyxml/tinystr.o ExternalLibrary/tinyxml/tinyxml.o ExternalLibrary/tinyxml/tinyxmlerror.o ExternalLibrary/tinyxml/tinyxmlparser.o -lc++

clean:
	rm RayTracer
