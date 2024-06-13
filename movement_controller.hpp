#pragma once

#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include "entity_components.hpp"
#include "KeyInput.hpp"  // Include the KeyInput header
#include <memory>      // For std::unique_ptr
#include <map>
#include <string>

namespace lve {
    class MovementController {
    public:
        struct KeyMappings {
            std::map<std::string, int> keyMap;

            KeyMappings() {
                keyMap["moveLeft"] = GLFW_KEY_A;
                keyMap["moveRight"] = GLFW_KEY_D;
                keyMap["moveForward"] = GLFW_KEY_W;
                keyMap["moveBackward"] = GLFW_KEY_S;
                keyMap["moveUp"] = GLFW_KEY_E;
                keyMap["moveDown"] = GLFW_KEY_Q;
                keyMap["lookLeft"] = GLFW_KEY_LEFT;
                keyMap["lookRight"] = GLFW_KEY_RIGHT;
                keyMap["lookUp"] = GLFW_KEY_UP;
                keyMap["lookDown"] = GLFW_KEY_DOWN;
            }
        };

        MovementController(GLFWwindow* window);
        void moveInPlaneXZ(float dt, LveGameObject& gameObject);
        void moveInPlaneXZ(float dt, TransformComponent& transform);

        KeyMappings keys;
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.5f };

    private:
        std::unique_ptr<KeyInput> keyInput;
    };
}
