
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Skybox.hpp>

namespace engine::graphics {

namespace {
float floorVertices[] = {
        // positions              // texture Coords     // Normal Coords
        15.0f, -2.5f, 15.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -15.0f, -2.5f, 15.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -15.0f, -2.5f, -15.0f, 0.0f, 2.0f, 0.0f, 1.0f, 0.0f,

        15.0f, -2.5f, 15.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -15.0f, -2.5f, -15.0f, 0.0f, 2.0f, 0.0f, 1.0f, 0.0f,
        15.0f, -2.5f, -15.0f, 2.0f, 2.0f, 0.0f, 1.0f, 0.0f
};

float grassVertices[] = {
        // positions         // texture Coords
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f, 0.0f
};
}

void GraphicsController::initialize_floor() {
    glGenVertexArrays(1, &m_floor_vao);
    glGenBuffers(1, &m_floor_vbo);

    glBindVertexArray(m_floor_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_floor_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute (3 floats)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void GraphicsController::initialize_grass() {
    glGenVertexArrays(1, &m_grass_vao);
    glGenBuffers(1, &m_grass_vbo);

    glBindVertexArray(m_grass_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_grass_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void GraphicsController::initialize_light_cube_mesh(const std::vector<float> &vertices,
                                                    const std::vector<unsigned int> &indices) {
    glGenVertexArrays(1, &m_light_cube_vao);
    glGenBuffers(1, &m_light_cube_vbo);
    glGenBuffers(1, &m_light_cube_ebo);

    glBindVertexArray(m_light_cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_light_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_light_cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    m_light_cube_index_count = indices.size();
}

void GraphicsController::initialize_sphere_mesh(const std::vector<float> &vertices,
                                                const std::vector<unsigned int> &indices) {
    glGenVertexArrays(1, &m_sphere_vao);
    glGenBuffers(1, &m_sphere_vbo);
    glGenBuffers(1, &m_sphere_ebo);

    glBindVertexArray(m_sphere_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    m_sphere_index_count = indices.size();
}

void GraphicsController::setup_tree_instancing(const std::vector<glm::mat4> &modelMatrices, unsigned int amount,
                                               engine::resources::Model *treeModel) {
    glGenBuffers(1, &m_tree_instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_tree_instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    treeModel->add_instance_vbo(m_tree_instance_vbo);
}

void GraphicsController::draw_floor() {
    glBindVertexArray(m_floor_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GraphicsController::draw_grass() {
    glBindVertexArray(m_grass_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GraphicsController::draw_light_cube() {
    glBindVertexArray(m_light_cube_vao);
    glDrawElements(GL_TRIANGLES, m_light_cube_index_count, GL_UNSIGNED_INT, 0);
}

void GraphicsController::draw_sphere() {
    glBindVertexArray(m_sphere_vao);
    glDrawElements(GL_TRIANGLES, m_sphere_index_count, GL_UNSIGNED_INT, 0);
}

void GraphicsController::draw_instanced_model(engine::resources::Model *model, engine::resources::Shader *shader,
                                              unsigned int amount) {
    if (model) {
        model->draw_instanced(shader, amount);
    }
}

void GraphicsController::initialize() {
    const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto handle = platform->window()
                          ->handle_();
    m_perspective_params.FOV = glm::radians(m_camera.Zoom);
    m_perspective_params.Width = static_cast<float>(platform->window()
                                                            ->width());
    m_perspective_params.Height = static_cast<float>(platform->window()
                                                             ->height());
    m_perspective_params.Near = 0.1f;
    m_perspective_params.Far = 100.f;

    m_ortho_params.Bottom = 0.0f;
    m_ortho_params.Top = static_cast<float>(platform->window()
                                                    ->height());
    m_ortho_params.Left = 0.0f;
    m_ortho_params.Right = static_cast<float>(platform->window()
                                                      ->width());
    m_ortho_params.Near = 0.1f;
    m_ortho_params.Far = 100.0f;
    platform->register_platform_event_observer(
            std::make_unique<GraphicsPlatformEventObserver>(this));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
    RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    glDeleteVertexArrays(1, &m_floor_vao);
    glDeleteBuffers(1, &m_floor_vbo);

    glDeleteVertexArrays(1, &m_grass_vao);
    glDeleteBuffers(1, &m_grass_vbo);

    glDeleteVertexArrays(1, &m_light_cube_vao);
    glDeleteBuffers(1, &m_light_cube_vbo);
    glDeleteBuffers(1, &m_light_cube_ebo);

    glDeleteVertexArrays(1, &m_sphere_vao);
    glDeleteBuffers(1, &m_sphere_vbo);
    glDeleteBuffers(1, &m_sphere_ebo);

    if (m_tree_instance_vbo != 0) {
        glDeleteBuffers(1, &m_tree_instance_vbo);
    }
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params()
              .Width = static_cast<float>(width);
    m_graphics->perspective_params()
              .Height = static_cast<float>(height);

    m_graphics->orthographic_params()
              .Right = static_cast<float>(width);
    m_graphics->orthographic_params()
              .Top = static_cast<float>(height);
}

std::string_view GraphicsController::name() const {
    return "GraphicsController";
}

void GraphicsController::begin_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicsController::end_gui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
    CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    CHECKED_GL_CALL(glDepthFunc, GL_LESS); // set depth function back to default
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}
}
