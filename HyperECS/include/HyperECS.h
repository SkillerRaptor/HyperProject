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

	class System
	{
	public:
		virtual void OnTick(Registry& registry, int currentTick) = 0;
		virtual void OnUpdate(Registry& registry, float deltaTime) = 0;
		virtual void OnRender(Registry& registry) = 0;
	};

	class World
	{
	private:
		Registry m_Registry;
		std::unordered_map<size_t, System*> m_Systems;

	public:
		Entity Construct()
		{
			return m_Registry.Construct();
		}

		template<class T, typename... Args>
		constexpr T& AddComponent(Entity entity, Args&&... args)
		{
			return m_Registry.AddComponent<T>(entity, args...);
		}

		template<class T>
		constexpr void RemoveComponent(Entity entity)
		{
			m_Registry.RemoveComponent<T>(entity);
		}

		template<class... T>
		constexpr void RemoveMultipleComponent(Entity entity)
		{
			m_Registry.RemoveMultipleComponent<T...>(entity);
		}

		template<class T>
		constexpr T& GetComponent(Entity entity)
		{
			return m_Registry.GetComponent<T>(entity);
		}

		template<class T>
		constexpr bool HasComponent(Entity entity)
		{
			return m_Registry.HasComponent<T>(entity);
		}

		template<class... T>
		constexpr bool HasMultipleComponent(Entity entity)
		{
			return m_Registry.HasMultipleComponent<T...>(entity);
		}

		void Each(const typename std::common_type<std::function<void(Entity)>>::type function)
		{
			m_Registry.Each(function);
		}

		template<class... T>
		constexpr void Each(const typename std::common_type<std::function<void(Entity, T&...)>>::type function)
		{
			m_Registry.Each<T...>(function);
		}

		std::vector<Entity> GetEntities() const
		{
			return m_Registry.GetEntities();
		}

		template<class... T>
		constexpr std::vector<Entity> GetEntities()
		{
			return m_Registry.GetEntities<T...>();
		}

		template<class T, class = class std::enable_if<std::is_base_of<System, T>::value, T>::type, typename... Args>
		constexpr T& AddSystem(Args&&... args)
		{
			if (HasSystem<T>())
			{
				std::cerr << "[HyperECS] World already has the System!" << std::endl;
				__debugbreak();
			}

			m_Systems[typeid(T).hash_code()] = new T(std::forward<Args>(args)...);
			return *static_cast<T*>(m_Systems.at(typeid(T).hash_code()));
		}

		template<class T, class = class std::enable_if<std::is_base_of<System, T>::value, T>::type>
		constexpr void RemoveSystem()
		{
			if (!HasSystem<T>())
			{
				std::cerr << "[HyperECS] World has not the System!" << std::endl;
				__debugbreak();
			}

			m_Systems.erase(typeid(T).hash_code());
		}

		template<class... T, class = class std::enable_if<std::is_base_of<System, T...>::value, T...>::type>
		constexpr void RemoveMultipleSystem()
		{
			if (!HasMultipleSystem<T...>())
			{
				std::cerr << "[HyperECS] World has not the System!" << std::endl;
				__debugbreak();
			}

			auto lambda = [&]<typename C>() mutable
			{
				RemoveSystem<C>();
			};
			(lambda.template operator() < T > (), ...);
		}

		template<class T, class = class std::enable_if<std::is_base_of<System, T>::value, T>::type>
		constexpr T& GetSystem()
		{
			if (!HasSystem<T>())
			{
				std::cerr << "[HyperECS] World has not the System!" << std::endl;
				__debugbreak();
			}

			return *static_cast<T*>(m_Systems.at(typeid(T).hash_code()));
		}

		template<class T, class = class std::enable_if<std::is_base_of<System, T>::value, T>::type>
		constexpr bool HasSystem()
		{
			return m_Systems.find(typeid(T).hash_code()) != m_Systems.end();
		}

		template<class... T, class = class std::enable_if<std::is_base_of<System, T...>::value, T...>::type>
		constexpr bool HasMultipleSystem()
		{
			bool shouldSkip = false;
			auto lambda = [&]<typename C>() mutable
			{
				if (shouldSkip)
					return;
				if (!HasSystem<C>())
					shouldSkip = true;
			};
			(lambda.template operator() < T > (), ...);
			return !shouldSkip;
		}

		std::vector<System*> GetSystems()
		{
			std::vector<System*> systems;
			for (auto& system : m_Systems)
				systems.push_back(system.second);
			return systems;
		}

		void OnTick(int currentTick)
		{
			for (auto& system : m_Systems)
				system.second->OnTick(m_Registry, currentTick);
		}

		void OnUpdate(float deltaTime)
		{
			for (auto& system : m_Systems)
				system.second->OnUpdate(m_Registry, deltaTime);
		}

		void OnRender()
		{
			for (auto& system : m_Systems)
				system.second->OnRender(m_Registry);
		}
	};
}