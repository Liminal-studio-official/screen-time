#include <Geode/modify/MenuLayer.hpp>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

static int64_t s_sessionStart = 0;

class TimeTracker {
public:
    static int64_t getAccumulatedSeconds() {
        return Mod::get()->getSavedValue<int64_t>("accumulated-seconds", 0);
    }

    static std::string formatDate(int64_t ts) {
        std::time_t raw = static_cast<std::time_t>(ts);
        std::tm* t = std::localtime(&raw);

        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", t);

        return std::string(buf);
    }

    static void addSeconds(int64_t s) {
        auto cur = getAccumulatedSeconds();
        Mod::get()->setSavedValue("accumulated-seconds", cur + s);
    }

    static int64_t getInstallTimeStamp() {
        auto v = Mod::get()->getSavedValue<int64_t>("install-time", 0);

        if (v == 0) {
            v = static_cast<int64_t>(std::time(nullptr));
            Mod::get()->setSavedValue("install-time", v);
        }

        return v;
    }

    static std::string formatDuration(int64_t total) {
        int d = total / 86400;
        int h = (total % 86400) / 3600;
        int m = (total % 3600) / 60;
        int s = total % 60;

        std::string out;

        if (d > 0) out += std::to_string(d) + "d ";
        if (h > 0 || d > 0) out += std::to_string(h) + "h ";

        out += std::to_string(m) + "m ";
        out += std::to_string(s) + "s";

        return out;
    }
};

class SessionSaver : public CCNode {
public:
    static SessionSaver* create() {
        auto r = new SessionSaver();

        if (r && r->init()) {
            r->autorelease();
            return r;
        }

        delete r;
        return nullptr;
    }

    bool init() {
        if (!CCNode::init()) return false;

        this->schedule(schedule_selector(SessionSaver::tick), 30.f);
        return true;
    }

    void tick(float) {
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
            auto s = SessionSaver::create();
            s->setID("screentime-saver"_spr);
            this->addChild(s);
        }

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("no bottom menu?");
            return true;
        }

        auto spr = CircleButtonSprite::create(
            CCLabelBMFont::create("Time", "bigFont.fnt"),
            CircleBaseColor::Green,
            CircleBaseSize::Medium
        );

        if (auto l = spr->getChildByType<CCLabelBMFont>(0)) {
            l->setScale(0.4f);
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(MyMenuLayer::onTime)
        );

        btn->setID("time-button"_spr);

        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onTime(CCObject*) {
        auto install = TimeTracker::getInstallTimeStamp();
        auto total = TimeTracker::getAccumulatedSeconds();

        if (s_sessionStart > 0) {
            total += static_cast<int64_t>(std::time(nullptr)) - s_sessionStart;
        }

        auto text = fmt::format(
            "Installed: {}\nPlaytime: {}",
            TimeTracker::formatDate(install),
            TimeTracker::formatDuration(total)
        );

        FLAlertLayer::create("Screen Time", text.c_str(), "OK")->show();
    }
};

$on_mod(Loaded) {
    TimeTracker::getInstallTimeStamp();

    s_sessionStart = static_cast<int64_t>(std::time(nullptr));

    log::info("screen time ready");
}
