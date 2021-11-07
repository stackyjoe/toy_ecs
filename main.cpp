#include "ecs.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "set.hpp"

int main() {
    using namespace ListsViaTypes;
    using namespace jl::containers;
    using ComponentsList = TypeList<int, std::string, set<int>>;
    using myECS = ECS<set, set, ComponentsList>;

    auto ecs = std::make_unique<myECS>();
    try {
        myECS::entity<TypeList<std::string>>* obj1 = ecs->create_entity<std::string>();
        auto & string_component = obj1->get_component<std::string>();
        string_component = "meow";

        auto * obj2 = ecs->create_entity<int, std::string>();
        auto & string_component2 = obj2->get_component<std::string>();
        string_component2 = "woof";
        auto & int_component = obj2->get_component<int>();
        int_component = 5;

        auto* obj3 = ecs->create_entity<std::string, set<int>>();
        auto& string_component3 = obj3->get_component<std::string>();
        string_component3 = "nay";
        auto& set_component = obj3->get_component<set<int>>();
        *set_component.create_new() = 5;
    }
    catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }

    /*
    ecs->apply_to_entities_of_archetype<TypeList<int, std::string>>([](auto const& entity) {
        std::cout << entity->get_component<std::string>() << "\n";
    });*/

    auto print = [](auto const& entity) {
        std::cout << entity->get_component<std::string>() << "\n";
    };

    ecs->for_each_entity_with_components<TypeList<std::string>>(print);


    return EXIT_SUCCESS;
}

/*
int main() {
    using ComponentsList = TypeList<int,std::string,std::map<int,int>>;
//    using ComponentsAsOptionalTuple = ComponentsList::template apply_to_each<std::optional>::as_tuple;
    using EntityType = Entity<ComponentsList>;
    using EntityManagerType = EntityManager<EntityType, std::vector>;


    EntityManagerType a;

    a.reserve(10);
    a.make_entity(6, std::string("abc"), std::map<int,int>({std::make_pair(1,1)}));
    a.make_entity(7, std::string("efg"), std::map<int,int>({std::make_pair(2,2)}));
    a.make_entity(8, std::string("hij"), std::map<int,int>({std::make_pair(3,3)}));
    a.make_entity(9, std::string("klm"), std::map<int,int>({std::make_pair(4,4)}));


    for(auto *number : a.get_storage_for_component<int>()) {
	    std::cout << *number << std::endl;
    }

    for(auto *word : a.get_storage_for_component<std::string>()) {
	    std::cout << *word << std::endl;
    }

    for(auto *map : a.get_storage_for_component<std::map<int,int>>()) {
            for(auto & [key,value] : *map) {
	        std::cout << value << std::endl;
	    }
    }

    return EXIT_SUCCESS;
}
*/
