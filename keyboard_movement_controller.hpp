#pragma once

#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include "entity_components.hpp"


//https://www.glfw.org/docs/3.3/input_guide.html 

namespace lve {

	class KeyboardMovementController {

	public:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};

		void moveInPlaneXZ(GLFWwindow* window, float dt, LveGameObject& gameObject);
		void moveInPlaneXZ(GLFWwindow* window, float dt, TransformComponent& transform);

		KeyMappings keys{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
	};
}