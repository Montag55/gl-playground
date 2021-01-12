#pragma once
#include <algorithm>

#include <chrono>
#include <variant>
#include <application.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <boxSelect.hpp>
#include <axisDrag.hpp>


// assure compile class will be there
class BoxSelect;
class AxisDrag;

class GraphApp : 
    public Application, 
    public std::enable_shared_from_this<GraphApp> 
{
 public:
    GraphApp();
    ~GraphApp();
	bool draw() const override;
    void updateColor(const std::vector<int>& ids, bool reset = false) const;
    void updateAxis(const std::vector<float>& axis) const;
    void updateVertexIndicies(const std::vector<int>& order) const;
    const std::vector<Vertex>* getVertecies();
    const std::vector<unsigned short>* getIndicies();
    const std::vector<float>* getAxis();
    const std::vector<float>* getData();
    const glm::mat4& getModel();
    const std::vector<glm::vec2>* getRanges();

private: 
	std::vector<float> initializeData();
    std::vector<float> initializeAxis();
    std::vector<glm::vec2> initializeRanges();
	void initializeColor();
	void initializeVertexBuffers();
	void initializeStorageBuffers();
	void initializeIndexBuffer();	
    
    void drawPolyLines() const;
	void mouseEventListener() const;

protected:
    glm::mat4 m_model;
	GLuint m_polyline_program;
    GLuint m_axis_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_data_ssbo;
    GLuint m_color_ssbo;
    GLuint m_attribute_ssbo;
    GLuint m_range_ssbo;
    
    int m_num_attributes;
    std::vector<float> m_axis;
    std::vector<float> m_data;
    std::vector<Vertex> m_vertices;
    std::vector<glm::vec2> m_ranges;
    std::vector<glm::vec4> m_colors;
    std::vector<Vertex> m_selection;
    std::vector<unsigned short> m_indicies;
    
    bool m_selecting;
    BoxSelect* m_boxSelect_tool;
    AxisDrag* m_axisDrag_tool;
    MouseStatus m_prevMouseState;
};