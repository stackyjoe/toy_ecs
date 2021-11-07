#pragma once

#include <concepts>

namespace jl {
	template<class T>
	concept container_model = requires(T t) {
		typename T::held_object;
		typename T::iterator;
		typename T::const_iterator;
		{ t.create_new() } -> std::same_as<typename T::held_object*>;
		{ t.begin() } -> std::same_as<typename T::iterator>;
		{ t.begin() } -> std::same_as<typename T::const_iterator>;
		{ t.end() } -> std::same_as<typename T::const_iterator>;
	};
}
