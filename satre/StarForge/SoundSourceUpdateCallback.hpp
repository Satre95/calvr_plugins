// Created by satre on 6/13/18.

#pragma once

#include <osg/Callback>
#include <OAS/OASClient.h>
#include <osg/PositionAttitudeTransform>
#include <osg/Vec3>


class SoundSourceUpdateCallback: public osg::Callback {
public:
    explicit SoundSourceUpdateCallback(oasclient::Sound * _sound = nullptr) : osg::Callback(), mSound(_sound) {}
    SoundSourceUpdateCallback(const SoundSourceUpdateCallback & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY) :
            Callback(other, copyOp), mSound(other.mSound){

    }
    virtual bool run (osg::Object *object, osg::Object *data) override {
        auto owner = dynamic_cast<osg::PositionAttitudeTransform*>(object);
        auto & pos = owner->getPosition();
        mSound->setPosition(pos.x(), pos.y(), pos.z());
    }

    META_Object(osg, SoundSourceUpdateCallback);

protected:
    oasclient::Sound * mSound;
};


