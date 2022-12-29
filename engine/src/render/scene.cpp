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
	auto node = m_root.AddChild();
	node->GetRenderObject() = std::move(object);

	return &node->GetRenderObject();
}

Mesh& Scene::InsertMesh(const std::string& name)
{
	auto foundIt = m_meshes.find(name);
	if (foundIt != m_meshes.end())
	{
		return foundIt->second;
	}

	Mesh& insertedMesh = m_meshes.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple()).first->second;

	return insertedMesh;
}

void Scene::DrawObjects(Renderer* renderer,
	std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc)
{
	CORE_ASSERT(renderer, "Renderer can't be null");

	PipelineLayout* lastLayout{ nullptr };
	m_root.TraverseTree([=, &lastLayout](SceneNode& node)
	{
		RenderObject& renderable = node.GetRenderObject();

		if (!renderable.mesh) return;

		auto forwardEffect = renderable.material->original->passShaders[MeshpassType::Forward];
		const bool pipelineChanged = !lastLayout || lastLayout != forwardEffect->GetShaderEffect()->GetPipelineLayout();
		if (pipelineChanged)
			renderer->BindPipeline(forwardEffect->GetPipeline());

		CORE_ASSERT(renderable.mesh, "Mesh can't be null");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh vertex buffer can't be null, is it built?");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh index buffer can't be null, is it built?");

		renderer->BindVertexBuffer(renderable.mesh->vertexBuffer.get());
		renderer->BindIndexBuffer(renderable.mesh->indexBuffer.get());

		PerRenderObjectFunc(renderable, pipelineChanged);

		renderer->DrawIndexed(renderable.mesh->IndexCount(), 1, 0, 0, 0);

		lastLayout = forwardEffect->GetShaderEffect()->GetPipelineLayout();
	});
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

void Scene::Reset()
{
	m_root.Remove();

	m_meshes.clear();
	m_textures.clear();
	m_vertexBuffers.clear();
	m_indexBuffers.clear();
}

SceneNode& Scene::Root()
{
	return m_root;
}
