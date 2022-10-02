#include "pch.h"
#include "render/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace SC;

namespace
{
	Mesh processMesh(aiMesh* aimesh, const aiScene* scene)
	{
		Mesh mesh;

		//TODO: add index buffer to reduce duplicated vertices
		for (unsigned int i = 0; i < aimesh->mNumFaces; i++)
		{
			const aiFace& face = aimesh->mFaces[i];

			for (int j = 0; j < 3; ++j)
			{
				uint32_t index = face.mIndices[j];
				Vertex vertex;

				glm::vec3 vector;
				vector.x = aimesh->mVertices[index].x;
				vector.y = aimesh->mVertices[index].y;
				vector.z = aimesh->mVertices[index].z;
				vertex.position = vector;

				vector.x = aimesh->mNormals[index].x;
				vector.y = aimesh->mNormals[index].y;
				vector.z = aimesh->mNormals[index].z;
				vertex.normal = vector;

				if (aimesh->HasVertexColors(index))
				{
					vector.x = aimesh->mColors[index]->r;
					vector.y = aimesh->mColors[index]->g;
					vector.z = aimesh->mColors[index]->b;
					vertex.color = vector;
				}

				mesh.vertices.push_back(vertex);
			}
		}

		return mesh;
	}

	void processNode(aiNode* node, const aiScene* scene, std::vector<SC::RenderObject>& outObjects)
	{
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			SC::RenderObject renderObject;
			renderObject.mesh = processMesh(mesh, scene);
			outObjects.push_back(renderObject);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene, outObjects);
		}
	}

}

bool RenderObject::LoadFromFile(const std::string& path, std::vector<RenderObject>& model)
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

	processNode(scene->mRootNode, scene, model);
}

uint32_t Mesh::VertexCount() const
{
	return vertices.size();
}

uint32_t SC::Mesh::Size() const
{
	return vertices.size() * sizeof(Vertex);
}
