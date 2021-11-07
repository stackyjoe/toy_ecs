#ifndef TypeListHPP_HEADER_GUARD
#define TypeListHPP_HEADER_GUARD

#include <algorithm>
#include <tuple>
#include <type_traits>
#include <variant>


namespace ListsViaTypes {

	template <typename T, typename... Ts>
	struct Indexer;

	template <typename T, typename... Ts>
	struct Indexer<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

	template <typename T, typename U, typename... Ts>
	struct Indexer<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Indexer<T, Ts...>::value> {};

	template<typename ... Types>
	struct TypeList {

		using as_tuple = std::tuple<Types...>;
		using as_variant = std::variant<Types...>;

		template<template<typename ...> typename Functional>
		using apply_to_each = TypeList< Functional<Types> ... >;

		template<typename Functional>
		static void call_on_each_type(Functional & f) {
			(f.template operator()<Types>(), ...);
		};

		template<template<typename ...> typename Functional>
		using pass_types_as_parameters_to = Functional<Types...>;

		template<typename T>
		using prepend = TypeList<T, Types...>;

		template<typename T>
		using append = TypeList<Types..., T>;

		template<size_t N>
		using type_of_index = std::tuple_element_t<N, as_tuple>;

		template<typename T>
		static constexpr size_t get_index_of() {
			return Indexer<T, Types...>::value;
		}

		template<typename T>
		static constexpr bool contains() {
			return std::disjunction_v<std::is_same<T, Types>...>;
		}

		template<typename ...Ts>
		static constexpr bool contains_all() {
			return (contains<Ts>() && ...);
		}


	};

	// Declare but do not implement a Concatenate type ...
	template<typename List1, typename List2, typename ... Lists>
	struct Concatenate;

	// and implement it only when its arguments are TypeLists
	template<typename ... ListElements1, typename ... ListElements2>
	struct Concatenate < TypeList<ListElements1...>, TypeList<ListElements2...> > {

		using type = TypeList<ListElements1..., ListElements2...>;
	};

	template<typename ... ListElements1, typename ... ListElements2, typename ... Lists>
	struct Concatenate<TypeList<ListElements1...>, TypeList<ListElements2...>, Lists...> {
		using type = typename Concatenate<TypeList<ListElements1...>, Concatenate<TypeList<ListElements2...>, Lists...>>::type;
	};

	template<typename ... Lists>
	using ConcatenateLists = typename Concatenate<Lists...>::type;

	template<typename T, typename ... Ts>
	struct Reverser;

	template<typename T>
	struct Reverser<TypeList<T>> {
		using type = TypeList<T>;
	};

	template<typename T, typename ... Ts>
	struct Reverser< TypeList<T, Ts...> > {
		using type = ConcatenateLists< typename Reverser<TypeList<Ts...>>::type, TypeList<T> >;
	};

	template<typename T>
	using ReverseList = typename Reverser<T>::type;

	template<typename T, typename ... Ts>
	struct RepeatRemover;

	template<typename T>
	struct RepeatRemover<TypeList<T>> {
		using type = TypeList<T>;
	};

	template<typename T, typename ... Ts>
	struct RepeatRemover<TypeList<T, Ts...>> {
		using type = std::conditional_t<TypeList<Ts...>::template contains<T>(),
			typename RepeatRemover<TypeList<Ts...>>::type,
			ConcatenateLists<TypeList<T>, typename RepeatRemover<TypeList<Ts...>>::type>>;
	};

	template<typename ... Ts>
	using RemoveRepeats = typename RepeatRemover<Ts...>::type;

	template<typename List>
	struct PowersetComputer {
		using type = std::void_t<List>;
	};

	template<typename T>
	struct PowersetComputer<TypeList<T>> {
		using type = TypeList<TypeList<>, TypeList<T>>;
	};

	template<typename T, typename ... Ts>
	struct PowersetComputer<TypeList<T, Ts...>> {

		template<typename R>
		using PrependWithT = typename R::template prepend<T>;

		using SubsetsWithoutFirst = typename PowersetComputer<TypeList<Ts...>>::type;
		using SubsetsWithFirst = typename SubsetsWithoutFirst::template apply_to_each<PrependWithT>;

		using type = ConcatenateLists<SubsetsWithoutFirst, SubsetsWithFirst>;
	};

	template<typename List>
	using PowersetOf = RemoveRepeats<typename PowersetComputer<List>::type>;	


	template<typename List1, typename List2>
	struct ContainsAllHelper {
		constexpr static bool value = false;
	};

	template<typename ... Ts, typename ... Rs>
	struct ContainsAllHelper<TypeList<Ts...>, TypeList<Rs...>> {
		constexpr static bool value = TypeList<Ts...>::template contains_all<Rs...>();
	};

	template<typename L, typename R>
	constexpr static bool ContainsAll = ContainsAllHelper<L, R>::value;

}

#endif // TypeListHPP_HEADER_GUARD
