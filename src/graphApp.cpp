#include<graphApp.hpp>

GraphApp::GraphApp() : 
    Application{}, 
    m_num_attributes{4},
    m_num_timeAxis{4},
    m_model{glm::scale(glm::mat4{1.0f}, glm::vec3{0.8f})},
    m_data{initializeData()}, // init for tools
    m_axis{initializeAxis()},  // init for tools
    m_ranges{initializeRanges()},  // init for tools
    m_boxSelect_tool{new BoxSelect(this)},  // enable boxSelection tool
    m_axisDrag_tool{new AxisDrag(this)},    // enable axisDrag tool
    m_timeSeries_tool{new TimeSeries(this)} // enable timeSeries tool
{     
    // setup shader program
    auto vert_shader = gl::load_shader_from_file("shaders/polyline.vert", GL_VERTEX_SHADER);
    auto tcs_shader = gl::load_shader_from_file("shaders/polyline.tesc", GL_TESS_CONTROL_SHADER);
    auto tes_shader = gl::load_shader_from_file("shaders/polyline.tese", GL_TESS_EVALUATION_SHADER);
    auto frag_shader = gl::load_shader_from_file("shaders/polyline.frag", GL_FRAGMENT_SHADER);
    m_polyline_program = gl::create_program({vert_shader, tcs_shader, tes_shader, frag_shader});
    
    
    // init index stuff
    initializeIndexBuffer();
    initializeColor();        

    // init gpu buffers
    initializeVertexBuffers();
    initializeStorageBuffers();
    
    // activate color blending and setup background color
    m_clear_color = glm::vec3(0.125, 0.133, 0.156);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

GraphApp::~GraphApp() {
    if (glIsBuffer(m_vbo)) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (glIsBuffer(m_data_ssbo)) {
        glDeleteBuffers(1, &m_data_ssbo);
    }
    if (glIsBuffer(m_color_ssbo)) {
        glDeleteBuffers(1, &m_color_ssbo);
    }
    if (glIsBuffer(m_range_ssbo)) {
        glDeleteBuffers(1, &m_range_ssbo);
    }
    if (glIsBuffer(m_attribute_ssbo)) {
        glDeleteBuffers(1, &m_attribute_ssbo);
    }
    if (glIsBuffer(m_ibo)) {
        glDeleteBuffers(1, &m_ibo);
    }
    if (glIsProgram(m_polyline_program)) {
        glDeleteProgram(m_polyline_program);
    } 
    if (glIsProgram(m_axis_program)) {
        glDeleteProgram(m_axis_program);
    }
}

bool GraphApp::draw() const {
    Application::draw();
    glEnable(GL_DEPTH_TEST);

    mouseEventListener();
    
    // painters algo.: first background then foreground
    m_axisDrag_tool->draw();
    
    // both share same ssbos
    m_timeSeries_tool->draw();
    drawPolyLines();
        
    m_boxSelect_tool->draw();

    return true;
}
    
std::vector<float> GraphApp::initializeData() {
    std::vector<float> tmp;
    
    // Utils::readData(tmp, "../../iris.txt", false, true);
    Utils::readData(tmp, "../iris.txt", false, true);
    // Utils::readData(tmp, "../../sea-ice-extent-annually.csv", true, false);
    // Utils::readData(tmp, "../../sea-ice-extent.csv", true, false);
    
    // replace with actual time data
    // appending same data over and over again
    int size = tmp.size();
    for (int i = 0; i < m_num_timeAxis - 1; i++) {
        tmp.insert(tmp.end(), tmp.begin(), tmp.begin() + size);
    }
    
    spdlog::debug("data size {}", tmp.size());
    spdlog::debug("single time size {}", tmp.size()/ m_num_timeAxis);

    if (tmp.empty()) {
		throw std::runtime_error("Failed to initialze data!");
	}

    return tmp;
}

std::vector<glm::vec2> GraphApp::initializeRanges() {
    std::vector<glm::vec2> tmp;
    auto max = Utils::getMaxValues(m_data, m_num_attributes);
    auto min = Utils::getMinValues(m_data, m_num_attributes);

    for (int i = 0; i < m_num_attributes; i++) {
        tmp.push_back(glm::vec2(min[i], max[i]));
    }

    //for (int i = 0; i < m_data.size(); i++) {
    //    auto t = m_data[i];
    //    m_data[i] = Utils::remap(m_data[i], tmp[i % m_num_attributes], glm::vec2(-1,1));
    //    spdlog::debug("{}: {} -> {}",i, t, m_data[i] );
    //}
    
    return tmp;
}
    
void GraphApp::initializeColor() {
    // has to be called after initializeData(), initializeAxis()
    if (m_data.empty() || m_axis.empty()) {
        throw std::runtime_error("Failed to initialze Color data!");
    }
    m_colors.clear();
    for (int i = 0; i < (m_data.size() / m_num_timeAxis) / m_axis.size(); i++) {
        if(i < 50)
            m_colors.push_back(glm::vec4(0.321, 0.580, 0.886, 0.4f));
        else if(i >= 50 && i < 100)
            m_colors.push_back(glm::vec4(0, 0.811, 0.882, 0.4f));
        else if(i >= 100)
            m_colors.push_back(glm::vec4(0, 0.898, 0.729, 0.4f));
    }
}

std::vector<float> GraphApp::initializeAxis() {
    std::vector<float> tmp(m_num_attributes, 0);
    for (int i = 0; i < m_num_attributes; i++) {
        tmp[i] = Utils::remap((float)i / (m_num_attributes - 1), glm::vec2{0,1}, glm::vec2{-1,1});
    }
    return tmp;
}

void GraphApp::initializeVertexBuffers() {
    /**
    *   Only passing in floats since int and uint seems to have padding / offset errors.
    *   ID and AttribueIndex are casted to int in shader           
    */
        
    // setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
    
    // setup value id attribute
    GLuint id_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, id_attrib_idx);
    glVertexArrayAttribFormat(m_vao, id_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, id));
    glVertexArrayAttribBinding(m_vao, id_attrib_idx, 0);

    // setup attribute index id attribute
    GLuint att_attrib_idx = 1;
    glEnableVertexArrayAttrib(m_vao, att_attrib_idx);
    glVertexArrayAttribFormat(m_vao, att_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, attIndx));
    glVertexArrayAttribBinding(m_vao, att_attrib_idx, 0);

    // setup vertices
    for (int i = 0; i < m_data.size() / m_num_timeAxis; i++) {
        m_vertices.push_back( Vertex{
            float(i / m_axis.size()),
            float(i % m_axis.size())
        });
    }

    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), m_vertices.data(), GL_DYNAMIC_DRAW);
}
    
void GraphApp::initializeStorageBuffers() {         
    // setup data ssbo
    GLuint data_binding = 0;
	glCreateBuffers(1, &m_data_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_binding, m_data_ssbo);
	glNamedBufferData(m_data_ssbo, Utils::vectorsizeof(m_data), m_data.data(), GL_DYNAMIC_DRAW);
             
    // setup color ssbo
    GLuint color_binding = 1;
	glCreateBuffers(1, &m_color_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, color_binding, m_color_ssbo);
	glNamedBufferData(m_color_ssbo, Utils::vectorsizeof(m_colors), m_colors.data(), GL_DYNAMIC_DRAW);
        
    // setup attribute ranges ssbo
    GLuint range_binding = 2;
	glCreateBuffers(1, &m_range_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, range_binding, m_range_ssbo);
	glNamedBufferData(m_range_ssbo, Utils::vectorsizeof(m_ranges), m_ranges.data(), GL_DYNAMIC_DRAW);

    // setup attribute axis pos (x-coord) ssbo
    GLuint attribute_pos_binding = 3;
	glCreateBuffers(1, &m_attribute_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, attribute_pos_binding, m_attribute_ssbo);
	glNamedBufferData(m_attribute_ssbo, Utils::vectorsizeof(m_axis), m_axis.data(), GL_DYNAMIC_DRAW);
}
    
void GraphApp::initializeIndexBuffer() {
    // has to be called after initializeData(), initializeAxis()
    if (m_data.empty() || m_axis.empty()) {
        throw std::runtime_error("Failed to initialze Color data!");
    }
        
    // tes-schader can't do linestrips -> push all vertex indicies 
    // in twice except for first and last of each line 
    for (unsigned short i = 0; i < m_data.size() / m_num_timeAxis; i++) {
        m_indicies.push_back(i);
        if (i % m_num_attributes != 0 && i % m_num_attributes != m_num_attributes - 1) {
            m_indicies.push_back(i);
        }
    }

    // Bind to Element array buffer -> Indexing so DrawElements can be used
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_DYNAMIC_DRAW);
}

void GraphApp::drawPolyLines() const {
    glUseProgram(m_polyline_program);
        
    gl::set_program_uniform(m_polyline_program, glGetUniformLocation(m_polyline_program, "transform"), m_model);
    gl::set_program_uniform(m_polyline_program, glGetUniformLocation(m_polyline_program, "to_range"), glm::vec2(-1, 1));
    glProgramUniform1i(m_polyline_program, glGetUniformLocation(m_polyline_program, "num_attributes"), m_axis.size());
        
    // bind buffers eventhough they were never unbinded, just to be sure
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_data_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_color_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_range_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_attribute_ssbo);
        
    // tell tesellation shader how many verts per line
    glPatchParameteri(GL_PATCH_VERTICES, 2);
                
    // uses buffer currently bound to GL_ELEMENT_ARRAY_BUFFER
    glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*) 0);
}
    
void GraphApp::mouseEventListener() const {
    GraphApp* ptr =  const_cast<GraphApp*> (this);
    MouseStatus current = MouseStatus {
        m_mouse_buttons[Left],
        m_mouse_buttons[Middle],
        m_mouse_buttons[Right],
        m_mouse_pos,
        MouseState::Default,
        m_prevMouseState.time[Left],
        m_prevMouseState.time[Middle],
        m_prevMouseState.time[Right]
    };
       
    // this section only gathers mouse event information, no logic here
    {   
        // left-mouse since last click duration
        auto duration = Utils::ellapsedTime(current.time[Left], std::chrono::system_clock::now());
        
        // left-mouse down - START
        if (!m_prevMouseState.buttons[Left] && current.buttons[Left]) {
            // Set mouse state to click
            current.state = Click;
            
            // left-mouse double click
            if (duration <= DOUBLECLICK_TIME_MS) {
                current.state = Double_Click;
            }

            // start reporting ellapsed time
            current.time[Left] = std::chrono::system_clock::now();
            // start reporting moved mouse distance
            current.distance = 0.0f;
        }
        // left-mouse down - ONGOING
        else if(m_prevMouseState.buttons[Left] && current.buttons[Left]){
            // Set mouse state to drag
            current.state = Drag;
            // reporting moved mouse distance
            current.distance = m_prevMouseState.distance + glm::distance(m_prevMouseState.pos, current.pos);
        }
        // left-mouse down - END
        else if (m_prevMouseState.buttons[Left] && !current.buttons[Left]){
            // Set mouse state to release
            current.state = Release;
        }
    }  
    
    // do something with mouse info 
    switch (current.state) {
        case Click: 
            // set boxselection tool starting location
            if(!m_axisDrag_tool->updateSelection(m_prevMouseState.pos, current.pos))
                m_boxSelect_tool->setSelectionOrigin_callback(current.pos);
            break;
        case Drag:
            // upate boxselection tool here
            if(!m_axisDrag_tool->updateSelection(m_prevMouseState.pos, current.pos))
                m_boxSelect_tool->updateSelection_callback(current.pos); 
            break;
        case Release:
            // clear boxselection tool here and update all time expansions
            if (!m_axisDrag_tool->updateSelection(m_prevMouseState.pos, current.pos)) 
                m_boxSelect_tool->stopSelection_callback();
            else
                m_timeSeries_tool->updateSelections();
                  
            break;
        case Double_Click:
            // update time-series-expansion tool here
            m_timeSeries_tool->checkSelection(current.pos);
            break;
        default:
            // check if mouse over axis
            m_axisDrag_tool->checkSelection();
            break;
    }
    ptr->m_prevMouseState = current;
}

void GraphApp::updateAxis(const std::vector<float>& axis) const {
    GraphApp* ptr = const_cast<GraphApp*>(this);
    ptr->m_axis = axis;
    glNamedBufferSubData(m_attribute_ssbo, 0, Utils::vectorsizeof(m_axis), m_axis.data());
}

void GraphApp::updateColor(const std::vector<int>& ids, bool reset) const {
    if(reset){
        glNamedBufferSubData(m_color_ssbo, 0, Utils::vectorsizeof(m_colors), m_colors.data());
        return;
    }
    
    std::vector<glm::vec4> tmp_colors = m_colors;
    for (const auto& i : ids) {
        tmp_colors[i] = glm::vec4(1,0,0,1);
    }
    glNamedBufferSubData(m_color_ssbo, 0, Utils::vectorsizeof(tmp_colors), tmp_colors.data());
}

void GraphApp::updateVertexIndicies() const {
    /**
     * Determines new indecies since either vertex order 
     * is nolong the same eg. 1st vertex in vbo, 
     * might now be 2nd vertex in polyline.
     * or line segments are excluded
    **/
    
    GraphApp* ptr = const_cast<GraphApp*>(this);
    ptr->m_indicies.clear();
    
    // count number of an excluded axis occurence
    // 0 - enter index twice
    // 1 - enter index once
    // 2 - skip index completly
    std::vector<int> occurences;
    for (int i = 0; i < m_axis.size(); i++) {
        occurences.push_back(std::count(m_excludedAxis.begin(), m_excludedAxis.end(), i));
    }

    // create new index ordering
    for (int i = 0; i < m_data.size() / m_num_timeAxis; i += m_axis.size()) {
        for(int j = 0; j < m_axis.size(); j++){
            // if axis is excluded twice, skip it
            if (occurences[m_axisOrder[j]] > 1)
                continue;

            // if axis is excluded once and is start or endpoint of line, skip it
            if (occurences[m_axisOrder[j]] > 0 && (j == 0 || j == m_axis.size() - 1))
                continue;
            
            // add all but first and last indecies
            if (j != 0 && j != m_axis.size() - 1 && occurences[m_axisOrder[j]] <= 0) {
                ptr->m_indicies.push_back(i + m_axisOrder[j]);
            }
            
            // all all indicies
            ptr->m_indicies.push_back(i + m_axisOrder[j]);
        }
    }

    
    if (m_indicies.empty()) {
        ptr->m_indicies.push_back(0);
        ptr->m_indicies.push_back(0);
    }
   
    glNamedBufferSubData(m_ibo, 0, Utils::vectorsizeof(m_indicies), m_indicies.data());
}

void GraphApp::updateOrder(const std::vector<int>& order) const {
    GraphApp* ptr = const_cast<GraphApp*>(this);
    if (m_axisOrder != order) {
        ptr->m_axisOrder = order;
        updateVertexIndicies();
    }
}

void GraphApp::updateExcludedAxis(const std::vector<int>& axis) const {
    GraphApp* ptr = const_cast<GraphApp*>(this);
    if (m_excludedAxis != axis) {
        ptr->m_excludedAxis = axis;
        updateVertexIndicies();
    }
}

const std::vector<Vertex>* GraphApp::getVertecies(){
    return &m_vertices;
}

const std::vector<float>* GraphApp::getAxis() {
    return &m_axis;
}

const std::vector<unsigned short>* GraphApp::getIndicies() {
    return &m_indicies;
}

const std::vector<float>* GraphApp::getData() {
    return &m_data;
}

const std::vector<glm::vec4>* GraphApp::getColor() {
    return &m_colors;
}

const glm::mat4& GraphApp::getModel() {
    return m_model;
}

const std::vector<glm::vec2>* GraphApp::getRanges() {
    return &m_ranges;
}

const std::vector<int>* GraphApp::getAxisOrder() {
    return &m_axisOrder;
}

const int* GraphApp::getNumTimeAxis() {
    return &m_num_timeAxis;
}

const GraphApp* GraphApp::getPtr() {
    return this;
}

const GLuint* GraphApp::getVAO() {
    return &m_vao;
}

const GLuint* GraphApp::getAttribute_SSBO() {
    return &m_attribute_ssbo;
}

int main() {
    GraphApp app; 
    app.run();
    return 0;
}
