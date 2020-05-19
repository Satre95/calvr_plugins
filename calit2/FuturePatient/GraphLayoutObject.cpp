#include "GraphLayoutObject.h"
#include "ColorGenerator.h"
#include "FuturePatient.h"
#include "GraphGlobals.h"
#include "SingleMicrobeObject.h"
#include "MicrobeScatterGraphObject.h"

#include <cvrInput/TrackingManager.h>
#include <cvrConfig/ConfigManager.h>

using namespace cvr;

GraphLayoutObject::GraphLayoutObject(float width, float height, int maxRows, std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) : FPTiledWallSceneObject(name,navigation,movable,clip,true,showBounds)
{
    _width = width;
    _height = height;
    _maxRows = maxRows;
    makeGeometry();
    makeKeys();

    _maxX = _minX = _currentMaxX = _currentMinX = 0;

    _resetLayoutButton = new MenuButton("Reset Layout");
    _resetLayoutButton->setCallback(this);
    addMenuItem(_resetLayoutButton);

    _syncTimeCB = new MenuCheckbox("Sync",false);
    _syncTimeCB->setCallback(this);
    addMenuItem(_syncTimeCB);

    _zoomCB = new MenuCheckbox("Zoom",false);
    _zoomCB->setCallback(this);
    addMenuItem(_zoomCB);

    _multiSelect = new MenuCheckbox("Multi Select",false);
    _multiSelect->setCallback(this);
    addMenuItem(_multiSelect);

    _rowsRV = new MenuRangeValueCompact("Rows",1.0,40.0,maxRows);
    _rowsRV->setCallbackType(MenuRangeValueCompact::ON_RELEASE);
    _rowsRV->setCallback(this);
    addMenuItem(_rowsRV);

    _widthRV = new MenuRangeValueCompact("Width",100.0,width*1.5,width);
    _widthRV->setCallbackType(MenuRangeValueCompact::ON_RELEASE);
    _widthRV->setCallback(this);
    addMenuItem(_widthRV);

    _heightRV = new MenuRangeValueCompact("Height",100.0,height*1.5,height);
    _heightRV->setCallbackType(MenuRangeValueCompact::ON_RELEASE);
    _heightRV->setCallback(this);
    addMenuItem(_heightRV);

    _minmaxButton = new MenuButton("Minimize");
    _minmaxButton->setCallback(this);
    addMenuItem(_minmaxButton);

    _linRegSort = new MenuButton("Linear Reg Sort");
    _linRegSort->setCallback(this);
    addMenuItem(_linRegSort);

    _selectAll = new MenuButton("Select All");
    _selectAll->setCallback(this);
    addMenuItem(_selectAll);

    _selectNone = new MenuButton("Select None");
    _selectNone->setCallback(this);
    addMenuItem(_selectNone);

    _removeUnselected = new MenuButton("Remove Unselected");
    _removeUnselected->setCallback(this);
    addMenuItem(_removeUnselected);

    _activeHand = -1;
    _activeHandType = TrackerBase::INVALID;

    _minimized = false;

    _selectionMenu = new PopupMenu("Selection Info","Plugin.FuturePatient.SelectionMenu");
    _selectionMenu->setVisible(false);

    _selectionText = new MenuText("",1.0,false);
    _selectionMenu->addMenuItem(_selectionText);
}

GraphLayoutObject::~GraphLayoutObject()
{
}

void GraphLayoutObject::addGraphObject(LayoutTypeObject * object)
{
    for(int i = 0; i < _objectList.size(); i++)
    {
	if(object == _objectList[i])
	{
	    return;
	}
    }

    _objectList.push_back(object);

    TimeRangeObject * tro = dynamic_cast<TimeRangeObject*>(object);
    ValueRangeObject * vro = dynamic_cast<ValueRangeObject*>(object);
    LogValueRangeObject * lvro = dynamic_cast<LogValueRangeObject*>(object);


    if((tro || vro || lvro) && _syncTimeCB->getValue())
    {
	if(vro || lvro || !_zoomCB->getValue())
	{
	    menuCallback(_syncTimeCB);
	}
	else if(tro)
	{
	    tro->setGraphDisplayRange(_currentMinX,_currentMaxX);
	}
    }

    addChild(object);
    object->objectAdded();

    _perGraphActiveHand.push_back(-1);
    _perGraphActiveHandType.push_back(TrackerBase::INVALID);

    MenuButton * button = new MenuButton("Delete");
    button->setCallback(this);
    object->addMenuItem(button);

    _deleteButtonMap[object] = button;

    updateLayout();
}

void GraphLayoutObject::removeGraphObject(LayoutTypeObject * object)
{
    int index = 0;
    for(std::vector<LayoutTypeObject *>::iterator it = _objectList.begin(); it != _objectList.end(); it++, index++)
    {
	if((*it) == object)
	{
	    object->removeMenuItem(_deleteButtonMap[object]);
	    delete _deleteButtonMap[object];
	    _deleteButtonMap.erase(object);
	    object->objectRemoved();
	    removeChild(object);
	    _objectList.erase(it);
	    if(object->getLayoutDoesDelete())
	    {
		delete object;
	    }
	    break;
	}
    }

    checkLineRefs();

    if(index < _perGraphActiveHand.size())
    {
	std::vector<int>::iterator it = _perGraphActiveHand.begin();
	it += index;
	_perGraphActiveHand.erase(it);
    }

    if(index < _perGraphActiveHandType.size())
    {
	std::vector<TrackerBase::TrackerType>::iterator it = _perGraphActiveHandType.begin();
	it += index;
	_perGraphActiveHandType.erase(it);
    }

    bool selectedObjects = false;

    for(int i = 0; i < _objectList.size(); ++i)
    {
	if(dynamic_cast<MicrobeSelectObject*>(_objectList[i]))
	{
	    selectedObjects = true;
	    break;
	}
    }

    if(!selectedObjects)
    {
	_currentSelectedMicrobeGroup = "";
	_currentSelectedMicrobes.clear();
    }

    bool selectedPatients = false;

    for(int i = 0; i < _objectList.size(); ++i)
    {
	if(dynamic_cast<PatientSelectObject*>(_objectList[i]))
	{
	    selectedPatients = true;
	    break;
	}
    }

    if(!selectedPatients)
    {
	//_currentSelectedPatientGroup = "";
	//_currentSelectedPatients.clear();
	_currentSelectedPatientMap.clear();
    }

    updateLayout();
}

void GraphLayoutObject::addLineObject(LayoutLineObject * object)
{
    if(!object)
    {
	return;
    }

    for(int i = 0; i < _lineObjectList.size(); ++i)
    {
	if(_lineObjectList[i] == object)
	{
	    return;
	}
    }

    _lineObjectList.push_back(object);

    addChild(object);

    updateLayout();
}

void GraphLayoutObject::removeLineObject(LayoutLineObject * object)
{
    for(std::vector<LayoutLineObject *>::iterator it = _lineObjectList.begin(); it != _lineObjectList.end(); ++it)
    {
	if((*it) == object)
	{
	    removeChild(object);
	    _lineObjectList.erase(it);
	    break;
	}
    }

    updateLayout();
}

void GraphLayoutObject::selectMicrobes(std::string & group, std::vector<std::string> & keys)
{
    // make copy to apply to new graphs when added
    _currentSelectedMicrobeGroup = group;
    _currentSelectedMicrobes = keys;

    for(int i = 0; i < _objectList.size(); ++i)
    {
	MicrobeSelectObject * mso = dynamic_cast<MicrobeSelectObject *>(_objectList[i]);
	if(mso)
	{
	    mso->selectMicrobes(group,keys);
	}
    }
    setSelectionText();
}

void GraphLayoutObject::selectPatients(std::string & group, std::vector<std::string> & patients)
{
    if(group.empty())
    {
	_currentSelectedPatientMap.clear();
    }
    else
    {
	if(!_multiSelect->getValue())
	{
	    _currentSelectedPatientMap.clear();
	    //_currentSelectedPatientGroup = group;
	    //_currentSelectedPatients = patients;
	    _currentSelectedPatientMap[group] = patients;
	}
	else
	{
	    if(!patients.size())
	    {
		_currentSelectedPatientMap[group] = patients;
	    }
	    else
	    {
		std::map<std::string,std::vector<std::string> >::iterator it;
		it = _currentSelectedPatientMap.find(group);
		if(it == _currentSelectedPatientMap.end())
		{
		    _currentSelectedPatientMap[group] = patients;
		}
		else if(it->second.size())
		{
		    for(int i = 0; i < patients.size(); ++i)
		    {
			bool add = true;
			for(int j = 0; j < it->second.size(); ++j)
			{
			    if(patients[i] == it->second[j])
			    {
				add = false;
				break;
			    }
			}
			if(add)
			{
			    it->second.push_back(patients[i]);
			}
		    }
		}
	    }
	}
    }

    for(int i = 0; i < _objectList.size(); ++i)
    {
	PatientSelectObject * pso = dynamic_cast<PatientSelectObject*>(_objectList[i]);
	if(pso)
	{
	    //pso->selectPatients(group,patients);
	    pso->selectPatients(_currentSelectedPatientMap);
	}
    }
}

void GraphLayoutObject::removeAll()
{
    for(int i = 0; i < _objectList.size(); i++)
    {
	_objectList[i]->objectRemoved();
	removeChild(_objectList[i]);
	if(_objectList[i]->getLayoutDoesDelete())
	{
	    delete _objectList[i];
	}
    }

    for(std::map<LayoutTypeObject *,cvr::MenuButton *>::iterator it = _deleteButtonMap.begin(); it != _deleteButtonMap.end(); it++)
    {
	it->first->removeMenuItem(it->second);
	delete it->second;
    }

    _deleteButtonMap.clear();
    _perGraphActiveHand.clear();
    _perGraphActiveHandType.clear();

    _currentSelectedMicrobeGroup = "";
    _currentSelectedMicrobes.clear();

    //_currentSelectedPatientGroup = "";
    //_currentSelectedPatients.clear();
    _currentSelectedPatientMap.clear();

    _objectList.clear();

    checkLineRefs();

    setTitle(getName());
    
    _selectionText->setText("");
    _selectionMenu->setVisible(false);
}

void GraphLayoutObject::perFrame()
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	_objectList[i]->perFrame();
    }
}

void GraphLayoutObject::minimize()
{
    if(_minimized)
    {
	return;
    }

    osg::Vec3 pos = ConfigManager::getVec3("Plugin.FuturePatient.MinimizedLayout");
    float scale = ConfigManager::getFloat("scale","Plugin.FuturePatient.MinimizedLayout",0.5);

    setScale(scale);
    setPosition(pos);

    for(int i = 0; i < _objectList.size(); ++i)
    {
	_objectList[i]->setGLScale(scale);
    }

    _minmaxButton->setText("Maximize");

    _minimized = true;
}

void GraphLayoutObject::maximize()
{
    if(!_minimized)
    {
	return;
    }

    osg::Vec3 pos = ConfigManager::getVec3("Plugin.FuturePatient.Layout");

    setScale(1.0);
    setPosition(pos);

    _minmaxButton->setText("Minimize");

    for(int i = 0; i < _objectList.size(); ++i)
    {
	_objectList[i]->setGLScale(1.0);
    }

    _minimized = false;
}

void GraphLayoutObject::setRows(float rows)
{
    _rowsRV->setValue(rows);
    menuCallback(_rowsRV);
}

void GraphLayoutObject::setSyncTime(bool sync)
{
    if(sync != _syncTimeCB->getValue())
    {
	_syncTimeCB->setValue(sync);
	menuCallback(_syncTimeCB);
    }
}

bool GraphLayoutObject::dumpState(std::ostream & out)
{
    out << getName() << std::endl;
    out << _objectList.size() << std::endl;
    for(int i = 0; i < _objectList.size(); ++i)
    {
	_objectList[i]->dumpState(out);
    }

    out << _width << " " << _height << " " << _maxRows << std::endl;
    out << _syncTimeCB->getValue() << " " << _zoomCB->getValue() << std::endl;
    out << _maxX << " " << _minX << " " << _currentMaxX << " " << _currentMinX << std::endl;
    out << _minimized << std::endl;

    osg::Vec3 pos = getPosition();
    out << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;
    out << getScale() << std::endl;

    out << !_currentSelectedMicrobeGroup.empty() << " " << _currentSelectedMicrobes.size() << std::endl;
    if(!_currentSelectedMicrobeGroup.empty())
    {
	out << _currentSelectedMicrobeGroup << std::endl;
    }
    for(int i = 0; i < _currentSelectedMicrobes.size(); ++i)
    {
	out << _currentSelectedMicrobes[i] << std::endl;
    }

    return true;
}

bool GraphLayoutObject::loadState(std::istream & in)
{
    _syncTimeCB->setValue(false);
    _zoomCB->setValue(false);

    char tempstr[1024];
    in.getline(tempstr,1024);
    in.getline(tempstr,1024);
    setTitle(tempstr);

    int numObjects;
    in >> numObjects;

    for(int i = 0; i < numObjects; ++i)
    {
	if(!loadObject(in))
	{
	    return false;
	}
    }

    // not using these for the moment
    float width, height;
    in >> width >> height >> _maxRows;
    //std::cerr << "Width: " << _width << " Height: " << _height << " MaxRows: " << _maxRows << std::endl;
    _rowsRV->setValue(_maxRows);
    //_widthRV->setValue(_width);
    //_heightRV->setValue(_height);

    bool sync, zoom;
    in >> sync >> zoom;
    //std::cerr << "Sync: " << sync << " Zoom: " << zoom << std::endl;
    _syncTimeCB->setValue(sync);
    _zoomCB->setValue(zoom);

    in >> _maxX >> _minX >> _currentMaxX >> _currentMinX;
    //std::cerr << "MaxX: " << _maxX << " MinX: " << _minX << " CMaxX: " << _currentMaxX << " CMinX: " << _currentMinX << std::endl;
    bool minimized;
    in >> minimized;
    //std::cerr << "Minimized: " << minimized << std::endl;
    
    float x,y,z;
    in >> x >> y >> z;
    float scale;
    in >> scale;

    // not used for the moment
    //setScale(scale);
    //setPosition(osg::Vec3(x,y,z));
    
    bool selectedGroup;
    int selectedMicrobes;
    in >> selectedGroup >> selectedMicrobes;
    //std::cerr << "Group: " << selectedGroup << " Microbes: " << selectedMicrobes << std::endl;
    
    if(selectedGroup || selectedMicrobes)
    {
	// call consume previous end line
	in.getline(tempstr,1024);
    }

    if(selectedGroup)
    {
	in.getline(tempstr,1024);
	_currentSelectedMicrobeGroup = tempstr;
    }
    else
    {
	_currentSelectedMicrobeGroup = "";
    }

    //std::cerr << "Selected Group: " << _currentSelectedMicrobeGroup << std::endl;

    _currentSelectedMicrobes.clear();

    for(int i = 0; i < selectedMicrobes; ++i)
    {
	in.getline(tempstr,1024);
	//std::cerr << "Microbe: " << tempstr << std::endl;
	_currentSelectedMicrobes.push_back(tempstr);
    }

    updateGeometry();
    updateLayout();

    return true;
}

bool lrSort(const std::pair<GraphObject*,time_t> & first, const std::pair<GraphObject*,time_t> & second)
{
    return first.second < second.second;
}

void GraphLayoutObject::menuCallback(MenuItem * item)
{
    if(item == _resetLayoutButton)
    {
	updateLayout();
	return;
    }

    if(item == _multiSelect)
    {
	std::string group;
	std::vector<std::string> patients;
	selectPatients(group,patients);
    }

    if(item == _rowsRV)
    {
	if(((int)_rowsRV->getValue()) != _maxRows)
	{
	    _maxRows = (int)_rowsRV->getValue();
	    updateLayout();
	}
	return;
    }

    if(item == _minmaxButton)
    {
	if(_minimized)
	{
	    maximize();
	}
	else
	{
	    minimize();
	}
	return;
    }

    if(item == _widthRV)
    {
	_width = _widthRV->getValue();
	updateGeometry();
	updateLayout();
	return;
    }

    if(item == _heightRV)
    {
	_height = _heightRV->getValue();
	updateGeometry();
	updateLayout();
	return;
    }

    if(item == _zoomCB)
    {
	if(_zoomCB->getValue())
	{
	}
	else
	{
	    if(_syncTimeCB->getValue())
	    {
		_activeHand = -1;
		_activeHandType = TrackerBase::INVALID;
		for(int i = 0; i < _objectList.size(); i++)
		{
		    _objectList[i]->setBarVisible(false);
		}
	    }
	    else
	    {
		for(int i = 0; i < _objectList.size(); i++)
		{
		    _perGraphActiveHand[i] = -1;
		    _perGraphActiveHandType[i] = TrackerBase::INVALID;
		    _objectList[i]->setBarVisible(false);
		}
	    }
	    menuCallback(_syncTimeCB);
	}
    }

    if(item == _linRegSort)
    {
	std::vector<GraphObject*> objects;
	std::vector<LinearRegFunc*> funcs;

	for(int i = 0; i < _objectList.size(); ++i)
	{
	    GraphObject * go = dynamic_cast<GraphObject*>(_objectList[i]);
	    if(go)
	    {
		for(int j = 0; j < go->getNumMathFunctions(); ++j)
		{
		    LinearRegFunc* func = dynamic_cast<LinearRegFunc*>(go->getMathFunction(j));
		    if(func)
		    {
			objects.push_back(go);
			funcs.push_back(func);
		    }
		}
	    }
	}

	std::vector<std::pair<GraphObject*,time_t> > graphSortList;
	for(int i = 0; i < objects.size(); ++i)
	{
	    if(funcs[i]->getHealthyIntersectTime() > 0)
	    {
		graphSortList.push_back(std::pair<GraphObject*,time_t>(objects[i],funcs[i]->getHealthyIntersectTime()));
	    }
	}
	std::sort(graphSortList.begin(),graphSortList.end(),lrSort);

	/*for(int i = 0; i < graphSortList.size(); ++i)
	{
	    std::cerr << graphSortList[i].second << std::endl;
	}*/

	for(std::vector<LayoutTypeObject*>::iterator it = _objectList.begin(); it != _objectList.end();)
	{
	    bool erase = false;
	    for(int i = 0; i < graphSortList.size(); ++i)
	    {
		if((*it) == graphSortList[i].first)
		{
		    erase = true;
		    break;
		}
	    }

	    if(erase)
	    {
		it = _objectList.erase(it);
	    }
	    else
	    {
		it++;
	    }
	}

	for(int i = graphSortList.size() - 1; i >= 0; --i)
	{
	    _objectList.insert(_objectList.begin(),graphSortList[i].first);
	}

	forceUpdate();
    }

    if(item == _syncTimeCB)
    {
	if(_syncTimeCB->getValue())
	{
	    //find the global max and min timestamps
	    _maxX = _minX = 0;
	    for(int i = 0; i < _objectList.size(); i++)
	    {
		TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
		if(!tro)
		{
		    continue;
		}

		time_t value = tro->getMaxTimestamp();
		if(value)
		{
		    if(!_maxX || value > _maxX)
		    {
			_maxX = value;
		    }
		}

		value = tro->getMinTimestamp();
		if(value)
		{
		    if(!_minX || value < _minX)
		    {
			_minX = value;
		    }
		}
	    }

	    _currentMaxX = _maxX;
	    _currentMinX = _minX;

	    if(_maxX && _minX)
	    {
		for(int i = 0; i < _objectList.size(); i++)
		{
		    TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
		    if(!tro)
		    {
			continue;
		    }

		    tro->setGraphDisplayRange(_minX,_maxX);
		}
	    }

	    if(_zoomCB->getValue())
	    {
		for(int i = 0; i < _objectList.size(); i++)
		{
		    _perGraphActiveHand[i] = -1;
		    _perGraphActiveHandType[i] = TrackerBase::INVALID;
		    _objectList[i]->setBarVisible(false);
		}
	    }

	    // sync grouped graph range
	    float dataMin = FLT_MAX;
	    float dataMax = FLT_MIN;

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		ValueRangeObject * vro = dynamic_cast<ValueRangeObject *>(_objectList[i]);
		if(!vro)
		{
		    continue;
		}

		float temp = vro->getGraphDisplayRangeMax();
		if(temp > dataMax)
		{
		    dataMax = temp;
		}
		temp = vro->getGraphDisplayRangeMin();
		if(temp < dataMin)
		{
		    dataMin = temp;
		}
	    }

	    //dataMin = 1.16434e-05;
	    //dataMin = 8.71397e-05;

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		ValueRangeObject * vro = dynamic_cast<ValueRangeObject *>(_objectList[i]);
		if(!vro)
		{
		    continue;
		}

		vro->setGraphDisplayRange(dataMin,dataMax);
	    }

            //std::cerr << "dataMin: " << dataMin << " max: " << dataMax << std::endl;

	    float xMax = FLT_MIN;
	    float xMin = FLT_MAX;
	    float zMax = FLT_MIN;
	    float zMin = FLT_MAX;

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		LogValueRangeObject * lvro = dynamic_cast<LogValueRangeObject *>(_objectList[i]);
		if(!lvro)
		{
		    continue;
		}

		float temp = lvro->getGraphXDisplayRangeMax();
		if(temp > xMax)
		{
		    xMax = temp;
		}
		temp = lvro->getGraphXDisplayRangeMin();
		if(temp < xMin)
		{
		    xMin = temp;
		}

		temp = lvro->getGraphZDisplayRangeMax();
		if(temp > zMax)
		{
		    zMax = temp;
		}
		temp = lvro->getGraphZDisplayRangeMin();
		if(temp < zMin)
		{
		    zMin = temp;
		}
	    }

	    // sorta a hack for now
	    xMax = zMax = 1.0;

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		LogValueRangeObject * lvro = dynamic_cast<LogValueRangeObject *>(_objectList[i]);
		if(!lvro)
		{
		    continue;
		}

		lvro->setGraphXDisplayRange(xMin,xMax);
		lvro->setGraphZDisplayRange(zMin,zMax);
	    }
	}
	else
	{
	    for(int i = 0; i < _objectList.size(); i++)
	    {
		TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
		if(!tro)
		{
		    continue;
		}

		tro->resetGraphDisplayRange();
	    }

	    if(_zoomCB->getValue())
	    {
		_activeHand = -1;
		_activeHandType = TrackerBase::INVALID;
		for(int i = 0; i < _objectList.size(); i++)
		{
		    _objectList[i]->setBarVisible(false);
		}
	    }

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		ValueRangeObject * vro = dynamic_cast<ValueRangeObject *>(_objectList[i]);
		if(!vro)
		{
		    continue;
		}

		vro->resetGraphDisplayRange();
	    }

	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		LogValueRangeObject * lvro = dynamic_cast<LogValueRangeObject*>(_objectList[i]);
		if(!lvro)
		{
		    continue;
		}

		lvro->resetGraphDisplayRange();
	    }
	}
	return;
    }

    for(std::map<LayoutTypeObject *,cvr::MenuButton *>::iterator it = _deleteButtonMap.begin(); it != _deleteButtonMap.end(); it++)
    {
	if(it->second == item)
	{
	    it->first->closeMenu();
	    removeGraphObject(it->first);
	    return;
	}
    }

    if(item == _removeUnselected)
    {
	std::vector<LayoutTypeObject*> removeList;

	for(int i = 0; i < _objectList.size(); ++i)
	{
	    SelectableObject * so = dynamic_cast<SelectableObject*>(_objectList[i]);
	    if(!so || !so->isSelected())
	    {
		removeList.push_back(_objectList[i]);
	    }
	}

	GraphGlobals::setDeferUpdate(true);

	for(int i = 0; i < removeList.size(); ++i)
	{
	    removeGraphObject(removeList[i]);
	}

	GraphGlobals::setDeferUpdate(false);
	forceUpdate();

	return;
    }

    if(item == _selectAll)
    {
	for(int i = 0; i < _objectList.size(); ++i)
	{
	    SelectableObject * so = dynamic_cast<SelectableObject*>(_objectList[i]);
	    if(so)
	    {
		so->setSelected(true);
		if(so->getMenuItem())
		{
		    _objectList[i]->menuCallback(so->getMenuItem());
		}
	    }
	}
    }

    if(item == _selectNone)
    {
	for(int i = 0; i < _objectList.size(); ++i)
	{
	    SelectableObject * so = dynamic_cast<SelectableObject*>(_objectList[i]);
	    if(so)
	    {
		so->setSelected(false);
		if(so->getMenuItem())
		{
		    _objectList[i]->menuCallback(so->getMenuItem());
		}
	    }
	}
    }

    FPTiledWallSceneObject::menuCallback(item);
}

bool GraphLayoutObject::processEvent(InteractionEvent * event)
{
    ValuatorInteractionEvent * vie = event->asValuatorEvent();
    if(vie)
    {
	if(_zoomCB->getValue())
	{
	    if(_syncTimeCB->getValue())
	    {
		if(_objectList.size() && vie->getHand() == _activeHand)
		{
		    double pos = _objectList[0]->getBarPosition();
		    bool found = false;

		    for(int i = 0; i < _objectList.size(); i++)
		    {
			TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
			if(tro)
			{
			    pos = _objectList[i]->getBarPosition();
			    found = true;
			    break;
			}
		    }

		    if(found)
		    {
			time_t change = (time_t)(difftime(_currentMaxX,_currentMinX)*0.01);
			if(change <= 0.0 && vie->getValue() < 0.0)
			{
			    time_t diff = difftime(_currentMaxX,_currentMinX);
			    change = std::max(diff >> 1, (time_t)1);
			}

			_currentMinX += change * pos * ((double)vie->getValue());
			_currentMaxX -= change * (1.0 - pos) * ((double)vie->getValue());
			_currentMinX = std::max(_currentMinX,_minX);
			_currentMaxX = std::min(_currentMaxX,_maxX);

			for(int i = 0; i < _objectList.size(); i++)
			{
			    TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
			    if(!tro)
			    {
				continue;
			    }

			    tro->setGraphDisplayRange(_currentMinX,_currentMaxX);
			}
			return true;
		    }
		}
	    }
	    else
	    {
		for(int i = 0; i < _objectList.size(); i++)
		{
		    TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
		    if(!tro)
		    {
			continue;
		    }

		    if(_perGraphActiveHand[i] == vie->getHand())
		    {
			time_t currentStart,currentEnd;
			tro->getGraphDisplayRange(currentStart,currentEnd);
			double pos = _objectList[i]->getBarPosition();

			time_t change = (time_t)(difftime(currentEnd,currentStart)*0.03);
			if(change <= 0.0 && vie->getValue() < 0.0)
			{
			    time_t diff = difftime(currentEnd,currentStart);
			    change = std::max(diff >> 2, (time_t)1);
			}

			currentStart += change * pos * ((double)vie->getValue());
			currentEnd -= change * (1.0 - pos) * ((double)vie->getValue());
			currentStart = std::max(currentStart,tro->getMinTimestamp());
			currentEnd = std::min(currentEnd,tro->getMaxTimestamp());
			tro->setGraphDisplayRange(currentStart,currentEnd);

			return true;
		    }
		}
	    }
	}
    }

    return FPTiledWallSceneObject::processEvent(event);
}

void GraphLayoutObject::enterCallback(int handID, const osg::Matrix &mat)
{
}

void GraphLayoutObject::updateCallback(int handID, const osg::Matrix &mat)
{
    if(!_zoomCB->getValue())
    {
	return;
    }

    // not using tracked wand at the moment, keeps it from holding the interaction
    if(TrackingManager::instance()->getHandTrackerType(handID) == TrackerBase::TRACKER)
    {
	return;
    }

    if(_syncTimeCB->getValue())
    {
	if(handID != _activeHand && _activeHandType <= TrackingManager::instance()->getHandTrackerType(handID))
	{
	    return;
	}

	bool hit = false;
	osg::Vec3 graphPoint;
	for(int i = 0; i < _objectList.size(); i++)
	{
	    if(!_objectList[i]->getGraphSpacePoint(mat,graphPoint) || graphPoint.z() < 0 || graphPoint.z() > 1.0)
	    {
		continue;
	    }

	    hit = true;
	    break;
	}

	if(!hit && _activeHand == handID)
	{
	    _activeHand = -1;
	    _activeHandType = TrackerBase::INVALID;
	    for(int i = 0; i < _objectList.size(); i++)
	    {
		_objectList[i]->setBarVisible(false);
	    }
	    return;
	}
	else if(!hit)
	{
	    return;
	}

	graphPoint.x() = std::max(graphPoint.x(),0.0f);
	graphPoint.x() = std::min(graphPoint.x(),1.0f);

	if(_activeHand == -1)
	{
	    for(int i = 0; i < _objectList.size(); i++)
	    {
		_objectList[i]->setBarVisible(true);
	    }
	}

	if(_activeHand != handID)
	{
	    _activeHand = handID;
	    _activeHandType = TrackingManager::instance()->getHandTrackerType(handID);
	}

	for(int i = 0; i < _objectList.size(); i++)
	{
	    _objectList[i]->setBarPosition(graphPoint.x());
	}
    }
    else
    {
	for(int i = 0; i < _objectList.size(); i++)
	{
	    if(_perGraphActiveHand[i] != handID && _perGraphActiveHandType[i] <= TrackingManager::instance()->getHandTrackerType(handID))
	    {
		continue;
	    }

	    osg::Vec3 graphPoint;
	    if(!_objectList[i]->getGraphSpacePoint(mat,graphPoint) || graphPoint.z() < 0 || graphPoint.z() > 1.0)
	    {
		if(handID == _perGraphActiveHand[i])
		{
		    _perGraphActiveHand[i] = -1;
		    _perGraphActiveHandType[i] = TrackerBase::INVALID;
		    _objectList[i]->setBarVisible(false);
		}
		continue;
	    }

	    graphPoint.x() = std::max(graphPoint.x(),0.0f);
	    graphPoint.x() = std::min(graphPoint.x(),1.0f);

	    if(_perGraphActiveHand[i] == -1)
	    {
		_objectList[i]->setBarVisible(true);
	    }

	    if(_perGraphActiveHand[i] != handID)
	    {
		_perGraphActiveHand[i] = handID;
		_perGraphActiveHandType[i] = TrackingManager::instance()->getHandTrackerType(handID);
	    }

	    _objectList[i]->setBarPosition(graphPoint.x());
	}
    }
}

void GraphLayoutObject::leaveCallback(int handID)
{
    if(_syncTimeCB->getValue())
    {
	if(_activeHand == handID)
	{
	    _activeHand = -1;
	    _activeHandType = TrackerBase::INVALID;
	    for(int i = 0; i < _objectList.size(); i++)
	    {
		_objectList[i]->setBarVisible(false);
	    }
	}
    }
    else
    {
	for(int i = 0; i < _objectList.size(); i++)
	{
	    if(_perGraphActiveHand[i] == handID)
	    {
		_perGraphActiveHand[i] = -1;
		_perGraphActiveHandType[i] = TrackerBase::INVALID;
		_objectList[i]->setBarVisible(false);
	    }
	}
    }
}

void GraphLayoutObject::setChartLinearRegression(bool lr)
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	GraphObject * go = dynamic_cast<GraphObject *>(_objectList[i]);
	if(go)
	{
	    go->setLinearRegression(lr);
	}
    }
}

void GraphLayoutObject::setChartDisplayType(GraphDisplayType displayType)
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	GraphObject * go = dynamic_cast<GraphObject *>(_objectList[i]);
	if(go)
	{
	    go->setDisplayType(displayType);
	}
    }
}

void GraphLayoutObject::setSingleMicrobeLogScale(bool logScale)
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	SingleMicrobeObject * smo = dynamic_cast<SingleMicrobeObject*>(_objectList[i]);
	if(smo)
	{
	    smo->setLogScale(logScale);
	}
    }
    menuCallback(_syncTimeCB);
}

void GraphLayoutObject::setSingleMicrobeShowStdDev(bool value)
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	SingleMicrobeObject * smo = dynamic_cast<SingleMicrobeObject*>(_objectList[i]);
	if(smo)
	{
	    smo->setShowStdDev(value);
	}
    }
}

void GraphLayoutObject::setScatterLogScale(bool logScale)
{
    for(int i = 0; i < _objectList.size(); ++i)
    {
	MicrobeScatterGraphObject * sgo = dynamic_cast<MicrobeScatterGraphObject*>(_objectList[i]);
	if(sgo)
	{
	    sgo->setLogScale(logScale);
	}
    }
    menuCallback(_syncTimeCB);
}

void GraphLayoutObject::forceUpdate()
{
    updateLayout();
}

void GraphLayoutObject::setAllGraphMinTime()
{
    if(!_currentMinX)
    {
	return;
    }

    struct tm tempTime = *localtime(&_currentMinX);
    //std::cerr << "Year: " << tempTime.tm_year << std::endl;
    if(tempTime.tm_year < 96) 
    {
	tempTime.tm_year = 96;
	tempTime.tm_mon = 10;
	_currentMinX = mktime(&tempTime);
    }

    for(int i = 0; i < _objectList.size(); i++)
    {
	TimeRangeObject * tro = dynamic_cast<TimeRangeObject *>(_objectList[i]);
	if(!tro)
	{
	    continue;
	}

	tro->setGraphDisplayRange(_currentMinX,_currentMaxX);
    }
}

void GraphLayoutObject::makeGeometry()
{
    _layoutGeode = new osg::Geode();
    addChild(_layoutGeode);

    osg::Vec4 color(0.0,0.0,0.0,1.0);

    float halfw = (_width * 1.05) / 2.0;
    float halfh = (_height * 1.05) / 2.0;

    osg::Geometry * geo = new osg::Geometry();
    _verts = new osg::Vec3Array();
    _verts->push_back(osg::Vec3(halfw,2,halfh+(_height*0.1)));
    _verts->push_back(osg::Vec3(halfw,2,-halfh));
    _verts->push_back(osg::Vec3(-halfw,2,-halfh));
    _verts->push_back(osg::Vec3(-halfw,2,halfh+(_height*0.1)));

    // Title line
    _verts->push_back(osg::Vec3(-_width / 2.0,1.5,halfh));
    _verts->push_back(osg::Vec3(_width / 2.0,1.5,halfh));

    geo->setVertexArray(_verts);

    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
	    osg::PrimitiveSet::QUADS,0);

    ele->push_back(0);
    ele->push_back(1);
    ele->push_back(2);
    ele->push_back(3);
    geo->addPrimitiveSet(ele);

    ele = new osg::DrawElementsUInt(
	    osg::PrimitiveSet::LINES,0);

    ele->push_back(4);
    ele->push_back(5);
    geo->addPrimitiveSet(ele);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(osg::Vec4(1.0,1.0,1.0,1.0));
    colors->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    geo->setColorArray(colors);
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    _layoutGeode->addDrawable(geo);

    osg::StateSet * stateset = _layoutGeode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    setTitle(getName());
}

void GraphLayoutObject::makeKeys()
{
    _patientKey = new GraphKeyObject("Patinet Key",false,false,false,false,false);

    std::vector<osg::Vec4> colors;
    std::vector<std::string> labels;

    //labels.push_back("Smarr");
    labels.push_back("LS");
    labels.push_back("Crohns");
    labels.push_back("UC");
    labels.push_back("Healthy");

    for(int i = 0; i < labels.size(); ++i)
    {
	colors.push_back(ColorGenerator::makeColor(i,labels.size()));
    }
    
    _patientKey->setCallbackType(KC_PATIENT_TYPE);
    _patientKey->setKeys(colors,labels);

    _phylumKey = new GraphKeyObject("Phylum Key",false,false,false,false,false);
    
    colors.clear();
    labels.clear();

    for(std::map<std::string,osg::Vec4>::const_iterator it = GraphGlobals::getPhylumColorMap().begin(); it != GraphGlobals::getPhylumColorMap().end(); ++it)
    {
	colors.push_back(it->second);
	labels.push_back(it->first);
    }

    labels.push_back("Other");
    colors.push_back(GraphGlobals::getDefaultPhylumColor());

    _phylumKey->setCallbackType(KC_PHYLUM);
    _phylumKey->setKeys(colors,labels);
}

void GraphLayoutObject::updateGeometry()
{
    float halfw = (_width * 1.05) / 2.0;
    float halfh = (_height * 1.05) / 2.0;

    _verts->at(0) = osg::Vec3(halfw,2,halfh+(_height*0.1));
    _verts->at(1) = osg::Vec3(halfw,2,-halfh);
    _verts->at(2) = osg::Vec3(-halfw,2,-halfh);
    _verts->at(3) = osg::Vec3(-halfw,2,halfh+(_height*0.1));

    // Title line
    _verts->at(4) = osg::Vec3(-_width / 2.0,1.5,halfh);
    _verts->at(5) = osg::Vec3(_width / 2.0,1.5,halfh);

    _verts->dirty();

    float targetWidth = _width * 0.9;
    float targetHeight = _height * 0.1 * 0.9;
    _text->setCharacterSize(1.0);
    osg::BoundingBox bb = _text->getBoundingBox();
    float hsize = targetHeight / (bb.zMax() - bb.zMin());
    float wsize = targetWidth / (bb.xMax() - bb.xMin());
    _text->setCharacterSize(std::min(hsize,wsize));
    _text->setPosition(osg::Vec3(0,1.5,halfh+(_height*0.05)));

    for(int i = 0; i < _layoutGeode->getNumDrawables(); i++)
    {
	_layoutGeode->getDrawable(i)->dirtyDisplayList();
    }
}

void GraphLayoutObject::updateLayout()
{
    if(GraphGlobals::getDeferUpdate())
    {
	return;
    }

    int totalGraphs = _objectList.size();

    if(!totalGraphs)
    {
	return;
    }

    float lineHeightMult = 0.06;
    float maxLineMult = 0.33;
    float layoutLineHeight;
    float layoutGraphHeight;

    layoutLineHeight = _height * std::min(((float)_lineObjectList.size())*lineHeightMult,maxLineMult);

    layoutGraphHeight = _height - layoutLineHeight;

    float graphWidth, graphHeight;

    if(totalGraphs >= _maxRows)
    {
	graphHeight = layoutGraphHeight / (float)_maxRows;
    }
    else
    {
	graphHeight = layoutGraphHeight / (float)totalGraphs;
    }

    float div = (float)((totalGraphs-1) / _maxRows);
    div += 1.0;

    graphWidth = _width / div;

    float posX = -(_width*0.5) + (graphWidth*0.5);
    float posZ = (_height*0.5) - (graphHeight*0.5);

    int microbeGraphCount = 0;

    for(int i = 0; i < _objectList.size(); i++)
    {
	_objectList[i]->setGraphSize(graphWidth,graphHeight);
	_objectList[i]->setPosition(osg::Vec3(posX,0,posZ));
	posZ -= graphHeight;
	if(posZ < -(_height*0.5) + layoutLineHeight)
	{
	    posX += graphWidth;
	    posZ = (_height*0.5) - (graphHeight*0.5);
	}

	if(dynamic_cast<MicrobeGraphObject*>(_objectList[i]))
	{
	    microbeGraphCount++;
	}
    }

    if(_lineObjectList.size())
    {
	float lineHeight = layoutLineHeight / ((float)_lineObjectList.size());
	posZ = (-_height / 2.0) + layoutLineHeight - (lineHeight / 2.0);
	for(int i = 0; i < _lineObjectList.size(); ++i)
	{
	    _lineObjectList[i]->setSize(_width,lineHeight);
	    _lineObjectList[i]->setPosition(osg::Vec3(0,0,posZ));

	    posZ -= lineHeight;
	}
    }

    if(microbeGraphCount)
    {
	int currentCount = 0;
	for(int i = 0; i < _objectList.size(); ++i)
	{
	    MicrobeGraphObject * mgo = dynamic_cast<MicrobeGraphObject*>(_objectList[i]);
	    if(mgo)
	    {
		mgo->setColor(ColorGenerator::makeColor(currentCount,microbeGraphCount));
		currentCount++;
	    }
	}
    }

    for(int i = 0; i < _objectList.size(); ++i)
    {
	MicrobeSelectObject * mso = dynamic_cast<MicrobeSelectObject*>(_objectList[i]);
	if(mso)
	{
	    mso->selectMicrobes(_currentSelectedMicrobeGroup,_currentSelectedMicrobes);
	}
	setSelectionText();
    }

    for(int i = 0; i < _objectList.size(); ++i)
    {
	PatientSelectObject * pso = dynamic_cast<PatientSelectObject*>(_objectList[i]);
	if(pso)
	{
	    //pso->selectPatients(_currentSelectedPatientGroup,_currentSelectedPatients);
	    pso->selectPatients(_currentSelectedPatientMap);
	}
    }
}

void GraphLayoutObject::checkLineRefs()
{
    std::vector<LayoutLineObject*> removeList;

    for(int i = 0; i < _lineObjectList.size(); ++i)
    {
	if(!_lineObjectList[i]->hasRef())
	{
	    removeList.push_back(_lineObjectList[i]);
	}
    }

    for(int i = 0; i < removeList.size(); ++i)
    {
	removeLineObject(removeList[i]);
    }
}

void GraphLayoutObject::setTitle(std::string title)
{
    if(!_layoutGeode)
    {
	return;
    }

    if(_text)
    {
	_layoutGeode->removeDrawable(_text);
    }

    float targetWidth = _width * 0.9;
    float targetHeight = _height * 0.06 * 0.9;
    float halfh = (_height * 1.05) / 2.0;

    _text = GraphGlobals::makeText(title,osg::Vec4(1.0,1.0,1.0,1.0));

    osg::BoundingBox bb = _text->getBoundingBox();
    float hsize = targetHeight / (bb.zMax() - bb.zMin());
    float wsize = targetWidth / (bb.xMax() - bb.xMin());
    _text->setCharacterSize(std::min(hsize,wsize));
    _text->setAxisAlignment(osgText::Text::XZ_PLANE);

    _text->setPosition(osg::Vec3(0,1.5,halfh+(_height*0.03)));

    _layoutGeode->addDrawable(_text);
}

void GraphLayoutObject::setSelectionText()
{
    if(_currentSelectedMicrobeGroup != "")
    {
	std::stringstream ss;
	std::string spacer = "   ";
	if(!_currentSelectedMicrobes.size())
	{
	    ss << _currentSelectedMicrobeGroup << " Totals" << std::endl;
	    // group stats
	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		MicrobeSelectObject * mso = dynamic_cast<MicrobeSelectObject *>(_objectList[i]);
		if(mso)
		{
		    for(int j = 0; j < mso->getNumDisplayValues(); ++j)
		    {
			ss << spacer << mso->getDisplayLabel(j) << ": ";
			float val = mso->getGroupValue(_currentSelectedMicrobeGroup,j);
			if(val >= 0.0)
			{
			    ss << val << std::endl;
			}
			else
			{
			    ss << "~" << std::endl;
			}
		    }
		}
	    }
	}
	else if(_currentSelectedMicrobes.size() != 1)
	{
	    ss << _currentSelectedMicrobeGroup << " Selected Totals" << std::endl;
	    // group stats
	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		MicrobeSelectObject * mso = dynamic_cast<MicrobeSelectObject *>(_objectList[i]);
		if(mso)
		{
		    for(int k = 0; k < mso->getNumDisplayValues(); ++k)
		    {
			ss << spacer << mso->getDisplayLabel(k) << ": ";
			float val = 0.0;

			for(int j = 0; j < _currentSelectedMicrobes.size(); ++j)
			{
			    float tval = mso->getMicrobeValue(_currentSelectedMicrobeGroup,_currentSelectedMicrobes[j],k);
			    if(tval >= 0.0)
			    {
				val += tval;
			    }
			}
			ss << val << std::endl;
		    }
		}
	    }
	}
	else
	{
	    // single microbe stats
	    ss << _currentSelectedMicrobes[0] << " Values" << std::endl;
	    // group stats
	    for(int i = 0; i < _objectList.size(); ++i)
	    {
		MicrobeSelectObject * mso = dynamic_cast<MicrobeSelectObject *>(_objectList[i]);
		if(mso)
		{
		    for(int j = 0; j < mso->getNumDisplayValues(); ++j)
		    {
			ss << spacer << mso->getDisplayLabel(j) << ": ";
			float val = mso->getMicrobeValue(_currentSelectedMicrobeGroup,_currentSelectedMicrobes[0],j);
			if(val >= 0.0)
			{
			    ss << val << std::endl;
			}
			else
			{
			    ss << "~" << std::endl;
			}
		    }
		}
	    }
	}

	_selectionText->setText(ss.str());

	_selectionMenu->setVisible(true);
    }
    else
    {
	_selectionMenu->setVisible(false);
    }
}

bool GraphLayoutObject::loadObject(std::istream & in)
{
    char tempstr[1024];
    std::string objectType;

    do
    {
	in.getline(tempstr,1024);
	objectType = tempstr;
    }
    while(objectType.empty() && !in.eof());

    bool ret = true;

    if(objectType == "GRAPH_OBJECT")
    {
	//std::cerr << "Loading new GraphObject" << std::endl;
	LayoutTypeObject * obj = new GraphObject(FuturePatient::getDBManager(), 1000.0, 1000.0, "DataGraph", false, true, false, true, false);
	ret = obj->loadState(in);
	addGraphObject(obj);
    }
    else if(objectType == "SYMPTOM_GRAPH")
    {
	//std::cerr << "Loading new SymptomGraphObject" << std::endl;
	LayoutTypeObject * obj = new SymptomGraphObject(FuturePatient::getDBManager(), 1000.0, 1000.0, "Symptom Graph", false, true, false, true);
	ret = obj->loadState(in);
	addGraphObject(obj);
    }
    else if(objectType == "MICROBE_GRAPH")
    {
	//std::cerr << "Loading new MicrobeGraphObject" << std::endl;
	LayoutTypeObject * obj = new MicrobeGraphObject(FuturePatient::getDBManager(), 1000.0, 1000.0, "Microbe Graph", false, true, false, true);
	ret = obj->loadState(in);
	addGraphObject(obj);
    }
    else if(objectType == "MICROBE_BAR_GRAPH")
    {
	//std::cerr << "Loading new MicrobeBarGraphObject" << std::endl;
	LayoutTypeObject * obj = new MicrobeBarGraphObject(FuturePatient::getDBManager(), 1000.0, 1000.0, "Microbe Graph", false, true, false, true);
	ret = obj->loadState(in);
	addGraphObject(obj);
    }
    else if(objectType == "UNKNOWN")
    {
	std::cerr << "Warning: unknown object type" << std::endl;
	return true;
    }
    else
    {
	std::cerr << "Error: really unknown object type" << std::endl;
	return false;
    }
    return ret;
}
