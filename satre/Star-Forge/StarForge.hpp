#pragma once

#include <cvrKernel/CVRPlugin.h>
#include <cvrKernel/InteractionEvent.h>
#include <glm/glm.hpp>

class StarForge: public cvr::CVRPlugin {
public:

	StarForge();
	~StarForge();
	
	virtual bool init() override;
	virtual void preFrame() override;
	virtual void postFrame() override;
	virtual bool processEvent(cvr::InteractionEvent * event) override;
	virtual int getPriority() override { return 50; }
};