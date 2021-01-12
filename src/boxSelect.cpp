#include <boxSelect.hpp>

BoxSelect::BoxSelect(GraphApp* app) :
    Tool{app}
{
    auto vert_shader = gl::load_shader_from_file("shaders/selection_rect.vert", GL_VERTEX_SHADER);
    auto frag_shader = gl::load_shader_from_file("shaders/selection_rect.frag", GL_FRAGMENT_SHADER);
    m_program = gl::create_program({vert_shader, frag_shader});
    
    // model relative to Polylines scale
    m_model= glm::scale(glm::mat4{1.0f}, glm::vec3{
        1.0f / m_linkedApp->getModel()[0][0], 
        1.0f / m_linkedApp->getModel()[1][1], 
        1.0f / m_linkedApp->getModel()[2][2]}
    );
    
    // init gpu buffers
    initializeVertexBuffers();
}

BoxSelect::~BoxSelect() {
    if (glIsBuffer(m_vbo)) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
        glDeleteVertexArrays(1, &m_vao);
    } 
    if (glIsProgram(m_program)) {
        glDeleteProgram(m_program);
    }
}
    
void BoxSelect::initializeVertexBuffers() {
    // setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Point));

    GLuint pos_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, pos_attrib_idx);
    glVertexArrayAttribFormat(m_vao, pos_attrib_idx, 2, GL_FLOAT, false, offsetof(Point, pos));
    glVertexArrayAttribBinding(m_vao, pos_attrib_idx, 0);
        
    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), NULL, GL_DYNAMIC_DRAW);
}

void BoxSelect::updateSelection_callback(const glm::vec2& cursor) const {
  
    float x = Utils::remap(cursor.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1,1));
    float y = Utils::remap(cursor.y, glm::vec2(0, m_linkedApp->resolution().y), glm::vec2(-1,1));
      
    BoxSelect* ptr =  const_cast<BoxSelect*> (this);
    ptr->m_selectionArea.c2 = glm::vec2(x,y);

    ptr->m_vertices[0] = Point{  glm::vec2( m_selectionArea.c1.x, m_selectionArea.c1.y) };
    ptr->m_vertices[1] = Point{  glm::vec2( m_selectionArea.c2.x, m_selectionArea.c1.y) };
    ptr->m_vertices[2] = Point{  glm::vec2( m_selectionArea.c2.x, m_selectionArea.c2.y) };
    ptr->m_vertices[3] = Point{  glm::vec2( m_selectionArea.c1.x, m_selectionArea.c2.y) };
             
    glNamedBufferSubData(m_vao, 0, Utils::vectorsizeof(m_vertices), m_vertices.data());

    // now check intersection;
    m_linkedApp->updateColor(checkIntersection());
}
    
void BoxSelect::setSelectionOrigin_callback(const glm::vec2& pos) const {
    // clear prev selection
    clearSelection();
    
    float x = Utils::remap(pos.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1,1));
    float y = Utils::remap(pos.y, glm::vec2(0, m_linkedApp->resolution().y), glm::vec2(-1,1));
        
    BoxSelect* ptr =  const_cast<BoxSelect*> (this);
    ptr->m_selectionArea.c1 = glm::vec2(x,y);
    ptr->m_active = true;
    
    // start on first frame of interaction
    updateSelection_callback(pos);
}

void BoxSelect::clearSelection() const {
    // remove current selected line ids    
    BoxSelect* ptr =  const_cast<BoxSelect*> (this);
    ptr->m_current_selection_ids.clear();
    
    // reset colors
    m_linkedApp->updateColor(std::vector<int>{}, true);
}

void BoxSelect::stopSelection_callback() const {
    BoxSelect* ptr =  const_cast<BoxSelect*> (this);
    ptr->m_active = false;
}

bool BoxSelect::draw() const {
    if(!m_active)
        return true;
        
    glUseProgram(m_program);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "color"), glm::vec4(0.639, 0.670, 0.741, 0.5));
        
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_LOOP, 0, m_vertices.size());

    return true;
}

bool BoxSelect::registerTool() {
    /*m_linkedApp->registerCallbacks(Callback<BoxSelect*, BS_VEC>{
        this,
        &BoxSelect::updateSelection_callback
    });

    m_linkedApp->registerCallbacks(Callback<BoxSelect*, BS_VEC>{
        this,
        &BoxSelect::setSelectionOrigin_callback
    });

    m_linkedApp->registerCallbacks(Callback<BoxSelect*, BS_EMPTY>{
        this,
        &BoxSelect::stopSelection_callback
    });*/
    
    return true;
}
  
std::vector<int> BoxSelect::checkIntersection() const{
    // list of selected line ID's - list to remove duplicates
    std::list<int> selected;
    
    // transform selection AABB relativ to Polylines 
    auto selection_p1 = m_model * glm::vec4( m_selectionArea.c1.x, m_selectionArea.c1.y, 0, 1 );
    auto selection_p2 = m_model * glm::vec4( m_selectionArea.c2.x, m_selectionArea.c2.y, 0, 1 );
    
    // create sorted AABB - [TopLeft, BottomRight] for Selection
    auto selectionAABB = AABB {
        glm::vec2(glm::min(selection_p1.x, selection_p2.x), 
                  glm::max(selection_p1.y, selection_p2.y)),
        glm::vec2(glm::max(selection_p1.x, selection_p2.x), 
                  glm::min(selection_p1.y, selection_p2.y)) 
    };
        
    // check every line if it is inside AABB - later maybe BSP
    for (unsigned int i = 0; i < m_linkedApp->getIndicies()->size() - 1; i+=2) {
              
        // Get Vertecies of line segment
        Vertex v1 = (*m_linkedApp->getVertecies())[(*m_linkedApp->getIndicies())[i]];
        Vertex v2 = (*m_linkedApp->getVertecies())[(*m_linkedApp->getIndicies())[i + 1]];
        
        auto x1 = (*m_linkedApp->getAxis())[v1.attIndx];
        auto y1 = Utils::remap((*m_linkedApp->getData())[ m_linkedApp->getAxis()->size() * (int)v1.id + (int)v1.attIndx], (*m_linkedApp->getRanges())[(int)v1.attIndx], glm::vec2(-1,1));
        
        auto x2 = (*m_linkedApp->getAxis())[v2.attIndx];
        auto y2 = Utils::remap((*m_linkedApp->getData())[ m_linkedApp->getAxis()->size() * (int)v2.id + (int)v2.attIndx], (*m_linkedApp->getRanges())[(int)v2.attIndx], glm::vec2(-1,1));

        auto lineAABB = AABB {
            glm::vec2(glm::min(x1, x2), glm::max(y1, y2)),
            glm::vec2(glm::max(x1, x2), glm::min(y1, y2))   
        };
        
        // these are just intersection candidates (overlapping AABB)
        if (Utils::intersectionAABB(lineAABB, selectionAABB)) {
            
            // check if either start or endpoint are within AABB
            // if they are, no need to check the other line verts
            if (Utils::insideAABB(glm::vec2(x1, y1), selectionAABB) || Utils::insideAABB(glm::vec2(x2, y2), selectionAABB)) {
                selected.push_back(v1.id);
                //i += m_linkedApp->getAxis()->size() - v1.attIndx - 1;
                continue;
            }

	        // compute intermediate vertices
            auto dist = x2 - x1;
	        float intermediate_x = x1 + 0.5 * dist;
	        glm::vec2 p1 = glm::vec2(intermediate_x, y1);
	        glm::vec2 p2 = glm::vec2(intermediate_x, y2);
            
            // check if bezier point at left-coord is in AABB
            // if they are, no need to check the other line verts
            auto ratio_l = (selectionAABB.c1.x - x1) / dist;
            auto inter_l = Utils::bezier(ratio_l, glm::vec2(x1, y1), p1, p2, glm::vec2(x2, y2));
            if (inter_l.y < selectionAABB.c1.y && inter_l.y > selectionAABB.c2.y ) {
                selected.push_back(v1.id);
                //i += m_linkedApp->getAxis()->size() - v1.attIndx - 1;
                continue;
            }

            // check if bezier point at right-coord is in AABB
            // if they are, no need to check the other line verts
            double ratio_r = (selectionAABB.c2.x - x1) / dist;
            auto inter_r = Utils::bezier(ratio_r, glm::vec2(x1, y1), p1, p2, glm::vec2(x2, y2));
            if (inter_r.y < selectionAABB.c1.y && inter_r.y > selectionAABB.c2.y ) {
                selected.push_back(v1.id);
                //i += m_linkedApp->getAxis()->size() - v1.attIndx - 1;
                continue;
            }
        }
    }
        
    BoxSelect* ptr =  const_cast<BoxSelect*> (this);
    ptr -> m_current_selection_ids = std::vector<int>(selected.begin(), selected.end());
    
    return m_current_selection_ids;
}