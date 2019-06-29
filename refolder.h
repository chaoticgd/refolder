/*
	Copyright (c) 2019 Thomas Evans

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef LIB_REFOLDER_H
#define LIB_REFOLDER_H

#include <tuple>
#include <functional>

namespace rf {
	template <typename T_value>
	struct property {
		std::function<T_value()> get;
		std::function<void(T_value)> set;
	};

	// A static reflection mechanism.
	//
	// For each call to visit_r, visit_m or visit_f, match the given property
	// with a callback of the correct type that was passed to the constructor,
	// if one exists.
	template <typename T_object, typename... T_callbacks>
	class reflector {
	public:
		reflector(T_object* that, T_callbacks... callbacks)
			: _that(that), _callbacks(callbacks...) {}

		template <typename T_value>
		void visit_r(const char* name, T_value& value) {
			// Wrap the reference in a getter and setter.
			visit_f(name,
				[&value]() { return value; },
				[&value](T_value v) { value = v; });
		}

		template <typename T_getter, typename T_setter>
			// where T_getter, T_setter are member function pointers.
		void visit_m(const char* name, T_getter get, T_setter set) {
			T_object* that = _that; // By value.
			// Convert the getter and setter to functors.
			visit_f(name,
				[=]() { return (*that.*get)(); },
				[=](decltype((*that.*get)()) value)
					{ (*that.*set)(value); });
		}

		template <typename T_getter, typename T_setter>
			// where T_getter, T_setter are functors.
		void visit_f(const char* name, T_getter get, T_setter set) {
			visit_n<T_getter, T_setter, 0>(name, get, set);
		}

	private:
		template <typename T_getter, typename T_setter, std::size_t T_index>
		void visit_n(const char* name, T_getter get, T_setter set) {
			call(name,
				std::function(get),
				std::function(set),
				std::function(std::get<T_index>(_callbacks)));
			if constexpr(std::tuple_size_v<decltype(_callbacks)> > T_index + 1) {
				visit_n<T_getter, T_setter, T_index + 1>(name, get, set);
			}
		}

		// Match a callback to a getter and setter.
		template <typename T_value>
		void call(
			const char* name,
			std::function<T_value()> get,
			std::function<void(T_value)> set,
			std::function<void(const char*, property<T_value>)> callback) {
			
			using value_type = decltype(get());
			property<value_type> p { get, set };
			callback(name, p);
		}

		// SFINAE fallthrough.
		template <typename T_getter, typename T_setter, typename T_callback>
		void call(const char* name, T_getter get, T_setter set, T_callback callback) {}

		T_object* _that;
		std::tuple<T_callbacks...> _callbacks;
	};
}

#endif
