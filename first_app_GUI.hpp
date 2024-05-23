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

namespace lve {
	class FirstAppGUI {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstAppGUI();
		~FirstAppGUI();

		FirstAppGUI(const FirstAppGUI&) = delete;
		FirstAppGUI& operator=(const FirstAppGUI&) = delete;

		void run();

	private:
		void loadGameObjects();

		LveWindow lveWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice };

		// note: order of declarations matters
		std::unique_ptr<LveDescriptorPool> globalPool{};
		LveGameObject::Map gameObjects;
		VkDescriptorPool imguiPool;
		ImGuiIO io;

		void init_imgui();
	};
}