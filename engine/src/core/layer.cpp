#include "pch.h"

#include "core/layer.h"

using namespace SC;

Layer::Layer(const std::string& name) : m_debugName(name)
{
}

LayerStack::LayerStack()
{
	m_layerInsert = m_layers.begin();
}

void LayerStack::PushLayer(std::shared_ptr<Layer>& layer)
{
	m_layerInsert = m_layers.emplace(m_layerInsert, layer);
}

void LayerStack::PushOverlay(std::shared_ptr<Layer>& layer)
{
	m_layers.emplace_back(layer);
}

void LayerStack::PopLayer(std::shared_ptr<Layer>& layer)
{
	auto it = std::find(m_layers.begin(), m_layers.end(), layer);
	if (it != m_layers.end())
	{
		m_layers.erase(it);
		--m_layerInsert;
	}
}

void LayerStack::PopOverlay(std::shared_ptr<Layer>& layer)
{
	auto it = std::find(m_layers.begin(), m_layers.end(), layer);
	if (it != m_layers.end())
	{
		m_layers.erase(it);
	}
}

void LayerStack::DeleteLayers()
{
	m_layers.clear();
}