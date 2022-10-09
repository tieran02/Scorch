#include "pch.h"
#include "render/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace SC;

namespace
{
	void convertAiVertex(uint32_t index, aiMesh* aimesh, Vertex& outVertex)
	{
		glm::vec3 vector;
		vector.x = aimesh->mVertices[index].x;
		vector.y = aimesh->mVertices[index].y;
		vector.z = aimesh->mVertices[index].z;
		outVertex.position = vector;

		vector.x = aimesh->mNormals[index].x;
		vector.y = aimesh->mNormals[index].y;
		vector.z = aimesh->mNormals[index].z;
		outVertex.normal = vector;

		if (aimesh->HasVertexColors(index))
		{
			vector.x = aimesh->mColors[index]->r;
			vector.y = aimesh->mColors[index]->g;
			vector.z = aimesh->mColors[index]->b;
			outVertex.color = vector;
		}
	}

	Mesh processMesh(aiMesh* aimesh, const aiScene* scene, bool indexBuffer)
	{
		Mesh mesh;

		//TODO: add index buffer to reduce duplicated vertices
		for (uint32_t i = 0; i < aimesh->mNumFaces; i++)
		{
			const aiFace& face = aimesh->mFaces[i];

			if (!indexBuffer) 
			{
				//duplicate vertices as were not using a index buffer
				for (uint32_t j = 0; j < 3; ++j)
				{
					uint32_t index = face.mIndices[j];
					Vertex vertex;
					convertAiVertex(index, aimesh, vertex);
					mesh.vertices.push_back(vertex);
				}
			}
			else
			{
				for (uint32_t i = 0; i < 3; ++i)
				{
					uint32_t index = face.mIndices[i];
					mesh.indices.push_back(face.mIndices[i]);
				}
			}
		}

		if (indexBuffer)
		{
			//using index buffer so just store vertices and use the index buffer to draw
			for (uint32_t i = 0; i < aimesh->mNumVertices; ++i)
			{
				Vertex vertex;
				convertAiVertex(i, aimesh, vertex);
				mesh.vertices.push_back(vertex);
			}
		}

		return mesh;
	}

	void processNode(aiNode* node, const aiScene* scene, std::vector<SC::RenderObject>& outObjects, bool indexBuffer)
	{
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			SC::RenderObject renderObject;
			renderObject.mesh = processMesh(mesh, scene, indexBuffer);
			outObjects.push_back(renderObject);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene, outObjects, indexBuffer);
		}
	}

}

bool RenderObject::LoadFromFile(const std::string& path, std::vector<RenderObject>& model, bool useIndexBuffer)
{
	// Check if file exists
	std::ifstream fin(path.c_str());
	if (fin.fail())
	{
		CORE_ASSERT(false, string_format("Failed to load model: %s", path.c_str()));
		return false;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);

	processNode(scene->mRootNode, scene, model, useIndexBuffer);
	return !model.empty();
}

RenderObject::RenderObject() : transform(glm::mat4(1))
{

}

uint32_t Mesh::VertexCount() const
{
	return static_cast<uint32_t>(vertices.size());
}

uint32_t SC::Mesh::VertexSize() const
{
	return static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
}

uint32_t Mesh::IndexCount() const
{
	return static_cast<uint32_t>(indices.size());
}

uint32_t Mesh::IndexSize() const
{
	return static_cast<uint32_t>(indices.size() * sizeof(VertexIndexType));
}