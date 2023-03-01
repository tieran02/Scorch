#include "pch.h"
#include "core/sceneGraph.h"

using namespace SC;

Transform::Transform() : position(0), scale(1), rotation(glm::vec3(0))
{

}

void Transform::Translate(const glm::vec3& t)
{
	position += t;
}

void Transform::SetPosition(const glm::vec3& p)
{
	position = p;
}

const glm::vec3& Transform::GetPosition() const
{
	return position;
}

void Transform::SetScale(const glm::vec3& s)
{
	scale = s;
}

const glm::vec3& Transform::GetScale() const
{
	return scale;
}

void Transform::Rotate(const glm::vec3& axis, float radians)
{
	rotation = glm::angleAxis(radians, axis) * rotation;

}

void Transform::SetRotation(const glm::vec3& axis, float radians)
{
	rotation = glm::angleAxis(radians, axis);
}

const glm::quat& Transform::GetRotation() const
{
	return rotation;
}

glm::mat4 Transform::ModelMatrix() const
{
	glm::mat4 transform(1.0f);
	transform = glm::translate(transform, position);
	transform *= glm::toMat4(rotation);
	transform = glm::scale(transform, scale);

	return std::move(transform);
}


SceneNode::SceneNode() :
	m_parent(nullptr)
{

}

void SceneNode::Remove()
{
	m_children.clear();

	//Make sure to remove ourself from the parent
	if (m_parent)
	{
		auto new_end = std::remove_if(m_parent->m_children.begin(), m_parent->m_children.end(),
			[=](const auto& node)
			{ 
				return node.get() == this;
			});

		m_parent->m_children.erase(new_end, m_parent->m_children.end());
	}
}

SceneNode::~SceneNode()
{

}

void SceneNode::SetParent(SceneNode& parent)
{
	SceneNode* oldParent = m_parent;

	m_parent = &parent;

	//Make sure to move ourself to the new parent
	if (oldParent)
	{
		auto self = std::find_if(oldParent->m_children.begin(), oldParent->m_children.end(),
			[=](const auto& node)
			{
				return node.get() == this;
			});

		if (self != oldParent->m_children.end())
		{
			m_parent->m_children.push_back(*self);
		}

		oldParent->m_children.erase(self, oldParent->m_children.end());
	}

}

RenderObject& SceneNode::GetRenderObject()
{
	return m_renderObject;
}

void SceneNode::TraverseTree(std::function<void(SceneNode& node)> func)
{
	std::stack<SceneNode*> stack;
	stack.push(this);

	while (!stack.empty()) 
	{
		SceneNode* currentNode = stack.top();
		stack.pop();

		// push all children onto the stack:
		for (std::list<std::shared_ptr<SceneNode>>::iterator i = currentNode->m_children.begin();
			i != currentNode->m_children.end();
			i++)
		{
			stack.push(i->get());
		}

		// do any processing for this node here
		func(*currentNode);
	}
}

Transform& SceneNode::GetTransform()
{
	return m_transform;
}

const std::list<std::shared_ptr<SC::SceneNode>>& SceneNode::Children() const
{
	return m_children;
}

