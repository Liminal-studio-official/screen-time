#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

static int64_t s_sessionStart = 0;

class TimeTracker {
public:
    static int64_t getInstallTimeStamp() {
        auto saved = Mod::get()->getSavedValue<int64_t>("install-time", 0);
        if (saved == 0) {
            saved = static_cast<int64_t>(std::time(nullptr));
            Mod::get()->setSavedValue("install-time", saved);
        }
        return saved;
    }

    static int64_t getAccumulatedSeconds() {
        return Mod::get()->getSavedValue<int64_t>("accumulated-seconds", 0);
    }

    static void addSeconds(int64_t seconds) {
        Mod::get()->setSavedValue("accumulated-seconds", getAccumulatedSeconds() + seconds);
    }

    static std::string formatDuration(int64_t totalSeconds) {
        int days = totalSeconds / 86400;
        int hours = (totalSeconds % 86400) / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int secs = totalSeconds % 60;

        std::string result;
        if (days > 0) result += std::to_string(days) + "d ";
        if (hours > 0 || days > 0) result += std::to_string(hours) + "h ";
        result += std::to_string(minutes) + "m ";
        result += std::to_string(secs) + "s";
        return result;
    }

    static std::string formatDate(int64_t timestamp) {
        std::time_t t = static_cast<std::time_t>(timestamp);
        std::tm* tm = std::localtime(&t);

        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm);
        return std::string(buffer);
    }
};

class SessionSaver : public CCNode {
public:
    static SessionSaver* create() {
        auto ret = new SessionSaver();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() {
        if (!CCNode::init()) return false;
        this->schedule(schedule_selector(SessionSaver::onTick), 30.0f);
        return true;
    }

    void onTick(float) {
        if (s_sessionStart <= 0) return;
        auto now = static_cast<int64_t>(std::time(nullptr));
        auto diff = now - s_sessionStart;
        if (diff > 0) {
            TimeTracker::addSeconds(diff);
            s_sessionStart = now;
        }
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!this->getChildByID("screentime-saver"_spr)) {
            auto saver = SessionSaver::create();
            saver->setID("screentime-saver"_spr);
            this->addChild(saver);
        }

        auto bottomMenu = this->getChildByID("bottom-menu");
        if (!bottomMenu) return true;

        auto sprite = CircleButtonSprite::create(
            CCSprite::create("icon.png"_spr),
            CircleBaseColor::Green,
            CircleBaseSize::Medium
        );

        auto btn = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(MyMenuLayer::onTime)
        );

        btn->setID("time-button"_spr);
        bottomMenu->addChild(btn);
        bottomMenu->updateLayout();

        return true;
    }

    void onTime(CCObject*) {
        auto install = TimeTracker::getInstallTimeStamp();
        auto total = TimeTracker::getAccumulatedSeconds();
        if (s_sessionStart > 0) {
            total += static_cast<int64_t>(std::time(nullptr)) - s_sessionStart;
        }

        auto msg = fmt::format(
            "Installed: {}\nPlaytime: {}",
            TimeTracker::formatDate(install),
            TimeTracker::formatDuration(total)
        );

        FLAlertLayer::create("Screen Time", msg.c_str(), "OK")->show();
    }
};

$on_mod(Loaded) {
    TimeTracker::getInstallTimeStamp();
    s_sessionStart = static_cast<int64_t>(std::time(nullptr));
    log::info("Screen Time loaded");
}
