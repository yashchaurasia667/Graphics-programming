#include "config.h"

class TriangleMesh
{
public:
    TriangleMesh();
    void draw();
    ~TriangleMesh();

    // VBO -> Vertex Buffer Object
    // VAO -> Vertex Array Object
    // EBO -> Element Buffer Object
private:
    unsigned int EBO, VAO, vertex_count;
    std::vector<unsigned int> VBOs;
};