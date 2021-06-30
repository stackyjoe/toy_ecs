#include "entity.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>

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

