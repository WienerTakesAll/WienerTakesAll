#include "RenderObject.h"

#include <iostream>

#include <assimp\Importer.hpp> 
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <glm\gtc\type_ptr.hpp>

void RenderObject::render_views
(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
	glVertexAttribPointer
		( 0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData)
		, reinterpret_cast<void*>(offsetof(VertexData, position)) );
	glVertexAttribPointer
		( 1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData)
		, reinterpret_cast<void*>(offsetof(VertexData, normal)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);

	GLuint uniformMVP = glGetUniformLocation(program_id, "MVP");

	for (unsigned int i = 0; i < count; i++)
	{
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(cameras[i]));
		glViewport(320 * (i % 2), 240 * ((i % 4) / 2), 320, 240);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}



}

void RenderObject::load_from_file(const std::string& file_path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile
		(	file_path,
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		std::cout << importer.GetErrorString();
		return;
	}

	if (!scene->HasMeshes())
	{
		std::cout << "No mesh found in model " << file_path << std::endl;
	}

	auto& mesh = scene->mMeshes[0];
	for (unsigned int f_i = 0; f_i < mesh->mNumFaces; f_i++)
	{
		auto& faces = mesh->mFaces[f_i];

		for (unsigned int i = 0; i < faces.mNumIndices; i++)
			indices.emplace_back(faces.mIndices[i]);

	}

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexData vertex;

		//Load vertex positions
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;

		//Load normals
		if (mesh->HasNormals())
		{
			vertex.normal[0] = mesh->mNormals[i].x;
			vertex.normal[1] = mesh->mNormals[i].y;
			vertex.normal[2] = mesh->mNormals[i].z;
		}

		//Load colors
		if (mesh->GetNumColorChannels())
		{
			vertex.colors[0] = mesh->mColors[i][0].r;
			vertex.colors[1] = mesh->mColors[i][0].g;
			vertex.colors[2] = mesh->mColors[i][0].b;
		}

		//Load uvs
		if (mesh->HasTextureCoords(0))
		{
			vertex.uv[0] = mesh->mTextureCoords[i][0].x;
			vertex.uv[1] = mesh->mTextureCoords[i][0].y;
		}

		vertices.push_back(vertex);
	}

	setupBuffer();
}

void RenderObject::setupBuffer()
{
	glGenBuffers(1, &gl_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData)*vertices.size(), &vertices.front(), GL_STATIC_DRAW);

	glGenBuffers(1, &gl_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices.front(), GL_STATIC_DRAW);

}