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
#include <expansionMiddle.hpp>
#include <list>
#include <functional>

class TimeSeries : Tool {
public:
    TimeSeries(GraphApp* app);
	~TimeSeries();
    bool draw() const override;
    bool registerTool() override;
    bool checkSelection(const glm::vec2& cursor);
    void updateSelections();
    
private:
    void createEntry(TimeExpansion& entry) const;
    void setEntryCoords(TimeExpansion& entry) const;
    void updateParentIndicies();
    void initializeVertexBuffers();
    void initializeIndexBuffers();
    void initializeStorageBuffers();
    void updateEntries() const;
    void deleteEntry(const int& index);
        
    GLuint m_program;
    GLuint m_middle_program;
    GLuint m_addVisualizer_program;
    glm::mat4 m_draw_model; // scale of polyline
    glm::mat4 m_mouse_model; // scale of screen to polyine
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_time_ssbo;

    int m_num_timeAxis;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned short> m_indicies;
    std::vector<TimeExpansion> m_expansions;
    std::vector<float> m_prevAxis;
    std::vector<float> m_timeAxis;
    std::vector<int> m_excludedAxis; // left & right
    std::vector<int> m_middleAxis;  // middle
};