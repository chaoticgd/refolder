#include <string>
#include <iostream>

#include "refolder.h"

struct person {

	std::string name;
	int age;
	float height;

	float get_height() { return height; }
	void set_height(float h) {
		if(h >= 0) {
			height = h;
		} else {
			std::cout << "A person's height cannot be negative.\n";
		}
	}

	template <typename... T>
	void reflect(T... callbacks) {
		rf::reflector r(this, callbacks...);
		r.visit_r("First Name", name);
		r.visit_f("Age",
			[=]() { return age; },
			[=](int a) { age = a; });
		r.visit_m("Height", &person::get_height, &person::set_height);
	}
};

int main() {
	person p;
	p.name = "John Smith";
	p.age = 42;
	p.height = 1.8;
	p.reflect(
		[](const char* name, rf::property<int> p) {
			std::cout << "Property \"" << name << "\" of type int with value \"" << p.get() << "\".\n";
			if(std::string(name) == "Age") {
				p.set(p.get() + 1);
			}
		},
		[](const char* name, rf::property<float> p) {
			std::cout << "Property \"" << name << "\" of type float with value \"" << p.get() << "\".\n";
		},
		[](const char* name, rf::property<std::string> p) {
			std::cout << "Property \"" << name << "\" of type string with string \"" << p.get() << "\".\n";
		}
	);
	std::cout << "John's new age is " << p.age << ".\n";
}
