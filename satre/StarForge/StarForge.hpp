#pragma once

#include <cvrKernel/CVRPlugin.h>
#include <cvrKernel/InteractionEvent.h>


#include "UniverseObject.hpp"
#include <memory>
class StarForge: public cvr::CVRPlugin {
public:

	StarForge();
	~StarForge();
	
	virtual bool init() override;
	virtual void preFrame() override;
	virtual void postFrame() override;
	virtual bool processEvent(cvr::InteractionEvent * event) override;
	virtual int getPriority() override { return 100; }

protected:
	std::unique_ptr<UniverseObject> mUniverse;

	bool mFirstPreframeCall = true;
	double mStartTime = 0.0;
};