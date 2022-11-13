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
	for (RenderObject& renderable : m_renderables)
	{
		auto forwardEffect = renderable.material->original->passShaders[MeshpassType::Forward];
		const bool pipelineChanged = !lastLayout || lastLayout != forwardEffect->GetShaderEffect()->GetPipelineLayout();
		if(pipelineChanged)
			renderer->BindPipeline(forwardEffect->GetPipeline());

		renderer->BindVertexBuffer(renderable.mesh->vertexBuffer.get());
		renderer->BindIndexBuffer(renderable.mesh->indexBuffer.get());

		PerRenderObjectFunc(renderable, pipelineChanged);
		
		renderer->DrawIndexed(renderable.mesh->IndexCount(), 1, 0, 0, 0);

		lastLayout = forwardEffect->GetShaderEffect()->GetPipelineLayout();
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