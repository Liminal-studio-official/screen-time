#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <fmt/format.h>
using namespace geode::prelude;

static int64_t s_sessionStart = 0;

class TimeTracker {
public:
    static int64_t getAccumulatedSeconds() {
        return Mod::get()->getSavedValue<int64_t>("accumulated-seconds", 0);
    }

    static void addSeconds(int64_t sec) {
        int64_t nowValue = getAccumulatedSeconds();
        Mod::get()->setSavedValue("accumulated-seconds", nowValue + sec);
    }

    static std::string formatDuration(int64_t t) {
        int d = t / 86400;
        int h = (t % 86400) / 3600;
        int m = (t % 3600) / 60;
        int s = t % 60;
        std::string out;
        if(d) out += std::to_string(d) + "d ";
        if(h || d) out += std::to_string(h) + "h ";
        out += std::to_string(m) + "m " + std::to_string(s) + "s";
        return out;
    }
};

class SessionSaver : public CCNode {
public:
    bool init() {
        if(!CCNode::init()) return false;
        this->schedule(schedule_selector(SessionSaver::tick), 30.f);
        return true;
    }

    void tick(float) {
        auto pl = PlayLayer::get();
        if(!pl || s_sessionStart <= 0) return;

        auto now = static_cast<int64_t>(std::time(nullptr));
        auto diff = now - s_sessionStart;
        if(diff > 0) {
            TimeTracker::addSeconds(diff);
            s_sessionStart = now;
        }
    }

    static SessionSaver* create() {
        auto s = new SessionSaver();
        if(s && s->init()) { s->autorelease(); return s; }
        delete s;
        return nullptr;
    }
};

$on_mod(Loaded){
    s_sessionStart = static_cast<int64_t>(std::time(nullptr));
    TimeTracker::getAccumulatedSeconds();
    log::info("Screen Time ready");
}

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if(!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if(!menu) return true;

        if(!this->getChildByID("screentime-saver"_spr)) {
            auto saver = SessionSaver::create();
            saver->setID("screentime-saver"_spr);
            this->addChild(saver);
        }

        auto tex = CCTextureCache::sharedTextureCache()->addImage("icon.png", false);
        if(!tex) return true;

        auto spr = CircleButtonSprite::create(menu, CircleBaseColor::Green, CircleBaseSize::Medium);
        spr->setTexture(tex);
        spr->setScale(0.7f);

        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyMenuLayer::onTime));
        btn->setID("time-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onTime(CCObject*) {
        auto pl = PlayLayer::get();
        if(!pl) {
            FLAlertLayer::create("Screen Time","Start a level to track playtime.","OK")->show();
            return;
        }

        auto t = TimeTracker::getAccumulatedSeconds();
        if(s_sessionStart > 0) t += static_cast<int64_t>(std::time(nullptr)) - s_sessionStart;

        FLAlertLayer::create(
            "Screen Time",
            fmt::format("Playtime: {}", TimeTracker::formatDuration(t)).c_str(),
            "OK"
        )->show();
    }
};
