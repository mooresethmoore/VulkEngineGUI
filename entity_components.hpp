#pragma once

#include <glm/gtc/matrix_transform.hpp>


namespace lve {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();

        glm::mat3 normalMatrix();


        void imgui_editor();
    };




    struct PointLightComponent {
        float lightIntensity = 1.0f;



       // void imgui_editor();
    };


    struct ColorComponent{
        glm::vec3 color{};



        //void imgui_editor();
    };


    struct CameraComponent {
        //for now I won't use this
        bool useProsective = true;
        

    };



}