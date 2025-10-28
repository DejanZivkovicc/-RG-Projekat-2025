#ifndef MATF_RG_PROJECT_MAINCONTROLLER_HPP
#define MATF_RG_PROJECT_MAINCONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/resources/Shader.hpp>
#include <glm/glm.hpp>

namespace app {

class MainController : public engine::core::Controller {
    void initialize() override;

    bool loop() override;

    void draw_manor();

    void draw_street_lamp();

    void draw_tree();

    void set_model_lighting(engine::resources::Shader *shader);

    void draw_floor();

    void draw_grass();

    void draw_skybox();

    void draw_light_cube();

    void draw_sphere();

    void update_camera();

    void update() override;

    void begin_draw() override;

    void draw() override;

    void end_draw() override;

    void terminate() override;

private:
    unsigned int m_tree_amount{100};
    std::vector<glm::mat4> m_tree_model_matrices;

    // Lighting (moon + 2 light cubes)
    glm::vec3 m_light_pos1;
    glm::vec3 m_light_pos2;
    glm::vec3 m_light_pos3;

    // Lightning colors
    glm::vec3 m_sphere_color;
    glm::vec3 m_light_cube_color;

public:
    std::string_view name() const override {
        return "app::MainController";
    }

    struct SphereMesh {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
    };

    SphereMesh generate_sphere_mesh(float radius, unsigned int sectors, unsigned int stacks);

};
} // app
#endif //MATF_RG_PROJECT_MAINCONTROLLER_HPP
