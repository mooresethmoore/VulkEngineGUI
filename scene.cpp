#include "scene.hpp"


#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

#include "dependencies/imgui.h"
#include "dependencies/imgui_impl_glfw.h"
#include "dependencies/imgui_impl_vulkan.h"



static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

namespace lve {

    void Scene::init_imgui() {
        // 1: create descriptor pool for IMGUI
        //  the size of the pool is very oversize, but it's copied from imgui demo
        //  itself.

        //TODO fix this, and possibly have this managed in the pool allocator
        VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        //VkDescriptorPool imguiPool;
        if (vkCreateDescriptorPool(lveDevice.device(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind imgui descriptor pool!");
        }


        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends


        ImGui_ImplGlfw_InitForVulkan(lveWindow.getGLFWwindow(), true);
        //ImGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Vulkan);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = lveDevice.getInstance();
        init_info.PhysicalDevice = lveDevice.getPhysicalDevice();
        init_info.Device = lveDevice.device();
        init_info.Queue = lveDevice.graphicsQueue();

        //queuefamily from 
        QueueFamilyIndices indices = lveDevice.findPhysicalQueueFamilies();
        init_info.QueueFamily = indices.graphicsFamily;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;


        /* use pre-existing renderpass*/
        // not sure if the Subpass is always 0
        init_info.RenderPass = lveRenderer.getSwapChainRenderPass();
        init_info.Subpass = 0;

        /*
        init_info.UseDynamicRendering = true;

        //dynamic rendering parameters for imgui to use
        init_info.PipelineRenderingCreateInfo = {};
        init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        //VkFormat imFormat =  // this method may be flawed if imformat changes after init of imgui
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = lveRenderer.getSwapChainImageFormatPtr();
        */

        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = check_vk_result;

        ImGui_ImplVulkan_Init(&init_info);

        //io.Fonts->AddFontDefault();
        io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 18.0f);

        ImGui_ImplVulkan_CreateFontsTexture();

        /* add the destroy the imgui created structures
        _mainDeletionQueue.push_function([=]() {
            ImGui_ImplVulkan_Shutdown();
            vkDestroyDescriptorPool(_device, imguiPool, nullptr);
            });*/
    }

    Scene::Scene() {
        globalPool =
            LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        //loadGameObjects();
        loadGameObjects2();
        init_imgui();
    }

    Scene::~Scene() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        //definitely don't like this, probably best to just have some form of wrapper class for imgui
        //or some generalized deletionqueue
        vkDestroyDescriptorPool(lveDevice.device(), imguiPool, nullptr);
    }

    void Scene::run() {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };

        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        bool show_demo_window = true; //imgui demo
        bool show_another_window = true; //imgui demo
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();


                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();



                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);


                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", frameTime * 1000, 1.0 / frameTime);
                    ImGui::End();
                }
                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);


                //let's try a simple inspector window here
                // then we will work on constructing an ECS, and writing specific subroutines for displaying &
                // modifying relevant params within each gameobject in the registry, based on components

                ImGui::Render();

                



                //order matters
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);


                ImDrawData* main_draw_data = ImGui::GetDrawData();
                const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);

                ImGui_ImplVulkan_RenderDrawData(main_draw_data, lveRenderer.getCurrentCommandBuffer());


                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void Scene::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel =
            LveModel::createModelFromFile(lveDevice, "models/dice.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = { -.5f, .5f, 0.f };
        flatVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
        auto smoothVase = LveGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        smoothVase.color = { 1.f,0.f,0.f }; //not getting renderered
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };
        gameObjects.emplace(floor.getId(), std::move(floor));

        float lightIntensity = 0.2f;
        // auto pointLight = LveGameObject::makePointLight(lightIntensity);
         //gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        std::vector<glm::vec3> lightColors{
             {1.f, .1f, .1f},
             {.1f, .1f, 1.f},
             {.1f, 1.f, .1f},
             {1.f, 1.f, .1f},
             {.1f, 1.f, 1.f},
             {1.f, 1.f, 1.f}  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = LveGameObject::makePointLight(lightIntensity);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f,-1.f,0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

    }

    std::shared_ptr<entt::entity> Scene::loadObj(const std::string& filepath, const std::string& objName, glm::vec3 color){
        std::shared_ptr<LveModel> lveModel =
            LveModel::createModelFromFile(lveDevice, filepath);
        entt::entity gameObj = registry.create();
        // integrate a map<name,gameObj>
        entityMap[objName] = gameObj; // not checking for name overlap for now, don't do it
        registry.emplace_or_replace<std::shared_ptr<LveModel>>(gameObj,lveModel); //idk if *lveModel or lveModel is right here

        registry.emplace<TransformComponent>(gameObj);
        //auto& transform = registry.emplace<TransformComponent>(gameObj);
        //here we could include initial conditions of the transform, 
        //
        auto &colorComponent = registry.emplace<ColorComponent>(gameObj);
        colorComponent.color = color;

        return std::make_shared<entt::entity>(gameObj);
    }

    std::shared_ptr<entt::entity> Scene::makePointLightObj(float intensity, float radius, glm::vec3 color, const std::string& objName) {
        entt::entity gameObj = registry.create();
        entityMap[objName] = gameObj; // not checking for name overlap for now, don't do it
        auto& transform = registry.emplace<TransformComponent>(gameObj);

        transform.scale.x = radius; //notational -- handled in point_light render 

        auto& pointLightComp = registry.emplace<PointLightComponent>(gameObj);
        pointLightComp.lightIntensity = intensity;

        auto& colorComponent = registry.emplace<ColorComponent>(gameObj);
        colorComponent.color = color;

        return std::make_shared<entt::entity>(gameObj);
    }

    std::shared_ptr<entt::entity> Scene::makeCameraObj(glm::vec3 initialPosition, const std::string& objName) {
        entt::entity gameObj = registry.create();
        entityMap[objName] = gameObj; // not checking for name overlap for now, don't do it
        auto& transform = registry.emplace<TransformComponent>(gameObj);
        transform.translation = initialPosition;
        auto& cameraController = registry.emplace<KeyboardMovementController>(gameObj);
        
        auto& camera = registry.emplace<LveCamera>(gameObj);

        return std::make_shared<entt::entity>(gameObj);

    }

    template <typename ComponentType> 
    ComponentType* Scene::getComponent(entt::entity entity) {
        /*auto* componentPtr = registry.try_get<ComponentType>(entity);
        if (componentPtr) {
            return std::shared_ptr<ComponentType>{componentPtr};
        }
        else {
            return nullptr;
        }*/
        return registry.try_get<ComponentType>(entity);
    }

    void Scene::loadGameObjects2() {
        
        auto flatVase = loadObj("models/dice.obj", "flatVase");
        
        {
            auto transform = getComponent<TransformComponent>(*flatVase); // smrt ptr to component
            //auto transform = registry.try_get<TransformComponent>(*flatVase);
            if (transform) {
                transform->translation = { -.5f, .5f, 0.f };
                transform->scale = { 3.f, 1.5f, 3.f };
            }
            
        }
        
        auto smoothVase = loadObj("models/smooth_vase.obj", "smoothVase");

        {
            auto transform = getComponent<TransformComponent>(*smoothVase);
            if (transform) {
                transform->translation = { .5f, .5f, 0.f };
                transform->scale = { 3.f, 1.5f, 3.f };
            }

        }

        auto floor = loadObj("models/quad.obj", "floor");
        
        {
            auto transform = getComponent<TransformComponent>(*floor);
            if (transform) {
                transform->translation = { 0.f, .5f, 0.f };
                transform->scale = { 3.f, 1.f, 3.f };
            }

        }
        
        float lightIntensity = 0.2f;
        float lightRadius = .1f;
        std::vector<glm::vec3> lightColors{
             {1.f, .1f, .1f},
             {.1f, .1f, 1.f},
             {.1f, 1.f, .1f},
             {1.f, 1.f, .1f},
             {.1f, 1.f, 1.f},
             {1.f, 1.f, 1.f}  //
        };
        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = makePointLightObj(lightIntensity,lightRadius,lightColors[i],"light"+std::to_string(i));
            
            auto rotateLight = glm::rotate(glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f,-1.f,0.f });

            auto transform = getComponent<TransformComponent>(*pointLight);
            if (transform) {
                transform->translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            }
        }


        
    }

    void Scene::run2() {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };

        auto cameraObj = makeCameraObj({ 0.f,0.f,-2.5f }, "camera");
        auto cameraPtr = getComponent<LveCamera>(*cameraObj);
        auto cameraController = getComponent<KeyboardMovementController>(*cameraObj);
        bool show_demo_window = true; //imgui demo
        bool show_another_window = true; //imgui demo

        bool show_render_window = true; 

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            auto transform = (getComponent<TransformComponent>(*cameraObj));
            cameraController->moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, *transform);
            cameraPtr->setViewYXZ(transform->translation, transform->rotation);
            float aspect = lveRenderer.getAspectRatio();
            cameraPtr->setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo2 frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    *cameraPtr,
                    globalDescriptorSets[frameIndex],
                    &registry };

                // update
                GlobalUbo ubo{};
                ubo.projection = cameraPtr->getProjection();
                ubo.view = cameraPtr->getView();
                ubo.inverseView = cameraPtr->getInverseView();

                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);


                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                
                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
                if(show_render_window)
                {

                    ImGui::Begin("Renderer (I barely know her)");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

                    
                    for (auto& kv : entityMap) {
                        const std::string& name = kv.first;
                        //ImGui::CollapsingHeader("Inputs & Focus")
                        //ImGui::TreeNode(name.c_str())
                        if (name!="" && ImGui::CollapsingHeader(name.c_str())) {
                            //search the way we did before, 
                            // through all relevant component prototypes that have imgui_editor implementation
                            auto transform = getComponent<TransformComponent>(kv.second);
                            if (transform) {
                                transform->imgui_editor();
                            }
                            auto color = getComponent<ColorComponent>(kv.second);
                            if (color) {
                                color->imgui_editor();

                            }
                            //ImGui::TreePop();
                        }
                    }



                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", frameTime * 1000, 1.0 / frameTime);
                    ImGui::End();
                }
                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);


                //let's try a simple inspector window here
                // then we will work on constructing an ECS, and writing specific subroutines for displaying &
                // modifying relevant params within each gameobject in the registry, based on components

                ImGui::Render();





                //order matters
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);


                ImDrawData* main_draw_data = ImGui::GetDrawData();
                const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);

                ImGui_ImplVulkan_RenderDrawData(main_draw_data, lveRenderer.getCurrentCommandBuffer());


                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }
}  // namespace lve
