#include "pch.h"
#include "core/sceneGraph.h"

using namespace SC;


Transform::Transform() :
	m_position(0, 0, 0),
	m_rotation(glm::vec3(0, 0, 0)),
	m_scale(1, 1, 1)
{

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

