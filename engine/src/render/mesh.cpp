#include "pch.h"
#include "render/mesh.h"
#include "render/materialSystem.h"
#include "jaam.h"
#include "render/texture.h"
#include "render/renderer.h"
#include "render/buffer.h"

using namespace SC;

RenderObject::RenderObject() : mesh(nullptr), material(nullptr)
{

}

uint32_t Mesh::VertexCount() const
{
	return static_cast<uint32_t>(vertices.size());
}

Mesh::Mesh() : vertexBuffer(nullptr), indexBuffer(nullptr)
{

}

Mesh::~Mesh()
{
	vertexBuffer.reset();
	indexBuffer.reset();
}

Mesh::Mesh(Mesh&& other)
{
	vertexBuffer = std::move(other.vertexBuffer);
	indexBuffer = std::move(other.indexBuffer);

	vertices = std::move(other.vertices);
	indices = std::move(other.indices);
}

Mesh& Mesh::operator=(Mesh&& other)
{
	vertexBuffer = std::move(other.vertexBuffer);
	indexBuffer = std::move(other.indexBuffer);

	vertices = std::move(other.vertices);
	indices = std::move(other.indices);

	return *this;
}

uint32_t SC::Mesh::VertexSize() const
{
	return static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
}

bool Mesh::Build()
{
	if(vertexBuffer)
		Log::PrintCore("Mesh::Build: Vertex buffer already created, this will overwrite the existing buffer", LogSeverity::LogWarning);
	if (indexBuffer)
		Log::PrintCore("Mesh::Build: Index buffer already created, this will overwrite the existing buffer", LogSeverity::LogWarning);

	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST); //Transfer this to gpu only memory

	vertexBuffer = SC::Buffer::Create(VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, vertices.data());

	SC::BufferUsageSet indexBufferUsage;
	indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
	indexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);

	indexBuffer = SC::Buffer::Create(IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE, indices.data());

	return vertexBuffer && indexBuffer;
}

uint32_t Mesh::IndexCount() const
{
	return static_cast<uint32_t>(indices.size());
}

uint32_t Mesh::IndexSize() const
{
	return static_cast<uint32_t>(indices.size() * sizeof(VertexIndexType));
}
