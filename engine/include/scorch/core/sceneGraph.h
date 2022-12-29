#pragma once
#include "scorch/render/mesh.h"
#include "xutility"

namespace SC 
{
	struct Transform
	{
		Transform();

		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale;
	};

	struct SceneNode
	{

		SceneNode();
		~SceneNode();

		template<typename... TArgs>
		std::shared_ptr<SceneNode> AddChild(const TArgs&... args);

		void Remove();
		void SetParent(SceneNode& parent);

		RenderObject& GetRenderObject();

		void TraverseTree(std::function<void(SceneNode& node)> func);

	private:
		std::list<std::shared_ptr<SceneNode>> m_children;
		SceneNode* m_parent;
		Transform m_transform;

		RenderObject m_renderObject;
	};

	template<typename... TArgs>
	std::shared_ptr<SceneNode> SC::SceneNode::AddChild(const TArgs&... args)
	{
		m_children.emplace_back(std::make_shared<SceneNode>(args...));
		m_children.back()->m_parent = this;
		return m_children.back();
	}

}