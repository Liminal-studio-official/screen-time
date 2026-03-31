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
        Mod::get()->setSavedValue("accumulated-seconds", getAccumulatedSeconds() + sec);
    }

    static std::string formatDuration(int64_t total) {
        int days = total / 86400;
        int hours = (total % 86400) / 3600;
        int minutes = (total % 3600) / 60;
        int seconds = total % 60;

        std::string result;
        if(days) result += std::to_string(days) + "d ";
        if(hours || days) result += std::to_string(hours) + "h ";
        result += std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
        return result;
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
        int64_t now = static_cast<int64_t>(std::time(nullptr));
        int64_t diff = now - s_sessionStart;
        if(diff > 0) {
            TimeTracker::addSeconds(diff);
            s_sessionStart = now;
        }
    }

    static SessionSaver* create() {
        auto saver = new SessionSaver();
        if(saver && saver->init()) {
            saver->autorelease();
            return saver;
        }
        delete saver;
        return nullptr;
    }
};

$on_mod(Loaded) {
    s_sessionStart = static_cast<int64_t>(std::time(nullptr));
    TimeTracker::getAccumulatedSeconds();
    Mod::get()->setIcon("icon.png");
    log::info("Screen Time ready");
}

class $modify(MyMenuLayer, MenuLayer) {
public:
    bool init() {
        if(!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if(!menu) return true;

        if(!this->getChildByID("screentime-saver"_spr)) {
            auto saver = SessionSaver::create();
            saver->setID("screentime-saver"_spr);
            this->addChild(saver);
        }

        auto spr = CircleButtonSprite::create(menu, CircleBaseColor::Green, CircleBaseSize::Medium);
        spr->setString("T");
        spr->setScale(0.8f);

        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyMenuLayer::onTime));
        btn->setID("time-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onTime(CCObject*) {
        auto pl = PlayLayer::get();
        if(!pl) {
            FLAlertLayer::create("Screen Time", "Start a level to track playtime.", "OK")->show();
            return;
        }

        int64_t total = TimeTracker::getAccumulatedSeconds();
        if(s_sessionStart > 0) total += static_cast<int64_t>(std::time(nullptr)) - s_sessionStart;

        FLAlertLayer::create(
            "Screen Time",
            fmt::format("Playtime: {}", TimeTracker::formatDuration(total)).c_str(),
            "OK"
        )->show();
    }
};
