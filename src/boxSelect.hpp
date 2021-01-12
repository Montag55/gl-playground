#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>

#include <tool.hpp>
#include <structs.hpp>
#include <utils.hpp>
#include <graphApp.hpp>
#include <list>
#include <functional>

class BoxSelect : Tool {
public:
    BoxSelect(GraphApp* app);
    ~BoxSelect();
    void initializeVertexBuffers();
    void clearSelection() const;
    bool draw() const override;
    bool registerTool() override;
    std::vector<int> checkIntersection() const;
    
    void stopSelection_callback() const;
    void updateSelection_callback(const glm::vec2& cursor) const;
    void setSelectionOrigin_callback(const glm::vec2& pos) const;

 protected:
    glm::mat4 m_model;
    GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    
    std::vector<Point> m_vertices{Point{}, Point{}, Point{}, Point{}};
    std::vector<int> m_current_selection_ids;
    glm::vec4 m_selection_color;
    AABB m_selectionArea;
    bool m_active;
};


