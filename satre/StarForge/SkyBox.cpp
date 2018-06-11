#include <osg/Depth>
#include <osgUtil/CullVisitor>
#include <osgDB/FileUtils>
#include <cvrConfig/ConfigManager.h>
#include "SkyBox.hpp"
#include <osg/BlendFunc>
SkyBox::SkyBox(float radius)
{
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setCullingActive( false );
    
    osg::StateSet* ss = getOrCreateStateSet();
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::LESS) );
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    ss->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    ss->setAttributeAndModes(new osg::BlendFunc);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto uTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_time");
    ss->addUniform(uTime);
    uTime->set(0.f);
    // Get the fade in out time from the config
    float fadeTime = cvr::ConfigManager::getFloat("value", "Plugin.StarForge.FadeTime", 4.f);
    auto uFadeTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_fadeTime");
    ss->addUniform(uFadeTime);
    uFadeTime->set(fadeTime);

//    ss->setRenderBinDetails( 5, "RenderBin" );
    auto shadersPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");
    auto vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shadersPath + "skybox.vert"));
    auto fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shadersPath + "skybox.frag"));
    if(!vertexShader) {
        std::cerr << "ERROR: Unable to load vertex shader in " << shadersPath << std::endl;
        return;
    }
    if(!fragShader) {
        std::cerr << "ERROR: Unable to load fragment shader in " << shadersPath << std::endl;
        return;
    }
    // Setup the programmable pipeline
    auto drawProgram = new osg::Program;
    drawProgram->addShader(vertexShader);
    drawProgram->addShader(fragShader);
    ss->setAttribute(drawProgram, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    mSkyGeode = new osg::Geode;
    auto sphere = new osg::ShapeDrawable(
            new osg::Sphere(osg::Vec3(), radius));
    sphere->setUseDisplayList(false);
    mSkyGeode->addDrawable(sphere);
    mSkyGeode->setCullingActive(false);


    addChild(mSkyGeode);
}

void SkyBox::setEnvironmentMap( unsigned int unit, osg::Image* posX, osg::Image* negX,
                                osg::Image* posY, osg::Image* negY, osg::Image* posZ, osg::Image* negZ )
{
    if ( posX && posY && posZ && negX && negY && negZ )
    {
        osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_X, posX );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_X, negX );
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_Y, posY );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Y, negY );
        cubemap->setImage( osg::TextureCubeMap::POSITIVE_Z, posZ );
        cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Z, negZ );

        cubemap->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        cubemap->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
        cubemap->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );
		cubemap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
		cubemap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
        cubemap->setResizeNonPowerOfTwoHint( false );
        getOrCreateStateSet()->setTextureAttributeAndModes( unit, cubemap.get() );
    }
}

void SkyBox::PreFrame(float runningTime) {
    getOrCreateStateSet()->getUniform("u_time")->set(runningTime);
}

bool SkyBox::computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
{
    if ( nv && nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
    {
        auto * cv = dynamic_cast<osgUtil::CullVisitor*>( nv );
        matrix.preMult( osg::Matrix::translate(cv->getEyeLocal()) );
        return true;
    }
    else
        return osg::Transform::computeLocalToWorldMatrix( matrix, nv );
}

bool SkyBox::computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
{
    if ( nv && nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
    {
        auto * cv = dynamic_cast<osgUtil::CullVisitor*>( nv );
        matrix.postMult( osg::Matrix::translate(-cv->getEyeLocal()) );
        return true;
    }
    else
        return osg::Transform::computeWorldToLocalMatrix( matrix, nv );
}
