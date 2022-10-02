#pragma once

namespace SC
{
	class Event;
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return m_debugName; }
	protected:
		std::string m_debugName;
	};

	class LayerStack
	{
	public:
		LayerStack();

		void PushLayer(std::shared_ptr<Layer>& layer);
		void PushOverlay(std::shared_ptr<Layer>& layer);
		void PopLayer(std::shared_ptr<Layer>& layer);
		void PopOverlay(std::shared_ptr<Layer>& layer);
		void DeleteLayers();

		std::vector<std::shared_ptr<Layer>>::iterator begin() { return m_layers.begin(); }
		std::vector<std::shared_ptr<Layer>>::iterator end() { return m_layers.end(); }
	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
		std::vector<std::shared_ptr<Layer>>::iterator m_layerInsert;
	};
}
