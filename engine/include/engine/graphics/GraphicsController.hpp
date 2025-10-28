/**
 * @file GraphicsController.hpp
 * @brief Defines the GraphicsController class that initializes OpenGL and ImGUI, and provides basic drawing functions.
*/

#ifndef GRAPHICSCONTROLLER_HPP
#define GRAPHICSCONTROLLER_HPP

#include <engine/graphics/Camera.hpp>
#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformEventObserver.hpp>
#include <engine/resources/Model.hpp>

struct ImGuiContext;

namespace engine::resources {
class Skybox;

class Shader;
}

namespace engine::graphics {
/**
* @brief Parameters used to define a perspective projection matrix.
*/
struct PerspectiveMatrixParams {
    float FOV;
    float Width;
    float Height;
    float Near;
    float Far;
};

/**
* @brief Parameters used to define an orthographic projection matrix.
*/
struct OrthographicMatrixParams {
    float Left;
    float Right;
    float Bottom;
    float Top;
    float Near;
    float Far;
};

enum ProjectionType {
    Perspective,
    Orthographic
};

/**
* @class GraphicsController
* @brief Implements basic drawing methods that the @ref core::App implementation uses.
*
* This class should implement all the complex functions needed for drawing an entity in the scene.
* For example @ref GraphicsController::draw_skybox.
*/
class GraphicsController final : public core::Controller {
public:
    std::string_view name() const override;

    /**
    * @brief Calls internal methods for the beginning of gui drawing. Should be called in pair with @ref GraphicsController::end_gui.
    *
    * Example:
    * @code
    * auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    * graphics->begin_gui();
    * ImGui::Begin("Camera info");
    * const auto &c = ...;
    * ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    * ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    * ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    * ImGui::End();
    * graphics->end_gui();
    * @endcode
    */
    void begin_gui();

    /**
    * @brief Calls internal method for the ending of gui drawing. Should be called in pair with @ref GraphicsController::begin_gui.
    */
    void end_gui();

    /**
    * @brief Draws a @ref resources::Skybox with the @ref resources::Shader.
    */
    void draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox);

    Camera *camera() {
        return &m_camera;
    }

    /**
    * @brief Compute the projection matrix.
    * @returns Return perspective projection by default.
    */
    template<ProjectionType type = Perspective>
    glm::mat4 projection_matrix() const {
        if constexpr (type == Perspective) {
            return glm::perspective(m_perspective_params.FOV,
                                    m_perspective_params.Width / m_perspective_params.Height,
                                    m_perspective_params.Near, m_perspective_params.Far);
        } else {
            return glm::ortho(m_ortho_params.Left, m_ortho_params.Right, m_ortho_params.Bottom, m_ortho_params.Top,
                              m_ortho_params.Near, m_ortho_params.Far);
        }
    }

    /**
    * @brief Compute the projection matrix.
    * @returns Return perspective projection by default.
    */
    glm::mat4 projection_matrix(ProjectionType type = Perspective) const {
        switch (type) {
            case Perspective: return projection_matrix<Perspective>();
            case Orthographic: return projection_matrix<Orthographic>();
            default: RG_SHOULD_NOT_REACH_HERE("Unsupported type");
        }
    }

    /**
    * @brief Use this function to change the perspective projection matrix parameters.
    * Projection matrix is always computed when the @ref GraphicsController::projection_matrix is called.
    * @returns @ref PerspectiveMatrixParams
    */
    PerspectiveMatrixParams &perspective_params() {
        return m_perspective_params;
    }

    /**
    * @brief Get the current @ref PerspectiveMatrixParams values.
    * @returns @ref PerspectiveMatrixParams
    */
    const PerspectiveMatrixParams &perspective_params() const {
        return m_perspective_params;
    }

    /**
    * @brief Use this function to change the orthographic projection matrix parameters.
    * Projection matrix is always computed
    * when @ref GraphicsController::projection_matrix is called.
    * @returns @ref PerspectiveMatrixParams
    */
    OrthographicMatrixParams &orthographic_params() {
        return m_ortho_params;
    }

    /**
    * @brief Get the current @ref OrthographicMatrixParams values.
    * @returns @ref PerspectiveMatrixParams
    */
    const OrthographicMatrixParams &orthographic_params() const {
        return m_ortho_params;
    }

    void initialize_floor();

    void initialize_grass();

    void initialize_light_cube_mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

    void initialize_sphere_mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);

    void setup_tree_instancing(const std::vector<glm::mat4> &modelMatrices, unsigned int amount,
                               engine::resources::Model *treeModel);

    void draw_floor();

    void draw_grass();

    void draw_light_cube();

    void draw_sphere();

    /**
    * @brief Draws an instanced model using the previously bound instance VBO.
    */
    void draw_instanced_model(engine::resources::Model *model, engine::resources::Shader *shader, unsigned int amount);
    // --

private:
    /**
    * @brief Initializes OpenGL, ImGUI, and projection matrix params;
    */
    void initialize() override;

    void terminate();

    PerspectiveMatrixParams m_perspective_params{};
    OrthographicMatrixParams m_ortho_params{};

    glm::mat4 m_projection_matrix{};
    Camera m_camera{};
    ImGuiContext *m_imgui_context{};

    unsigned int m_floor_vao{0}, m_floor_vbo{0};
    unsigned int m_grass_vao{0}, m_grass_vbo{0};
    unsigned int m_light_cube_vao{0}, m_light_cube_vbo{0}, m_light_cube_ebo{0};
    unsigned int m_sphere_vao{0}, m_sphere_vbo{0}, m_sphere_ebo{0};
    size_t m_light_cube_index_count{0}, m_sphere_index_count{0};

    unsigned int m_tree_instance_vbo{0};
};

/**
* @class GraphicsPlatformEventObserver
* @brief Observers change in window size in order to update the projection matrix.
*/
class GraphicsPlatformEventObserver final : public platform::PlatformEventObserver {
public:
    explicit GraphicsPlatformEventObserver(GraphicsController *graphics) : m_graphics(graphics) {
    }

    void on_window_resize(int width, int height) override;

private:
    GraphicsController *m_graphics;
};
}
#endif //GRAPHICSCONTROLLER_HPP
