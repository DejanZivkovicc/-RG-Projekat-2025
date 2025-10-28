#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>

#include <MainController.hpp>
#include <spdlog/spdlog.h>
#include "GuiController.hpp"

namespace app {

constexpr int32_t gl_texture0_const = 0x84C0;

class MainPlatformEventObserver : public engine::platform::PlatformEventObserver {
public:
    void on_mouse_move(engine::platform::MousePosition position) override;
};

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    auto gui_controller = engine::core::Controller::get<GUIController>();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    if (gui_controller->is_enabled()) {
        platform->set_enable_cursor(true);
        return;
    }

    // tell GLFW to capture our mouse
    platform->set_enable_cursor(false);

    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    camera->rotate_camera(position.dx, position.dy, true);
}

void MainController::initialize() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
    engine::graphics::OpenGL::enable_depth_testing();

    // --------------------------------lighting--------------------------------
    m_light_pos1 = glm::vec3(-4.2f, 25.0f, -45.0f);
    m_light_pos2 = glm::vec3(-4.33f, -0.4f, -3.93f);
    m_light_pos3 = glm::vec3(-3.53f, -0.4f, -3.93f);

    m_sphere_color = glm::vec3(0.7f, 0.8f, 1.0f);
    m_light_cube_color = glm::vec3(1.0f, 1.0f, 0.0f);

    // --------------------------------floor--------------------------------
    graphics->initialize_floor();

    // --------------------------------grass--------------------------------
    graphics->initialize_grass();

    // --------------------------------lightCube--------------------------------
    SphereMesh lightCube = generate_sphere_mesh(1.0f, 36, 18);
    graphics->initialize_light_cube_mesh(lightCube.vertices, lightCube.indices);

    // --------------------------------sphere--------------------------------
    SphereMesh sphere = generate_sphere_mesh(1.0f, 36, 18);
    graphics->initialize_sphere_mesh(sphere.vertices, sphere.indices);

    // --------------------------------trees--------------------------------
    const float area_radius = 65.0f;     // Trees have to be in <75.0f area (floor area)
    const float y_level = -2.5f;         // Height of the floor

    // Restricted area (because of the Manor and StreetLamp objects)
    struct ForbiddenZone {
        glm::vec3 center;
        float radius;
    };

    std::vector<ForbiddenZone> noTreeZones = {
            // Coordinates of the Manor
            {glm::vec3(-7.0f, y_level, -10.0f),  8.0f},
            // Coordinates of the Street Lamp
            {glm::vec3(-3.93f, y_level, -3.93f), 4.0f},
    };

    // Matrix
    m_tree_model_matrices.resize(m_tree_amount);
    srand(time(NULL));

    unsigned int matrices_generated = 0;
    while (matrices_generated < m_tree_amount) {
        glm::mat4 model = glm::mat4(1.0f);

        // Generating random position withing the circle (floor height included)
        float random_angle = (rand() % 360) * 3.14159f / 180.0f;
        float random_radius = (rand() % (int) (area_radius * 100)) / 100.0f; // 0 to area_radius

        float x = cos(random_angle) * random_radius;
        float z = sin(random_angle) * random_radius;

        glm::vec3 potential_pos(x, y_level, z);

        // Checking collision between tree and other objects
        bool collision = false;
        for (const auto &zone: noTreeZones) {
            if (glm::distance(potential_pos, zone.center) < zone.radius) {
                collision = true;
                break;
            }
        }

        // If there is a collision, generate a new position
        if (collision) { continue; }

        // Translate (if there's no collision)
        model = glm::translate(model, potential_pos);

        // Have a random scale of the tree, between 0.8 and 1.2
        float scale = (rand() % 40) / 100.0f + 0.8f; // od 0.8 do 1.2
        model = glm::scale(model, glm::vec3(scale));

        // Have a random rotation around Y axis
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.0f, 1.0f, 0.0f));

        // Add generated matrix and continue
        m_tree_model_matrices[matrices_generated++] = model;
    }

    engine::resources::Model *tree = resources->model("Tree");
    graphics->setup_tree_instancing(m_tree_model_matrices, m_tree_amount, tree);
}

bool MainController::loop() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE)
                .is_down()) {
        return false;
    }
    return true;
}

void MainController::set_model_lighting(engine::resources::Shader *shader) {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    shader->set_vec3("viewPos", graphics->camera()
                                        ->Position);

    // Light 1 (moon)
    shader->set_vec3("light1.position", m_light_pos1);
    shader->set_vec3("light1.ambient", m_sphere_color * 0.25f);
    shader->set_vec3("light1.diffuse", m_sphere_color * 0.45f);
    shader->set_vec3("light1.specular", m_sphere_color * 0.2f);

    // Light 2 (lamp)
    shader->set_vec3("light2.position", m_light_pos2);
    shader->set_vec3("light2.ambient", m_light_cube_color * 0.05f);
    shader->set_vec3("light2.diffuse", m_light_cube_color * 0.13f);
    shader->set_vec3("light2.specular", m_light_cube_color * 0.04f);

    // Light 3 (lamp)
    shader->set_vec3("light2.position", m_light_pos3);
    shader->set_vec3("light2.ambient", m_light_cube_color * 0.05f);
    shader->set_vec3("light2.diffuse", m_light_cube_color * 0.13f);
    shader->set_vec3("light2.specular", m_light_cube_color * 0.04f);

    // Material
    shader->set_float("shininess", 16.0f);
}

void MainController::draw_manor() {
    // Model
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    engine::resources::Model *manor = resources->model("SpookyManor");

    // Shader
    engine::resources::Shader *shader = resources->shader("modelShader");
    shader->use();
    set_model_lighting(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-7.0f, -2.5f, -10.0f));
    model = glm::scale(model, glm::vec3(4.0f));
    shader->set_mat4("model", model);
    manor->draw(shader);
}

void MainController::draw_street_lamp() {
    // Model
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    engine::resources::Model *street_lamp = resources->model("StreetLamp");

    // Shader
    engine::resources::Shader *shader = resources->shader("modelShader");
    shader->use();
    set_model_lighting(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.93, -2.5f, -3.93));
    model = glm::scale(model, glm::vec3(0.0045f));
    shader->set_mat4("model", model);
    street_lamp->draw(shader);
}

void MainController::draw_tree() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    engine::resources::Model *tree = resources->model("Tree");

    engine::resources::Shader *shader = resources->shader("treeShader");
    shader->use();
    set_model_lighting(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());

    graphics->draw_instanced_model(tree, shader, m_tree_amount);
}

void MainController::draw_floor() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    engine::resources::Texture *texture = resources->texture("floor");

    engine::resources::Shader *shader = resources->shader("floorShader");
    shader->use();
    set_model_lighting(shader);

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());

    // Transform Model Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f)); // Position the texture in the scene
    model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
    shader->set_mat4("model", model);

    // Bind the texture
    texture->bind(gl_texture0_const);//(GL_TEXTURE0); // Binds texture to GL_TEXTURE0
    shader->set_int("texture1", 0); // shader has a uniform 'texture1'

    graphics->draw_floor();
}

void MainController::draw_grass() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    engine::resources::Texture *texture = resources->texture("grass");

    engine::resources::Shader *shader = resources->shader("grassShader");
    shader->use();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());

    // Bind the texture
    texture->bind(gl_texture0_const);//(GL_TEXTURE0); // Binds texture to GL_TEXTURE0
    shader->set_int("grassTexture", 0);

    std::vector<glm::vec3> vegetation{
            glm::vec3(-1.5f, -2.0f, -4.48f),
            glm::vec3(1.5f, -2.0f, 0.51f),
            glm::vec3(6.0f, -2.0f, 3.7f),
            glm::vec3(-2.3f, -2.0f, -7.3f),
            glm::vec3(5.5f, -2.0f, -5.6f),
            glm::vec3(2.5f, -2.0f, 5.6f),
            glm::vec3(3.7f, -2.0f, -3.6f),
            glm::vec3(-1.8f, -2.0f, 2.6f),
            glm::vec3(-5.0f, -2.0f, -7.6f),
            glm::vec3(0.0f, -2.0f, 0.0f)
    };

    for (unsigned int i = 0; i < vegetation.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, vegetation[i]); // Position the texture in the scene
        shader->set_mat4("model", model);

        graphics->draw_grass();
    }
}

void MainController::draw_skybox() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto skybox = resources->skybox("forest_skybox");

    auto shader = resources->shader("skybox");
    set_model_lighting(shader);
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::draw_light_cube() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    engine::resources::Shader *shader = resources->shader("lightCubeShader");
    shader->use();
    shader->set_vec3("lightCubeColor", m_light_cube_color);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());

    std::vector<glm::vec3> lightPos;
    lightPos.push_back(m_light_pos2);
    lightPos.push_back(m_light_pos3);
    for (int i = 0; i < lightPos.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, lightPos[i]);
        model = glm::scale(model, glm::vec3(0.05f));
        shader->set_mat4("model", model);

        graphics->draw_light_cube();
    }
}

void MainController::draw_sphere() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    engine::resources::Shader *shader = resources->shader("sphereShader");
    shader->use();
    shader->set_vec3("sphereColor", m_sphere_color);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_light_pos1);
    model = glm::scale(model, glm::vec3(5.0f)); // a smaller cube
    shader->set_mat4("model", model);


    graphics->draw_sphere();
}

void MainController::update_camera() {
    auto gui_controller = engine::core::Controller::get<GUIController>();
    if (gui_controller->is_enabled()) {
        return;
    }

    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    float dt = platform->dt();

    if (platform->key(engine::platform::KeyId::KEY_W)
                .is_down() || platform->key(engine::platform::KeyId::KEY_UP)
                                      .is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_S)
                .is_down() || platform->key(engine::platform::KeyId::KEY_DOWN)
                                      .is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_A)
                .is_down() || platform->key(engine::platform::KeyId::KEY_LEFT)
                                      .is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_D)
                .is_down() || platform->key(engine::platform::KeyId::KEY_RIGHT)
                                      .is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
    }

    if (platform->key(engine::platform::KeyId::KEY_1)
                .is_down()) {
        m_sphere_color = glm::vec3(0.7f, 0.8f, 1.0f);
    }
    if (platform->key(engine::platform::KeyId::KEY_2)
                .is_down()) {
        m_sphere_color = glm::vec3(0.541f, 0.0118f, 0.0118f);
    }

    if (platform->key(engine::platform::KeyId::KEY_3)
                .is_down()) {
        m_light_cube_color = glm::vec3(1.0f, 1.0f, 0.0f);
    }
    if (platform->key(engine::platform::KeyId::KEY_4)
                .is_down()) {
        m_light_cube_color = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    if (platform->key(engine::platform::KeyId::KEY_5)
                .is_down()) {
        m_light_cube_color = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

MainController::SphereMesh MainController::generate_sphere_mesh(float radius, unsigned int sectors,
                                                                unsigned int stacks) {
    SphereMesh mesh;

    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    for (unsigned int i = 0; i <= stacks; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            mesh.vertices
                .push_back(x);
            mesh.vertices
                .push_back(y);
            mesh.vertices
                .push_back(z);
        }
    }
   
    // Generating the indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                mesh.indices
                    .push_back(k1);
                mesh.indices
                    .push_back(k2);
                mesh.indices
                    .push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                mesh.indices
                    .push_back(k1 + 1);
                mesh.indices
                    .push_back(k2);
                mesh.indices
                    .push_back(k2 + 1);
            }
        }
    }
    return mesh;
}

void MainController::update() {
    update_camera();
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    // clear buffer (color buffer, depth buffer)
    draw_manor();
    draw_street_lamp();
    draw_tree();
    draw_floor();
    draw_grass();
    draw_skybox();
    draw_light_cube();
    draw_sphere();
    // swapBuffers
}

void MainController::terminate() {
    engine::core::Controller::get<engine::resources::ResourcesController>()->model("Tree")
                                                                           ->destroy();
    engine::core::Controller::get<engine::resources::ResourcesController>()->model("StreetLamp")
                                                                           ->destroy();
    engine::core::Controller::get<engine::resources::ResourcesController>()->model("SpookyManor")
                                                                           ->destroy();
}

void MainController::end_draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

} // app