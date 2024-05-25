#pragma once

#include "scene.hpp"

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
		Scene scene{};
	};
}  // namespace lve
