#include <timeSeries.hpp>


bool operator==(const TimeExpansion& a, const TimeExpansion& b) {
  if (a.leftAxisIndex == b.leftAxisIndex && a.rightAxisIndex == b.rightAxisIndex)
      return true;
  return false;
}

TimeSeries::TimeSeries(GraphApp* app):
	Tool(app), 
    m_num_timeAxis{*app->getNumTimeAxis()}
 {
    auto vert_shader = gl::load_shader_from_file("shaders/timeseries.vert", GL_VERTEX_SHADER);
    auto tcs_shader = gl::load_shader_from_file("shaders/timeseries.tesc", GL_TESS_CONTROL_SHADER);
    auto tes_shader = gl::load_shader_from_file("shaders/timeseries.tese", GL_TESS_EVALUATION_SHADER);
    auto frag_shader = gl::load_shader_from_file("shaders/polyline.frag", GL_FRAGMENT_SHADER);
    m_program = gl::create_program({vert_shader, tcs_shader, tes_shader, frag_shader});
    
    vert_shader = gl::load_shader_from_file("shaders/timeseriesmiddle.vert", GL_VERTEX_SHADER);
    tcs_shader = gl::load_shader_from_file("shaders/polyline.tesc", GL_TESS_CONTROL_SHADER);
    tes_shader = gl::load_shader_from_file("shaders/polyline.tese", GL_TESS_EVALUATION_SHADER);
    m_middle_program = gl::create_program({vert_shader, tcs_shader, tes_shader, frag_shader});
    
    vert_shader = gl::load_shader_from_file("shaders/expansion_active.vert", GL_VERTEX_SHADER);
    frag_shader = gl::load_shader_from_file("shaders/axis.frag", GL_FRAGMENT_SHADER);
    m_addVisualizer_program = gl::create_program({vert_shader, frag_shader});

    vert_shader = gl::load_shader_from_file("shaders/expansion_handle.vert", GL_VERTEX_SHADER);
    m_handle_program = gl::create_program({vert_shader, frag_shader});

    
    // model relative to screen space
    m_mouse_model = glm::scale(glm::mat4{1.0f}, glm::vec3{
        1.0f / m_linkedApp->getModel()[0][0], 
        1.0f / m_linkedApp->getModel()[1][1], 
        1.0f / m_linkedApp->getModel()[2][2]}
    );
    
    // model relative to Polylines scale
    m_draw_model = m_linkedApp->getModel();

    // init index vertex stuff
    initializeVertexBuffers();
    initializeIndexBuffers();
    initializeStorageBuffers();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
}

TimeSeries::~TimeSeries() {
    if (glIsBuffer(m_vbo)) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (glIsBuffer(m_ibo)) {
        glDeleteBuffers(1, &m_ibo);
    }
    if (glIsBuffer(m_time_ssbo)) {
        glDeleteBuffers(1, &m_time_ssbo);
    }
    if (glIsProgram(m_program)) {
        glDeleteProgram(m_program);
    }
    if (glIsProgram(m_middle_program)) {
        glDeleteProgram(m_middle_program);
    } 
    if (glIsProgram(m_addVisualizer_program)) {
        glDeleteProgram(m_addVisualizer_program);
    }
}

bool TimeSeries::checkSelection(const glm::vec2& cursor) {
    // convert mouse position to   
    float x = Utils::remap(cursor.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1, 1));
    float y = Utils::remap(cursor.y, glm::vec2(0, m_linkedApp->resolution().y), glm::vec2(-1, 1));
    auto ss_pos = m_mouse_model * glm::vec4(glm::vec2(x, y), 0, 1);    

    // check if mouse is between axis and report these
    auto axis = *m_linkedApp->getAxis();
    auto order = *m_linkedApp->getAxisOrder();
    for (int i = 0; i < order.size() - 1; i++) {

        auto aabb = AABB{
            glm::vec2{axis[order[i]], 1},
            glm::vec2{axis[order[i+1]], -1}
        };
        
        if (Utils::insideAABB(ss_pos, aabb)) {
            auto timeSeries = TimeExpansion{ 
                order[i], 
                order[i + 1] 
            };
            
            // handle degenerate case
            // if axis is comprimised by others, erase parent entry
            auto left = std::find(m_middleAxis.begin(), m_middleAxis.end(), order[i]);
            auto right = std::find(m_middleAxis.begin(), m_middleAxis.end(), order[i+1]);
            if (left != m_middleAxis.end() || right != m_middleAxis.end()) { 
                // find entry with axis as middle, remove entry
                for (int j = 0; j < m_expansions.size(); j++) {
                    for (const auto& axis : m_expansions[j].middleAxisIndicies) {
                        if (axis == order[i] || axis == order[i+1]) {
                            deleteEntry(j);
                            break;
                        }
                    }
                }

                // update parent    
                updateParentIndicies();
                
                // skip further interaction
                continue;
            }
    
            // handel normal case
            auto entry = std::find(m_expansions.begin(), m_expansions.end(), timeSeries);            
            if (entry != m_expansions.end()) {
                // if enty already in list, delete it
                deleteEntry(entry - m_expansions.begin());
            } 
            else {
                // if new and not included as middle elsewhere, create entry
                createEntry(timeSeries);
            }
    
            updateParentIndicies();
        }
    }

    return true;
}

void TimeSeries::updateSelections() {
    // update middle part of all expansions to update contained axis
    for (const auto& entry : m_expansions) {
        entry.middle->updateAxisOrder(entry.middleAxisIndicies);
        entry.addVisualizer->setActive(false);
    }
}

bool TimeSeries::updateHandles(const glm::vec2& prev, const glm::vec2& current) {
    bool moving_handle = false;
    for (const auto& entry : m_expansions) {
        moving_handle = moving_handle || entry.left_handle->updateSelection(prev, current, -entry.angle);
        moving_handle = moving_handle || entry.right_handle->updateSelection(prev, current, entry.angle);
    }
    return moving_handle;
}

void TimeSeries::checkHandles() {
    // check if mouse over any handle
    for (const auto& entry : m_expansions) {
        entry.left_handle->checkSelection(); 
        entry.right_handle->checkSelection(); 
    }
}

void TimeSeries::createEntry(TimeExpansion& entry) const{       
           
    // create highlighter
    entry.addVisualizer = new ExpansionActive(
        entry.leftAxisIndex, 
        entry.rightAxisIndex,
        m_addVisualizer_program,
        m_linkedApp
    );
    
    // create left handle
    entry.left_handle = new ExpansionHandles{
        entry.model_left, 
        m_handle_program,
        m_linkedApp
    };
    
    // create right handle
    entry.right_handle = new ExpansionHandles{
        entry.model_right,
        m_handle_program,
        m_linkedApp
    };

    // create middle section
    entry.middle = new ExpansionMiddle(
        entry.leftAxisIndex, 
        entry.rightAxisIndex,
        entry.left_handle,
        entry.right_handle,
        m_middle_program,
        m_linkedApp
    );

    setEntryCoords(entry);
    
    // add entry as as expansion
    TimeSeries* ptr = const_cast<TimeSeries*>(this);
    ptr->m_expansions.push_back(entry);
    
    // update all entrys
    updateEntries();
}

void TimeSeries::setEntryCoords(TimeExpansion& entry) const {
    auto axis = *m_linkedApp->getAxis();
    auto order = *m_linkedApp->getAxisOrder();
    
    // dynamic rotation dependent on axis distance
    entry.angle = glm::clamp(45 * glm::distance(axis[entry.leftAxisIndex], axis[entry.rightAxisIndex]) / (2 * m_mouse_model[2][2]), 0.0f, 45.0f);
    
    // rotate and shift time expansion to align with left axis
    entry.model_left = m_draw_model;
    entry.model_left = glm::translate(entry.model_left, glm::vec3(axis[entry.leftAxisIndex], 0.0f, 0.0f));
    entry.model_left = glm::rotate(entry.model_left, glm::radians(90 - entry.angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // rotate and shift time expansion to align with right axis
    entry.model_right = m_draw_model;
    entry.model_right = glm::translate(entry.model_right, glm::vec3(axis[entry.rightAxisIndex], 0.0f, 0.0f));
    entry.model_right = glm::rotate(entry.model_right, glm::radians(90 + entry.angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // place camera between axis
    entry.view = glm::lookAt(glm::vec3((axis[entry.leftAxisIndex] + axis[entry.rightAxisIndex]) * 0.5f, 0, 0),
                           glm::vec3((axis[entry.leftAxisIndex] + axis[entry.rightAxisIndex]) * 0.5f, 0, -1),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    
    // update handle models
    entry.left_handle->updateVertecies(entry.model_left);
    entry.right_handle->updateVertecies(entry.model_right);
}

void TimeSeries::updateEntries() const {
    auto axis = *m_linkedApp->getAxis();
    
    // check if axis were shifted, if not -> return
    if (axis == m_prevAxis) {
        return;
    }

    auto order = m_linkedApp->getAxisOrder();
    TimeSeries* ptr = const_cast<TimeSeries*>(this);
    for (auto& entry : ptr->m_expansions) {
        
        // check if axis got flipped
        if (axis[entry.leftAxisIndex] > axis[entry.rightAxisIndex]) {
            auto tmp = entry.leftAxisIndex;
            entry.leftAxisIndex = entry.rightAxisIndex;
            entry.rightAxisIndex = tmp;
        }       

        // check if any axis is between left and right -> if so, add them to middle
        for (const auto& idx : *order) {
            auto it = std::find(entry.middleAxisIndicies.begin(), entry.middleAxisIndicies.end(), idx);
            if (it == entry.middleAxisIndicies.end() && // check if not in middle
                axis[idx] > axis[entry.leftAxisIndex] &&  // and inbetween axis
                axis[idx] < axis[entry.rightAxisIndex]) {
                
                // add to middle
                entry.middleAxisIndicies.push_back(idx);
                entry.addVisualizer->setActive(true);
                // entry.middle->updateAxis(entry.middleAxisIndicies);
                ptr->updateParentIndicies();
            } 
            else if (it != entry.middleAxisIndicies.end() && // if already in middle
                     (axis[idx] < axis[entry.leftAxisIndex] ||  // and nolong inbetween axis
                      axis[idx] > axis[entry.rightAxisIndex])) {
                
                // remove from middle
                entry.middleAxisIndicies.erase(it);
                entry.addVisualizer->setActive(false);
                // remove entry when draged outside immidiately
                entry.middle->updateAxisOrder(entry.middleAxisIndicies);
                ptr->updateParentIndicies();
            }
        }
        
        // setup all mats and coords
        setEntryCoords(entry);
    }

    // update axis
    ptr->m_prevAxis = axis;
}

void TimeSeries::updateParentIndicies() {
    // check get all axis included by any expansion
    m_excludedAxis.clear();
    m_middleAxis.clear();

    for (const auto& entry : m_expansions) {
        m_excludedAxis.push_back(entry.leftAxisIndex);
        m_excludedAxis.push_back(entry.rightAxisIndex);
        m_middleAxis.insert(m_middleAxis.end(), entry.middleAxisIndicies.begin(), entry.middleAxisIndicies.end());
        m_middleAxis.insert(m_middleAxis.end(), entry.middleAxisIndicies.begin(), entry.middleAxisIndicies.end());
    }
        
    // append exluded axis by all compromised middle axis
    m_excludedAxis.insert(m_excludedAxis.end(), m_middleAxis.begin(), m_middleAxis.end());
    m_linkedApp->updateExcludedAxis(m_excludedAxis);
}

bool TimeSeries::draw() const {
    // if there is nothing to draw -> retrun
    if (m_expansions.empty()) {
        return true;
    }
    
    // update all entrys according to their respectiv axis
    updateEntries();
    
    // draw axis befor lines
    for (const auto& item : m_expansions) {
        item.left_handle->draw();
        item.right_handle->draw();
    }
   
    // now here render that stuff
    glUseProgram(m_program);
    glDepthMask(GL_TRUE);
    
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "to_range"), glm::vec2(-1, 1));
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_data"), m_linkedApp->getData()->size() / m_num_timeAxis);
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_attrib"), m_linkedApp->getAxis()->size());
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_times"), m_num_timeAxis);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_time_ssbo);
    glPatchParameteri(GL_PATCH_VERTICES, 2);
    
    for (const auto& item : m_expansions) {
        
        // draw left 
        glProgramUniform1i(m_program, glGetUniformLocation(m_program, "attribute_idx"), item.leftAxisIndex);
        gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), item.model_left);
        glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
        
        // draw right
        glProgramUniform1i(m_program, glGetUniformLocation(m_program, "attribute_idx"), item.rightAxisIndex);
        gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), item.model_right);        
        glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
    }
      
    glDepthMask(GL_FALSE);

    // draw active indicator and middle over lines
    for (const auto& item : m_expansions) {
        item.middle->draw();
        item.addVisualizer->draw();
    }

	return true;
}

bool TimeSeries::registerTool() {
	return true;
}

void TimeSeries::initializeVertexBuffers() {
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
    
    // vertecie are the same for all time z-axis time esxpansions
    auto line_count = m_linkedApp->getData()->size() / m_linkedApp->getAxis()->size();
    for (int i = 0; i < line_count; i++) {
        m_vertices.push_back(Vertex{
            float(i / m_num_timeAxis),  // id
            float(i % m_num_timeAxis)   // time 
        });
    }
    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), m_vertices.data(), GL_DYNAMIC_DRAW);
}

void TimeSeries::initializeIndexBuffers() {
    // indicies are the same for all time z-axis time esxpansions    
    auto line_count = m_linkedApp->getData()->size() / m_linkedApp->getAxis()->size();
    for (unsigned short i = 0; i < line_count; i++) {
        m_indicies.push_back(i);
        if (i % m_num_timeAxis != 0 && i % m_num_timeAxis != m_num_timeAxis - 1) {
            m_indicies.push_back(i);
        }
    }

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_DYNAMIC_DRAW);
}

void TimeSeries::initializeStorageBuffers() {
    for (int i = 0; i < m_num_timeAxis; i++) {
        auto val = (float)i / (m_num_timeAxis - 1);
        m_timeAxis.push_back(val);
    }
   
    GLuint time_pos_binding = 4;
    glCreateBuffers(1, &m_time_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, time_pos_binding, m_time_ssbo);
    glNamedBufferData(m_time_ssbo, Utils::vectorsizeof(m_timeAxis), m_timeAxis.data(), GL_DYNAMIC_DRAW);
}

void TimeSeries::deleteEntry(const int& index) {
    auto entry = m_expansions[index];
    delete entry.middle;
    delete entry.addVisualizer;
    m_expansions.erase(m_expansions.begin() + index);
}