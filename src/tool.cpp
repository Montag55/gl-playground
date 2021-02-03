#include <tool.hpp>

Tool::Tool(GraphApp* app) :
	m_linkedApp{app}
{	
}

Tool::~Tool() {
	// delete m_linkedApp;
}

void Tool::setPriority(int prio) {
	m_priotity = prio;
}