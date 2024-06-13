#pragma once

#include "sceneEditor.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
	class FirstAppGUI {
	public:

		FirstAppGUI();
		~FirstAppGUI();

		FirstAppGUI(const FirstAppGUI&) = delete;
		FirstAppGUI& operator=(const FirstAppGUI&) = delete;

		void run();

	private:
		SceneEditor scene{};
	};
}  // namespace lve
