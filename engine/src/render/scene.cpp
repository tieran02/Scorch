#include "pch.h"
#include "render/scene.h"
#include "render/renderer.h"
#include "render/pipeline.h"
#include "render/mesh.h"
#include "render/buffer.h"

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
	vertexBufferUsage.set(SC::BufferUsage::MAP); //todo stage vertex buffer to fast memory

	auto vertexBuffer = SC::Buffer::Create(insertedMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE);
	{
		auto mappedData = vertexBuffer->Map();
		memcpy(mappedData.Data(), insertedMesh.vertices.data(), insertedMesh.VertexSize());
	}
	m_vertexBuffers.emplace(name, std::move(vertexBuffer));

	SC::BufferUsageSet indexBufferUsage;
	indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
	indexBufferUsage.set(SC::BufferUsage::MAP);

	auto indexBuffer = SC::Buffer::Create(insertedMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE);
	{
		auto mappedData = indexBuffer->Map();
		memcpy(mappedData.Data(), insertedMesh.indices.data(), insertedMesh.IndexSize());
	}
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
