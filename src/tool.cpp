#include <tool.hpp>

Tool::Tool(GraphApp* app) :
	m_linkedApp{app}
{	
}

void Tool::setPriority(int prio) {
	m_priotity = prio;
}