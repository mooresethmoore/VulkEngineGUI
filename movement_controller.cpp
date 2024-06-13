#include "movement_controller.hpp"

namespace lve {
    MovementController::MovementController(GLFWwindow* window) {
        // Initialize KeyInput with the keys we want to monitor
        std::vector<int> keysToMonitor = {
            keys.keyMap["moveLeft"], keys.keyMap["moveRight"], keys.keyMap["moveForward"],
            keys.keyMap["moveBackward"], keys.keyMap["moveUp"], keys.keyMap["moveDown"],
            keys.keyMap["lookLeft"], keys.keyMap["lookRight"], keys.keyMap["lookUp"], keys.keyMap["lookDown"]
        };
        keyInput = std::make_unique<KeyInput>(keysToMonitor);

        // Set up KeyInput with the GLFW window
        KeyInput::setupKeyInputs(window);
    }

    void MovementController::moveInPlaneXZ(float dt, LveGameObject& gameObject) {
        glm::vec3 rotate{ 0.0f };
        if (keyInput->getIsKeyDown(keys.keyMap["lookRight"])) rotate.y += 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookLeft"])) rotate.y -= 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookUp"])) rotate.x += 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookDown"])) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f); // limit pitch values about +-85ish degrees (don't go upside down) 
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>()); // don't overflow

        // Which direction?
        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw),0.f,cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z,0.f,forwardDir.x };
        const glm::vec3 upDir{ 0.f,-1.f,0.f };

        glm::vec3 moveDir{ 0.f };
        if (keyInput->getIsKeyDown(keys.keyMap["moveRight"])) moveDir += rightDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveLeft"])) moveDir -= rightDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveUp"])) moveDir += upDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveDown"])) moveDir -= upDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveForward"])) moveDir += forwardDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveBackward"])) moveDir -= forwardDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

    void MovementController::moveInPlaneXZ(float dt, TransformComponent& transform) {
        glm::vec3 rotate{ 0.0f };
        if (keyInput->getIsKeyDown(keys.keyMap["lookRight"])) rotate.y += 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookLeft"])) rotate.y -= 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookUp"])) rotate.x += 1.f;
        if (keyInput->getIsKeyDown(keys.keyMap["lookDown"])) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f); // limit pitch values about +-85ish degrees (don't go upside down) 
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>()); // don't overflow

        // Which direction?
        float yaw = transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw),0.f,cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z,0.f,forwardDir.x };
        const glm::vec3 upDir{ 0.f,-1.f,0.f };

        glm::vec3 moveDir{ 0.f };
        if (keyInput->getIsKeyDown(keys.keyMap["moveRight"])) moveDir += rightDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveLeft"])) moveDir -= rightDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveUp"])) moveDir += upDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveDown"])) moveDir -= upDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveForward"])) moveDir += forwardDir;
        if (keyInput->getIsKeyDown(keys.keyMap["moveBackward"])) moveDir -= forwardDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}
