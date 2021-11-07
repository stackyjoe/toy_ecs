#ifndef ECS_HPP
#define ECS_HPP

#include "TypeList.hpp"
#include "concepts.hpp"

#include <iostream>

template<template<class...> class ArchetypeContainerModel, template<class...> class ComponentContainerModel, class ListOfTypes>
struct ECS {
	using ArchetypeTypeList = ListsViaTypes::PowersetOf<ListOfTypes>;

	template<class S>
	using GetIteratorTypeOf = typename S::iterator;

	template<class S>
	using GetConstIteratorTypeOf = typename S::const_iterator;

	template<class TypeList>
	struct entity;

	template<class ... HeldComponents>
	struct entity<ListsViaTypes::TypeList<HeldComponents...>> {
		using held_components = ListsViaTypes::TypeList<HeldComponents...>;
		using container_types = held_components::template apply_to_each<ComponentContainerModel>;
		using iterator_types = container_types::template apply_to_each<GetIteratorTypeOf>;
		using const_iterator_types = container_types::template apply_to_each<GetConstIteratorTypeOf>;
		using pointer_types = held_components::template apply_to_each<std::add_pointer_t>;
		using const_pointer_types = held_components::template apply_to_each<std::add_const_t>::template apply_to_each<std::add_pointer_t>;

		pointer_types::as_tuple pointers_to_components;

		template<class R>
		auto set_component(R * r) {
			static_assert(held_components::template contains<R>(), "get_component() called with template parameter not in the archetype of the entity.");
			constexpr size_t index = typename held_components::template get_index_of<R>();
			using ptr_to_R_t = typename pointer_types::template type_of_index<index>;
			std::get<ptr_to_R_t>(pointers_to_components) = r;
			return r;
		}


		template<class R>
		auto get_component() -> R & {
			static_assert(held_components::template contains<R>(), "get_component() called with template parameter not in the archetype of the entity.");
			constexpr size_t index = typename held_components::template get_index_of<R>();
			using ptr_to_R_t = typename pointer_types::template type_of_index<index>;
			auto ptr = std::get<ptr_to_R_t>(pointers_to_components);
			return *ptr;
		}

		template<class R>
		auto get_component() const  -> R const & {
			static_assert(held_components::template contains<R>(), "get_component() called with template parameter not in the archetype of the entity.");

			constexpr size_t index = typename held_components::template index_of_type<R>();
			using const_itr_to_R = typename const_iterator_types::template type_of_index<index>;
			using cptr_to_R_t = typename pointer_types::template type_of_index<index>;
			auto cptr = std::get<cptr_to_R_t>(pointers_to_components);
			return *cptr;
		}
	};

	using Archetypes = ArchetypeTypeList::template apply_to_each<entity>;

	template<class List>
	auto& get_storage_for_archetypes() {
		return std::get<ArchetypeContainerModel<entity<List>>>(archetype_storage);
	}

	template<class ... Ts>
	auto& get_storage_for_archetype_with_components() {
		static_assert(ListOfTypes::template contains_all<Ts...>(), "get_storage_for_archetype_with_components() called on list of types containing at least one type not in the ECS type list.");
		return std::get<ArchetypeContainerModel<entity<TypeList<Ts...>>>>(archetype_storage);
	}

	template<class List, class Functional>
	void apply_to_entities_of_archetype(Functional const &f) {
		auto& storage = get_storage_for_archetypes<List>();
		for (auto & up : storage) {
			f(up.get());
		}
	}

	template<class List, class Functional>
	void for_each_entity_with_components(Functional const & f) {
		auto wrapped = [this, &f]<class R>() mutable -> void {
			if constexpr (ListsViaTypes::ContainsAll<R::held_components, List>) {
				this->apply_to_entities_of_archetype<R::held_components>(f);
			}
		};

		Archetypes::template call_on_each_type(wrapped);
	}


	template<class ... ComponentTypes>
	auto create_entity() -> entity<ListsViaTypes::TypeList<ComponentTypes...>> * {
		static_assert(ListOfTypes::template contains_all<ComponentTypes...>(),
			"create_entity() called with first template parameter which is a TypeList but which contains types not in the TypeList expected by the ECS class.");

		using EntityType = entity<ListsViaTypes::TypeList<ComponentTypes...>>;

		ArchetypeContainerModel<EntityType> & this_archetype = std::get<ArchetypeContainerModel<EntityType>>(archetype_storage);

		auto * instance = this_archetype.create_new();

		((instance->set_component<ComponentTypes>(std::get<ComponentContainerModel<ComponentTypes>>(component_storage).create_new())),...);

		return instance;
	}

private:
	ListOfTypes::template apply_to_each<ComponentContainerModel>::as_tuple component_storage;
	Archetypes::template apply_to_each<ArchetypeContainerModel>::as_tuple archetype_storage;

};


#endif // ECS_HPP