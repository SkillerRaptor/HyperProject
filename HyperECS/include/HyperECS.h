#pragma once

#include <functional>
#include <iostream>
#include <mutex>
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

		std::mutex m_FreeLock;
		std::mutex m_ComponentLock;
		std::mutex m_EntityLock;

	public:
		Entity Construct()
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			Entity entity = Entity({ m_Entities.size() });
			m_Entities[entity] = {};
			return entity;
		}

		template<class T, typename... Args>
		constexpr T& AddComponent(Entity entity, Args&&... args)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}
			entityLock.unlock();

			if (HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity already has the component!" << std::endl;
				__debugbreak();
			}

			entityLock.lock();

			size_t componentId = typeid(T).hash_code();
			if (m_Components.find(componentId) == m_Components.end())
				m_Components[componentId] = {};

			std::unique_lock<std::mutex> freeLock(m_FreeLock);
			std::unique_lock<std::mutex> componentLock(m_ComponentLock);

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
		constexpr void RemoveComponent(Entity entity)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}
			entityLock.unlock();

			if (!HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity has not the component!" << std::endl;
				__debugbreak();
			}

			entityLock.lock();

			std::unique_lock<std::mutex> freeLock(m_FreeLock);
			std::unique_lock<std::mutex> componentLock(m_ComponentLock);

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

		template<class... T>
		constexpr void RemoveMultipleComponent(Entity entity)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			entityLock.unlock();

			if (!HasMultipleComponent<T...>(entity))
			{
				std::cerr << "[HyperECS] Entity has not the component!" << std::endl;
				__debugbreak();
			}

			entityLock.lock();

			auto lambda = [&]<typename C>() mutable
			{
				entityLock.unlock();
				RemoveComponent<C>(entity);
				entityLock.lock();
			};
			(lambda.template operator() < T > (), ...);
		}

		template<class T>
		constexpr T& GetComponent(Entity entity)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			entityLock.unlock();

			if (!HasComponent<T>(entity))
			{
				std::cerr << "[HyperECS] Entity has not the component!" << std::endl;
				__debugbreak();
			}

			entityLock.lock();

			std::unique_lock<std::mutex> freeLock(m_FreeLock);
			std::unique_lock<std::mutex> componentLock(m_ComponentLock);

			size_t componentId = typeid(T).hash_code();
			std::vector<ComponentIndex> components = m_Entities[entity];
			for (ComponentIndex component : components)
				if (component.Handle == componentId)
					return *static_cast<T*>(m_Components[componentId][component.Index]);
		}

		template<class T>
		constexpr bool HasComponent(Entity entity)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			std::unique_lock<std::mutex> componentLock(m_ComponentLock);

			size_t componentId = typeid(T).hash_code();
			std::vector<ComponentIndex> components = m_Entities[entity];
			for (ComponentIndex component : components)
				if (component.Handle == componentId)
					return true;
			return false;
		}

		template<class... T>
		constexpr bool HasMultipleComponent(Entity entity)
		{
			std::unique_lock<std::mutex> entityLock(m_EntityLock);
			if (m_Entities.find(entity) == m_Entities.end())
			{
				std::cerr << "[HyperECS] Entity does not exists!" << std::endl;
				__debugbreak();
			}

			bool shouldSkip = false;
			auto lambda = [&]<typename C>() mutable
			{
				if (shouldSkip)
					return;
				entityLock.unlock();
				if (!HasComponent<C>(entity))
					shouldSkip = true;
				entityLock.lock();
			};
			(lambda.template operator() < T > (), ...);
			if (shouldSkip)
				return false;
			return true;
		}

		void Each(const typename std::common_type<std::function<void(Entity)>>::type function)
		{
			for (auto& entity : m_Entities)
				function(entity.first);
		}

		template<class... T>
		constexpr void Each(const typename std::common_type<std::function<void(Entity, T&...)>>::type function)
		{
			for (const auto& entity : m_Entities)
			{
				bool shouldSkip = false;
				auto lambda = [&]<typename C>() mutable
				{
					if (shouldSkip)
						return;
					if (!HasComponent<C>(entity.first))
						shouldSkip = true;
				};
				(lambda.template operator() < T > (), ...);
				if (shouldSkip)
					continue;
				function(entity.first, GetComponent<T>(entity.first)...);
			}
		}

		std::vector<Entity> GetEntities() const
		{
			std::vector<Entity> entities;
			for (const auto& entity : m_Entities)
				entities.push_back(entity.first);
			return entities;
		}

		template<class... T>
		constexpr std::vector<Entity> GetEntities()
		{
			std::vector<Entity> entities;
			for (auto& entity : m_Entities)
			{
				bool shouldSkip = false;
				auto lambda = [&]<typename C>() mutable {
					if (shouldSkip)
						return;
					if (!HasComponent<C>(entity.first))
						shouldSkip = true;
				}; (lambda.template operator() < T > (), ...);
				if (shouldSkip)
					continue;
				entities.push_back(entity.first);
			}
			return entities;
		}
	};
}