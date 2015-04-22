#include "VroomView.h"

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/ComController.h>

#include <iostream>

using namespace cvr;

CVRPLUGIN(VroomView)

VroomView::VroomView()
{
	_mls = NULL;
}

VroomView::~VroomView()
{
	if(_mls)
	{
		delete _mls;
	}
}

bool VroomView::init()
{
	if(ComController::instance()->isMaster())
	{
		int port = ConfigManager::getInt("value","Plugin.VroomView.ListenPort",12121);
		_mls = new MultiListenSocket(port);
		if(!_mls->setup())
		{
			std::cerr << "Error setting up MultiListen Socket on port " << port << " ." << std::endl;
			delete _mls;
			_mls = NULL;
		}
		else
		{
			std::cerr << "Socket listening on port: " << port << std::endl;
		}
	}

	return true;
}

void VroomView::preFrame()
{
	if(ComController::instance()->isMaster())
	{
		if(_mls)
		{
			CVRSocket * con;
			if((con = _mls->accept()))
			{
				std::cerr << "Adding socket." << std::endl;
				con->setNoDelay(true);
				//_socketList.push_back(con);
			}
		}

	}
}
