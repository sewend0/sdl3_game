

#ifndef SDL3_GAME_OBJECTS_H
#define SDL3_GAME_OBJECTS_H

#include <Components.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

// Base component owning object interface
class Game_object {
private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

public:
    template <typename T, typename... Args>
    auto add_component(Args&&... args) -> T* {
        auto component{std::make_unique<T>(std::forward<Args>(args)...)};
        T* ptr{component.get()};
        components[std::type_index(typeid(T))] = std::move(component);
        return ptr;
    }

    template <typename T>
    auto get_component() -> T* {
        const auto comp{components.find(std::type_index(typeid(T)))};
        return comp != components.end() ? static_cast<T*>(comp->second.get()) : nullptr;
    }
};

#endif    // SDL3_GAME_OBJECTS_H
