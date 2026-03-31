#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
using namespace geode::prelude;

static int64_t s_sessionStart = 0;

bool inLevel() {
    return PlayLayer::get() != nullptr;
}

class TimeTracker {
public:
    static int64_t getAccumulatedSeconds() {
        return Mod::get()->getSavedValue<int64_t>("accumulated-seconds", 0);
    }
    static void addSeconds(int64_t s) {
        Mod::get()->setSavedValue("accumulated-seconds", getAccumulatedSeconds() + s);
    }
    static std::string formatDuration(int64_t t) {
        int d=t/86400,h=(t%86400)/3600,m=(t%3600)/60,s=t%60;
        std::string out;
        if(d>0) out+=std::to_string(d)+"d ";
        if(h>0||d>0) out+=std::to_string(h)+"h ";
        out+=std::to_string(m)+"m "+std::to_string(s)+"s";
        return out;
    }
};

class SessionSaver:public CCNode{
public:
    static SessionSaver* create(){auto r=new SessionSaver();if(r&&r->init()){r->autorelease();return r;}delete r;return nullptr;}
    bool init(){if(!CCNode::init())return false;this->schedule(schedule_selector(SessionSaver::tick),30.f);return true;}
    void tick(float){if(!inLevel())return;if(s_sessionStart<=0)return;auto now=static_cast<int64_t>(std::time(nullptr));auto diff=now-s_sessionStart;if(diff>0){TimeTracker::addSeconds(diff);s_sessionStart=now;}}
};

class $modify(MyMenuLayer,MenuLayer){
    bool init(){
        if(!MenuLayer::init()) return false;
        if(!this->getChildByID("screentime-saver"_spr)){
            auto s=SessionSaver::create();
            s->setID("screentime-saver"_spr);
            this->addChild(s);
        }
        auto menu=this->getChildByID("bottom-menu");
        if(!menu) return true;

        auto tex=CCTextureCache::sharedTextureCache()->addImage("icon.png", false);
        auto spr=CircleButtonSprite::createWithTexture(tex);
        spr->setContentSize(CCSize(50,50)); // taille comme les autres boutons

        auto btn=CCMenuItemSpriteExtra::create(spr,this,menu_selector(MyMenuLayer::onTime));
        btn->setID("time-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();
        return true;
    }
    void onTime(CCObject*){
        auto t=TimeTracker::getAccumulatedSeconds();
        if(s_sessionStart>0 && inLevel())
            t+=static_cast<int64_t>(std::time(nullptr))-s_sessionStart;
        auto txt=fmt::format("Playtime: {}",TimeTracker::formatDuration(t));
        FLAlertLayer::create("Screen Time",txt.c_str(),"OK")->show();
    }
};

$on_mod(Loaded){
    s_sessionStart=static_cast<int64_t>(std::time(nullptr));
    TimeTracker::getAccumulatedSeconds();
    log::info("screen time ready");
}
