#include "pch.h"
#include "render/scene.h"
#include "render/renderer.h"
#include "render/pipeline.h"
#include "render/mesh.h"
#include "render/buffer.h"
#include "render/texture.h"

using namespace SC;

RenderObject* Scene::CreateRenderObject(RenderObject&& object)
{
	m_renderables.push_back(std::move(object));

	return &m_renderables.back();
}

Mesh* Scene::InsertMesh(const std::string& name, Mesh&& mesh)
{
	auto foundIt = m_meshes.find(name);
	if (foundIt != m_meshes.end())
	{
		return &foundIt->second;
	}

	Mesh& insertedMesh = m_meshes.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(mesh)).first->second;

	//also create the gpu buffers for the mesh
	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST); //Transfer this to gpu only memory

	auto vertexBuffer = SC::Buffer::Create(insertedMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, insertedMesh.vertices.data());
	m_vertexBuffers.emplace(name, std::move(vertexBuffer));

	SC::BufferUsageSet indexBufferUsage;
	indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
	indexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);

	auto indexBuffer = SC::Buffer::Create(insertedMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE, insertedMesh.indices.data());
	m_indexBuffers.emplace(name, std::move(indexBuffer));

	return &insertedMesh;
}


Material* Scene::CreateMaterial(Pipeline* pipeline, PipelineLayout* pipelineLayout, const std::string& name)
{
	auto foundIt = m_materials.find(name);
	if (foundIt != m_materials.end())
	{
		return &foundIt->second;
	}

	auto it = m_materials.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(pipeline, pipelineLayout));  // construct in-place

	return &it.first->second;
}

void Scene::DrawObjects(Renderer* renderer,
	std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc)
{
	CORE_ASSERT(renderer, "Renderer can't be null");

	PipelineLayout* lastLayout{ nullptr };
	for (RenderObject& renderable : m_renderables)
	{
		const bool pipelineChanged = !lastLayout || lastLayout != renderable.material->pipelineLayout;
		if(pipelineChanged)
			renderer->BindPipeline(renderable.material->pipeline);

		renderer->BindVertexBuffer(m_vertexBuffers.at(renderable.name).get());
		renderer->BindIndexBuffer(m_indexBuffers.at(renderable.name).get());

		PerRenderObjectFunc(renderable, pipelineChanged);
		
		renderer->DrawIndexed(renderable.mesh->IndexCount(), 1, 0, 0, 0);

		lastLayout = renderable.material->pipelineLayout;
	}
}

Texture* Scene::CreateTexture(const std::string& path)
{
	auto foundIt = m_textures.find(path);
	if (foundIt != m_textures.end())
	{
		return foundIt->second.get();
	}

	std::unique_ptr<Texture> texture = Texture::Create(TextureType::TEXTURE2D, TextureUsage::COLOUR, Format::R8G8B8A8_SRGB);
	texture->LoadFromFile(path);
	auto it = m_textures.insert(std::make_pair(path, std::move(texture)));

	return it.first->second.get();
}