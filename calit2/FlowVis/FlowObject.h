#ifndef FLOW_OBJECT_H
#define FLOW_OBJECT_H

#include <cvrKernel/SceneObject.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrMenu/MenuRangeValueCompact.h>
#include <cvrMenu/MenuRangeValue.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuList.h>

#include <OpenThreads/Mutex>

#include <vector>
#include <map>

#include "FlowVis.h"

enum FlowVisType
{
    FVT_NONE=0,
    FVT_ISO_SURFACE,
    FVT_PLANE,
    FVT_PLANE_VEC,
    FVT_VORTEX_CORES,
    FVT_SEP_ATT_LINES,
    FVT_VOLUME_CUDA,
    FVT_LIC_CUDA
};

static osg::ref_ptr<osg::Texture1D> lookupColorTable = NULL;
static void initColorTable();

class FlowObject : public cvr::SceneObject, public cvr::PerContextCallback
{
    public:
        FlowObject(FlowDataSet * set, std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds=false);
        virtual ~FlowObject();

        void perFrame();
        void postFrame();
        void menuCallback(cvr::MenuItem * item);

        virtual void perContextCallback(int contextid, PerContextCallback::PCCType type) const;

    protected:
        void setFrame(int frame);
        void setVisType(FlowVisType fvt);
        void setAttribute(std::string attrib);

        FlowDataSet * _set;
        FlowVisType _visType;

        std::string _lastAttribute;
        int _currentFrame;
        double _animationTime;

        cvr::MenuRangeValueCompact * _targetFPSRV;
        cvr::MenuList * _typeList;
        cvr::MenuList * _loadedAttribList;
        cvr::MenuRangeValue * _isoMaxRV;
        cvr::MenuCheckbox * _animateCB;
        cvr::MenuRangeValue * _planeVecSpacingRV;
        cvr::MenuRangeValueCompact * _alphaRV;

        osg::ref_ptr<osg::Program> _normalProgram;
        osg::ref_ptr<osg::Program> _normalFloatProgram;
        osg::ref_ptr<osg::Program> _normalIntProgram;
        osg::ref_ptr<osg::Program> _normalVecProgram;
        osg::ref_ptr<osg::Program> _isoProgram;
        osg::ref_ptr<osg::Program> _isoVecProgram;
        osg::ref_ptr<osg::Program> _planeProgram;
        osg::ref_ptr<osg::Program> _planeVecMagProgram;
        osg::ref_ptr<osg::Program> _planeVecProgram;
        osg::ref_ptr<osg::Program> _vcoreAlphaProgram;

        osg::ref_ptr<osg::Uniform> _floatMinUni;
        osg::ref_ptr<osg::Uniform> _floatMaxUni;
        osg::ref_ptr<osg::Uniform> _intMinUni;
        osg::ref_ptr<osg::Uniform> _intMaxUni;
        osg::ref_ptr<osg::Uniform> _isoMaxUni;
        osg::ref_ptr<osg::Uniform> _planePointUni;
        osg::ref_ptr<osg::Uniform> _planeNormalUni;
        osg::ref_ptr<osg::Uniform> _planeUpUni;
        osg::ref_ptr<osg::Uniform> _planeRightUni;
        osg::ref_ptr<osg::Uniform> _planeBasisInvUni;
        osg::ref_ptr<osg::Uniform> _planeAlphaUni;
        osg::ref_ptr<osg::Uniform> _vcoreMaxUni;
        osg::ref_ptr<osg::Uniform> _vcoreMinUni;

        osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::Geometry> _surfaceGeometry;
        osg::ref_ptr<osg::Geometry> _isoGeometry;
        osg::ref_ptr<osg::Geometry> _planeGeometry;
        osg::ref_ptr<osg::Geometry> _vcoreGeometry;
        osg::ref_ptr<osg::Geometry> _slineGeometry;
        osg::ref_ptr<osg::Geometry> _alineGeometry;
        osg::ref_ptr<osg::Geometry> _volGeometry;

        osg::ref_ptr<osg::FloatArray> _volDist;
        osg::ref_ptr<osg::FloatArray> _volSlope1;
        osg::ref_ptr<osg::FloatArray> _volSlope2;
        osg::ref_ptr<osg::Vec3iArray> _volPreSortInd;
        osg::ref_ptr<osg::DrawElementsUInt> _volInd;
        mutable osg::Vec3 _volViewerPos;
        mutable osg::Vec3 _volViewerDir;
        int _volFrame;
        mutable std::map<int,bool> _volInitMap;
        mutable std::map<int,bool> _volActiveKernelMap;
        mutable OpenThreads::Mutex _volCallbackLock;
        static std::map<int,bool> _cudaContextSet;
};

#endif
