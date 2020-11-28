#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>

namespace HyperECS
{
	struct Entity
	{
		size_t Handle;

		bool IsHandleValid() const { return Handle != 0; }
		bool operator==(const Entity& other) const { return Handle == other.Handle; }
	};

	class Registry
	{
	private:
		struct EntityHasher
		{
			size_t operator()(const Entity& entity) const { return (std::hash<size_t>()(entity.Handle)); }
		};

		struct ComponentIndex
		{
			size_t Handle;
			size_t Index;

			ComponentIndex(size_t handle, size_t index)
				: Handle(handle), Index(index) {}

			bool IsComponentValid() const { return Handle != 0 && Index != 0; }
			bool operator==(const ComponentIndex& other) const { return Handle == other.Handle && Index == other.Index; }
		};

		std::queue<size_t> m_FreeIndex;
		std::unordered_map<size_t, std::vector<void*>> m_Components;
		std::unordered_map<Entity, std::vector<ComponentIndex>, EntityHasher> m_Entities;

	public:
		Entity Construct()
		{
			Entity entity = Entity({ m_Entities.size() });
			m_Entities[entity] = {};
			return entity;
		}

		template<class T, typename... Args>
		T& AddComponent(Entity entity, Args&&... args)
		{
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			if (HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity already has the component!" << std::endl;
				__debugbreak();
			}

			size_t componentId = typeid(T).hash_code();
			if (m_Components.find(componentId) == m_Components.end())
				m_Components[componentId] = {};

			size_t index = 0;
			T* component;
			if (m_FreeIndex.empty())
			{
				index = m_Components[componentId].size();
				component = static_cast<T*>(m_Components[componentId].emplace_back((void*) new T(std::forward<Args>(args)...)));
			}
			else
			{
				index = m_FreeIndex.front();
				m_FreeIndex.pop();
				component = static_cast<T*>(m_Components[componentId][index] = ((void*) new T(std::forward<Args>(args)...)));
			}

			m_Entities[entity].emplace_back(componentId, index);

			return *component;
		}

		template<class T>
		void RemoveComponent(Entity entity)
		{
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			if (!HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity has not the component!" << std::endl;
				__debugbreak();
			}

			size_t componentId = typeid(T).hash_code();
			std::vector<ComponentIndex>& components = m_Entities[entity];
			for (ComponentIndex& component : components)
				if (component.Handle == componentId)
				{
					m_Components[componentId][component.Index] = nullptr;
					m_FreeIndex.push(component.Index);
					components.erase(std::find(components.begin(), components.end(), component));
				}
		}

		template<class T>
		T& GetComponent(Entity entity)
		{
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			if (!HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity has not the component!" << std::endl;
				__debugbreak();
			}

			size_t componentId = typeid(T).hash_code();
			std::vector<ComponentIndex> components = m_Entities[entity];
			for (ComponentIndex component : components)
				if (component.Handle == componentId)
					return *static_cast<T*>(m_Components[componentId][component.Index]);
		}

		template<class T>
		bool HasComponent(Entity entity)
		{
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			size_t componentId = typeid(T).hash_code();
			std::vector<ComponentIndex> components = m_Entities[entity];
			for (ComponentIndex component : components)
				if (component.Handle == componentId)
					return true;
			return false;
		}

		std::vector<Entity> GetEntities() const
		{
			std::vector<Entity> entities;
			for (const auto& entity : m_Entities)
				entities.push_back(entity.first);
			return entities;
		}
	};
}