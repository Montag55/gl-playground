#pragma once
#include <memory>

// assure compile class will be there
class GraphApp;

class Tool {
public:
	Tool(GraphApp* app);
	virtual bool draw() const = 0;
	virtual bool registerTool() = 0;
	void setPriority(int prio);

protected:
    GraphApp* m_linkedApp;
	int m_priotity;
};