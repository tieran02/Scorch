#pragma once
#include "scorch/render/mesh.h"
#include "xutility"

namespace SC 
{
	struct Transform
	{
	public:
		Transform();

		void Translate(const glm::vec3& translate);
		void SetPosition(const glm::vec3& position);
		const glm::vec3& GetPosition() const;

		void SetScale(const glm::vec3& scale);
		const glm::vec3& GetScale() const;


		void Rotate(const glm::vec3& axis, float radians);
		void SetRotation(const glm::vec3& axis, float radians);
		const glm::quat& GetRotation() const;

		glm::mat4 ModelMatrix() const;
	private:
		glm::vec3 position;
		glm::vec3 scale;
		glm::quat rotation;
	};

	struct SceneNode
	{

		SceneNode();
		~SceneNode();

		template<typename... TArgs>
		std::shared_ptr<SceneNode> AddChild(const TArgs&... args);

		void Remove();
		void SetParent(SceneNode& parent);

		const std::list<std::shared_ptr<SceneNode>>& Children() const;

		RenderObject& GetRenderObject();

		void TraverseTree(std::function<void(SceneNode& node)> func);

		Transform& GetTransform();

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