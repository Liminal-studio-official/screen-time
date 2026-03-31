#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

static int64_t g_start = 0;

static int64_t getAccum() {
    return Mod::get()->getSavedValue<int64_t>("playtime", 0);
}

static void flush() {
    if (g_start <= 0) return;
    auto now = (int64_t)std::time(nullptr);
    auto dt = now - g_start;
    if (dt < 1) return;
    Mod::get()->setSavedValue("playtime", getAccum() + dt);
    g_start = now;
}

static std::string pretty(int64_t sec) {
    int d = sec / 86400;
    int h = sec / 3600 % 24;
    int m = sec / 60 % 60;
    int s = sec % 60;

    std::string r;
    if (d) r += std::to_string(d) + "d ";
    if (d || h) r += std::to_string(h) + "h ";
    r += std::to_string(m) + "m ";
    r += std::to_string(s) + "s";
    return r;
}

class Ticker : public CCNode {
    bool init() override {
        if (!CCNode::init()) return false;
        schedule(schedule_selector(Ticker::tick), 25.f);
        return true;
    }
    void tick(float) { flush(); }
public:
    static Ticker* make() {
        auto t = new Ticker;
        if (t && t->init()) { t->autorelease(); return t; }
        CC_SAFE_DELETE(t);
        return nullptr;
    }
};

$on_mod(Loaded) {
    g_start = (int64_t)std::time(nullptr);
}

class $modify(HookedMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!getChildByID("ticker"_spr)) {
            auto tk = Ticker::make();
            tk->setID("ticker"_spr);
            addChild(tk);
        }

        auto menu = getChildByID("bottom-menu");
        if (!menu) return true;

        auto circle = CircleButtonSprite::create(
            CCLabelBMFont::create("T", "bigFont.fnt"),
            CircleBaseColor::Green,
            CircleBaseSize::Medium
        );
        circle->setScale(.75f);

        auto btn = CCMenuItemSpriteExtra::create(
            circle, this,
            menu_selector(HookedMenu::onTime)
        );
        btn->setID("playtime-btn"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onTime(CCObject*) {
        flush();
        auto t = getAccum();
        if (g_start > 0)
            t += (int64_t)std::time(nullptr) - g_start;

        FLAlertLayer::create(
            "Screen Time",
            pretty(t).c_str(),
            "OK"
        )->show();
    }
};
