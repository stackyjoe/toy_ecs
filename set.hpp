#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>


namespace jl {

    struct AddressGetter {
        // TY to D. Wright, B. Bi, and L. Szonolki for this one
        template<class T>
        T* operator()(T& t) {
            return std::addressof<T>(t);
        }

    };

    inline constexpr auto address_of = AddressGetter{};

    namespace containers {
        template<class T>
        struct set {
        private:
            std::vector<std::unique_ptr<T>> dynamic_array;


        public:
            using iterator = typename std::vector<std::unique_ptr<T>>::iterator;
            using const_iterator = typename std::vector<std::unique_ptr<T>>::const_iterator;

            iterator begin() noexcept {
                return dynamic_array.begin();
            }

            const_iterator begin() const noexcept {
                return dynamic_array.begin();
            }

            iterator end() noexcept {
                return dynamic_array.end();
            }

            const_iterator end() const noexcept {
                return dynamic_array.end();
            }

            iterator find(T * t) noexcept {
                // Generic implementation of std::lower_bound
                iterator first = dynamic_array.begin();
                iterator it = first;
                iterator last = dynamic_array.end();
                typename std::iterator_traits<iterator>::difference_type count, step;
                count = std::distance(first, last);

                while (count > 0) {
                    it = first;
                    step = count / 2;
                    std::advance(it, step);
                    if (it->get() < t) {
                        first = ++it;
                        count -= step + 1;
                    }
                    else
                        count = step;
                }

                if (first != last && first->get() == t)
                    return first;

                return last;
            }

            const_iterator find(T const *t) {
                // Generic implementation of std::lower_bound
                const_iterator first = dynamic_array.begin();
                const_iterator last = dynamic_array.end();
                const_iterator it = first;
                typename std::iterator_traits<const_iterator>::difference_type count, step;
                count = std::distance(first, last);

                while (count > 0) {
                    it = first;
                    step = count / 2;
                    std::advance(it, step);
                    if (it->get() < t) {
                        first = ++it;
                        count -= step + 1;
                    }
                    else
                        count = step;
                }

                if (first != last && first->get() == t)
                    return first;

                return last;
            }

            void reserve(size_t capacity) {
                dynamic_array.reserve(capacity);
            }

            // Envisioned use
            // auto widgets = all_satisfying([](T const &t) -> {
            //     return t.is_red() & t.is_sphere();
            // });
            // widgets is a vector with pointers to all spherical red objects in the set.
            template<class Predicate, class Operation = AddressGetter>
            auto all_satisfying(Predicate p,
                Operation o = address_of)
                noexcept(noexcept(p(std::declval<T const&>())) && noexcept(o(std::declval<T&>())))
                -> std::vector<decltype(o(std::declval<T>()))> {

                std::vector<decltype(o(std::declval<T&>()))> desiderata;

                desiderata.reserve(dynamic_array.size());

                for (auto& value : dynamic_array) {
                    if (p(value)) {
                        desiderata.push_back(o(value));
                    }
                }

                return desiderata;
            }

            T * create_new() {
                auto t = std::make_unique<T>();
                auto it = find(t.get());
                if ((it == dynamic_array.end()) || (it->get() < t.get())) {
                    return dynamic_array.insert(it, std::move(t))->get();
                }

                throw std::runtime_error("lolwut");
            }

            template<class ... ConstructorArgs>
            void emplace(ConstructorArgs && ... arguments) {
                insert(std::make_unique<T>((static_cast<ConstructorArgs&&>(arguments), ...)));
            }

            void insert_ordered_unique(std::vector<T>&& ordered_unique_element_vector) {
                auto old_size = dynamic_array.size();

                dynamic_array.reserve(dynamic_array.size() + ordered_unique_element_vector.size());
                dynamic_array.insert(dynamic_array.end(),
                    std::make_move_iterator(ordered_unique_element_vector.begin()),
                    std::make_move_iterator(ordered_unique_element_vector.end()));

                std::inplace_merge(dynamic_array.begin(), dynamic_array.begin() + old_size, dynamic_array.end());
            }

            void insert(std::vector<T>&& unordered_or_repeat_element_vector) {
                std::sort(unordered_or_repeat_element_vector.begin(), unordered_or_repeat_element_vector.end());
                std::unique(unordered_or_repeat_element_vector.begin(), unordered_or_repeat_element_vector.end());

                insert_ordered_unique(std::move(unordered_or_repeat_element_vector));
            }

            bool empty() const noexcept {
                return dynamic_array.empty();
            }

            void remove(iterator it) {
                dynamic_array.erase(it);
            }

            void remove(T * value) {
                dynamic_array.erase(std::lower_bound(dynamic_array.begin(),
                    dynamic_array.end(),
                    std::make_unique<T>(nullptr),
                    [](auto const &lhs, auto const &rhs){
                        return lhs.get() < rhs.get();
                    }));
            }

            void destroy_and_deallocate(T* value) {
                auto itr = std::lower_bound();
            }

            void clear() noexcept(noexcept(std::declval<T>().~T())) {
                dynamic_array.clear();
            }

        };
    }
}