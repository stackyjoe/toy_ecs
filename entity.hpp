#ifndef Entity_H
#define Entity_H 1

#include <atomic>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include "TypeList.hpp"
#include "tuple_of_optionals.hpp"

template<typename ComponentTypesList>
class Entity;


// EntityManager via inheritance
template<typename ... ComponentTypes>
class Entity<ListsViaTypes::TypeList<ComponentTypes...>> {
	using ArgsTypeList = TypeList<ComponentTypes...>;
	using ThisType = Entity<ArgsTypeList>;


public:
	Entity() : id(0) { }
	Entity(Entity&&) = default;
	Entity& operator=(Entity&&) = default;

	size_t id;

    Entity(typename ArgsTypeList::template apply_to_each<std::optional>::as_tuple && initializer_data)
		: id(0), storage(std::move(initializer_data)) {

	}

	Entity(ComponentTypes && ... components)
		: id(0), storage() {
        (storage.template make_in_place<ComponentTypes>(std::move(components)), ...);
	}

	template<typename T>
	T * get_component() noexcept {
		static_assert(ArgsTypeList::template contains<T>(), "Entity::get_component() called on type not in TypeList.");

        return storage.template get<T>();
	}



	template<typename T>
	T const * get_component() const noexcept {
		static_assert(ArgsTypeList::template contains<T>(), "Entity::get_component() called on type not in TypeList.");

        return storage.template get<T>();
	}

	template<typename T>
	T* get_value() noexcept {
		static_assert(ArgsTypeList::template contains<T>(), "Entity::get_component() called on type not in TypeList.");

        return storage.template get<T>();
	}

	template<typename T>
	T const* get_value() const noexcept {
		static_assert(ArgsTypeList::template contains<T>(), "Entity::get_component() called on type not in TypeList.");

        return storage.template get<T>();
	}

private:
	typename ListsViaTypes::TypeList<ComponentTypes...>::template apply_to_each<std::optional>::as_tuple storage;
	//tuple_of_optionals<TypeList<ComponentTypes ...>> storage;

};

template< typename ComponentTypeList, template<typename...> typename ContainerModel >
class EntityManager;


template<typename ... ComponentTypes, template<typename...> typename ContainerModel>
class EntityManager< Entity<ListsViaTypes::TypeList<ComponentTypes...>>, ContainerModel >  {
public:
	using ArgsTypeList = TypeList<ComponentTypes...>;
	using Entity_t = Entity <ArgsTypeList>;
	using ThisType = EntityManager<Entity_t, ContainerModel>;

	EntityManager() = default;


	void reserve(size_t desired_capacity) {
		entity_storage.reserve(desired_capacity);
		(get_storage_for_component<ComponentTypes>().reserve(desired_capacity),...);
	}

	template<typename ... Ts>
	void make_entity(Ts&& ... ts) {
		auto new_entity = std::make_unique<Entity_t>(std::forward<Ts>(ts)...);
		auto* address = new_entity.get();

		entity_storage.push_back(std::move(new_entity));

		address->id = ++global_id;
		register_entity(*address);
	}
	
	void push_back(std::unique_ptr<Entity_t> entity) {
		auto& actual_entity = *entity;
		entity_storage.push_back(std::move(entity));

		register_entity(actual_entity);
		actual_entity.id = ++global_id;
		
	}

	auto const & get_storage_for_entities() const noexcept{
		return entity_storage;
	}

	auto& get_storage_for_entities() noexcept {
		return entity_storage;
	}

	template<typename T>
	auto const & get_storage_for_component() const noexcept {
		static_assert(ListsViaTypes::TypeList<ComponentTypes...>::template contains<T>(), "EntityManager::get_storage_for_component called on type not in TypeList.");

		return std::get<ContainerModel<std::add_pointer_t<T>>>(storage);
	}

	template<typename T>
	auto & get_storage_for_component() noexcept {
		static_assert(ListsViaTypes::TypeList<ComponentTypes...>::template contains<T>(), "EntityManager::get_storage_for_component called on type not in TypeList.");

		return std::get<ContainerModel<std::add_pointer_t<T>>>(storage);
	}

private:

	void register_entity(Entity_t & e) {

	// I finally got a chance to use generic lambdas
	// Woe is me


            auto try_register_all_components = [this](auto& entity) mutable {

                auto try_register_component = [this]<class T>(auto & entity) mutable {
                    auto maybe_push_back_component_address = [this] <class R> (auto & entity, auto & storage_for_R) mutable -> void {
                        if (entity.template get_component<R>() != nullptr) {
                            storage_for_R.push_back(entity.template get_component<R>());
                        }
                    };

                    maybe_push_back_component_address.template operator() < T > (entity, this->get_storage_for_component<T>());
                };
            
	        (try_register_component.template operator() < ComponentTypes > (entity), ...);
            };

            try_register_all_components(e);
	}


	std::atomic<size_t> global_id;
	typename std::vector<std::unique_ptr<Entity_t>> entity_storage;
	typename ListsViaTypes::TypeList<ComponentTypes...>::template apply_to_each<std::add_pointer_t>::template apply_to_each<ContainerModel>::as_tuple storage;
};
#endif
