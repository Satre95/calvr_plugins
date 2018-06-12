#pragma once

#include <osg/TextureCubeMap>
#include <osg/Transform>
#include <osg/ShapeDrawable>
#include <osg/Geode>
class SkyBox : public osg::Transform
{
public:
    SkyBox(int phase=1, float radius =  1000.f);
    
    SkyBox( const SkyBox& copy, osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY )
    : osg::Transform(copy, copyop) {}

    void PreFrame(float runningTime);
    
    META_Node( osg, SkyBox );
    
    void setEnvironmentMap( unsigned int unit, osg::Image* posX, osg::Image* negX,
                            osg::Image* posY, osg::Image* negY, osg::Image* posZ, osg::Image* negZ );
    
    virtual bool computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const;
    virtual bool computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const;
    
protected:
    virtual ~SkyBox() {}
    void SetupFadeUniforms();

    osg::ref_ptr<osg::Geode> mSkyGeode = nullptr;
    int mPhase;
};
