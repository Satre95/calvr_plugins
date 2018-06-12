#include <osg/Depth>
#include <osgUtil/CullVisitor>
#include <osgDB/FileUtils>
#include <cvrConfig/ConfigManager.h>
#include "SkyBox.hpp"
#include <osg/BlendFunc>
#include <sstream>
#include "GlobalParameters.hpp"
using namespace cvr;

SkyBox::SkyBox(int phase, float radius): mPhase(phase)
{
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setCullingActive( false );
    
    osg::StateSet* stateset = getOrCreateStateSet();
    stateset->setAttributeAndModes( new osg::Depth(osg::Depth::LESS) );
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    stateset->setAttributeAndModes(new osg::BlendFunc);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto uTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_time");
    stateset->addUniform(uTime);
    uTime->set(0.f);
    // Get the fade in out time from the config
    float fadeTime = cvr::ConfigManager::getFloat("value", "Plugin.StarForge.FadeTime", 4.f);
    auto uFadeTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_fadeTime");
    stateset->addUniform(uFadeTime);
    uFadeTime->set(fadeTime);

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
    stateset->setAttribute(drawProgram, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    SetupFadeUniforms();

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

void SkyBox::SetupFadeUniforms() {
    auto stateset = getOrCreateStateSet();
    std::stringstream prefix;
    prefix << params::gPluginConfigPrefix + "Phase";
    prefix << std::to_string(mPhase);
    prefix << ".";
    std::string fullPrefix = prefix.str();

    float fadeInTime = ConfigManager::getFloat(fullPrefix + "Fades.FadeInTime");
    float fadeInDuration = ConfigManager::getFloat(fullPrefix + "Fades.FadeInDuration");
    float fadeOutTime = ConfigManager::getFloat(fullPrefix + "Fades.FadeOutTime");
    float fadeOutDuration = ConfigManager::getFloat(fullPrefix + "Fades.FadeOutDuration");

    auto uFadeInTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_phaseFadeInfo.fadeInTime");
    auto uFadeInDuration = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_phaseFadeInfo.fadeInDuration");
    auto uFadeOutTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_phaseFadeInfo.fadeOutTime");
    auto uFadeOutDuration = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_phaseFadeInfo.fadeOutDuration");

    uFadeInTime->set(fadeInTime); uFadeInDuration->set(fadeInDuration);
    uFadeOutTime->set(fadeOutTime); uFadeOutDuration->set(fadeOutDuration);

    stateset->addUniform(uFadeInTime); stateset->addUniform(uFadeInDuration);
    stateset->addUniform(uFadeOutTime); stateset->addUniform(uFadeOutDuration);
}