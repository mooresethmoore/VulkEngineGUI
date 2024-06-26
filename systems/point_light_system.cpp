#include "point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

namespace lve {

    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(
        LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : lveDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        LvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "shaders/point_light.vert.spv",
            "shaders/point_light.frag.spv",
            pipelineConfig);
    }


    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
        auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
        int lightIndex = 0;
        auto view = frameInfo.registry->view < PointLightComponent, TransformComponent, ColorComponent > ();
        for (auto entity: view) {
            

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");
            TransformComponent& transform = view.get<TransformComponent>(entity);
            // update light position
            transform.translation = glm::vec3(rotateLight * glm::vec4(transform.translation, 1.f));

            PointLightComponent& pointLight = view.get<PointLightComponent>(entity);

            // copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(view.get<ColorComponent>(entity).color, pointLight.lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo& frameInfo) {
        //sort semitransparent
        std::map<float, entt::entity> sorted;
        auto view = frameInfo.registry->view < PointLightComponent, TransformComponent>();
        for (auto entity : view) {
            
            TransformComponent& transform = view.get<TransformComponent>(entity);
            
            auto offset = frameInfo.camera.getPosition() - transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = entity;

        }


        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        //iterate through sorted lights in reverse order
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {//for (auto& kv : frameInfo.gameObjects) {
            entt::entity entity = it->second;
            //if (obj.pointLight == nullptr) continue;
            TransformComponent& transform = frameInfo.registry->get<TransformComponent>(entity);
            ColorComponent& colorComp = frameInfo.registry->get<ColorComponent>(entity);
            PointLightComponent& pointLight = frameInfo.registry->get<PointLightComponent>(entity);
            PointLightPushConstants push{};
            push.position = glm::vec4(transform.translation, 1.f);
            push.color = glm::vec4(colorComp.color, pointLight.lightIntensity);
            push.radius = transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }

    
    }

}  // namespace lve
