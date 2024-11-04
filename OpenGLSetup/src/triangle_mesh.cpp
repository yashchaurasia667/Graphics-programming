#include "triangle_mesh.h"

TriangleMesh::TriangleMesh()
{

	// In openGL a vertex is the collection of all the attributes needed to draw a point
	std::vector<float> positions = {
			-1.0f, -1.0f, 0.0f, // bottom left
			1.0f, -1.0f, 0.0f,	// bottom right
			-1.0f, 1.0f, 0.0f,	// top left
			1.0f, 1.0f, 0.0f		// top right
	};
	std::vector<int> colorIndices = {0, 1, 2, 3};
	std::vector<int> elementIndices = {0, 1, 2, 2, 1, 3};
	vertex_count = 6;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	VBOs.resize(2);

	// Position
	glGenBuffers(1, &VBOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

	// position is attribute 0, stride = 3 * sizeof(float) = 12
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void *)0);
	glEnableVertexAttribArray(0);

	// Color
	glGenBuffers(1, &VBOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, colorIndices.size() * sizeof(float), colorIndices.data(), GL_STATIC_DRAW);

	// color is attribute 1
	glVertexAttribIPointer(1, 1, GL_INT, 4, (void *)0);
	glEnableVertexAttribArray(1);

	// Element Buffer
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementIndices.size() * sizeof(int), elementIndices.data(), GL_STATIC_DRAW);
}

void TriangleMesh::draw()
{
	glBindVertexArray(VAO);
	// glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0);
}

TriangleMesh::~TriangleMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(VBOs.size(), VBOs.data());
	glDeleteBuffers(1, &EBO);
}