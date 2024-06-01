#pragma once

#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"

// std
#include <memory>
#include <vector>

#include "dependencies/imgui.h"
#include "dependencies/imgui_impl_glfw.h"
#include "dependencies/imgui_impl_vulkan.h"

#include "dependencies/entt.hpp"

namespace lve {
	class Scene {
	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;

		Scene();
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		std::shared_ptr<entt::entity> loadObj(const std::string& filepath, const std::string& objName, glm::vec3 color = {0.f,0.f,0.f});
		std::shared_ptr<entt::entity> makePointLightObj(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f), const std::string& objName="NoName");
		std::shared_ptr<entt::entity> makeCameraObj(glm::vec3 initialPosition = { 0.f,0.f,0.f }, const std::string& objName = "camera");
		template <typename ComponentType>
		ComponentType* getComponent(entt::entity entity);
		
		
		std::map<std::string, entt::entity> entityMap;

		void run();
		

	private:
		void loadGameObjects();

		LveWindow lveWindow{ WIDTH, HEIGHT, "Scene Editor!" };
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };

		// note: order of declarations matters
		std::unique_ptr<LveDescriptorPool> globalPool{};
		LveGameObject::Map gameObjects;

		entt::registry registry;
		std::shared_ptr<entt::entity> selectedEntity = nullptr;


		VkDescriptorPool imguiPool;
		ImGuiIO io;

		void init_imgui();
	};
}