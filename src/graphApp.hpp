#pragma once
#include <algorithm>

#include <chrono>
#include <variant>
#include <application.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <boxSelect.hpp>
#include <axisDrag.hpp>
#include <timeSeries.hpp>


// assure compile class will be there
class BoxSelect;
class AxisDrag;
class TimeSeries;

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
    void updateVertexIndicies() const;
    const std::vector<Vertex>* getVertecies();
    const std::vector<unsigned short>* getIndicies();
    const std::vector<float>* getAxis();
    const std::vector<float>* getData();
    const std::vector<glm::vec4>* getColor();
    const glm::mat4& getModel();
    const std::vector<glm::vec2>* getRanges();
    const std::vector<int>* getAxisOrder();
    const GraphApp* getPtr();
    const GLuint* getVAO();
    const int* getNumTimeAxis();
    void updateOrder(const std::vector<int>& order) const;
    void updateExcludedAxis(const std::vector<int>& axis) const;

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
    int m_num_timeAxis;
    std::vector<float> m_axis;
    std::vector<float> m_data;
    std::vector<Vertex> m_vertices;
    std::vector<glm::vec2> m_ranges;
    std::vector<glm::vec4> m_colors;
    std::vector<Vertex> m_selection;
    std::vector<unsigned short> m_indicies;
    std::vector<int> m_axisOrder;
    std::vector<int> m_excludedAxis;
    
    bool m_selecting;
    BoxSelect* m_boxSelect_tool;
    AxisDrag* m_axisDrag_tool;
    TimeSeries* m_timeSeries_tool;
    MouseStatus m_prevMouseState;
};