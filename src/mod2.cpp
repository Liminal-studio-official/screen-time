#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

static int64_t g_sess = 0;

static int64_t stored() {
    return Mod::get()->getSavedValue<int64_t>("playtime", 0);
}

static void flush() {
    if (g_sess < 1) return;
    int64_t n = (int64_t)std::time(nullptr);
    int64_t d = n - g_sess;
    if (d < 1) return;
    Mod::get()->setSavedValue("playtime", stored() + d);
    g_sess = n;
}

static std::string pretty(int64_t sec) {
    int y = sec / 31536000;
    int d = (sec / 86400) % 365;
    int h = (sec / 3600) % 24;
    int m = (sec / 60) % 60;
    int s = sec % 60;

    std::string out;
    if (y) out += std::to_string(y) + "y ";
    if (y || d) out += std::to_string(d) + "d ";
    if (y || d || h) out += std::to_string(h) + "h ";
    out += std::to_string(m) + "m " + std::to_string(s) + "s";
    return out;
}

class Ticker : public CCNode {
    void onTick(float) { flush(); }

    bool init() override {
        if (!CCNode::init()) return false;
        schedule(schedule_selector(Ticker::onTick), 20.f);
        return true;
    }
public:
    static Ticker* make() {
        auto r = new Ticker;
        if (r && r->init()) { r->autorelease(); return r; }
        CC_SAFE_DELETE(r);
        return nullptr;
    }
};

$on_mod(Loaded) {
    g_sess = (int64_t)std::time(nullptr);
}

class $modify(MyMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!getChildByID("tk"_spr)) {
            auto t = Ticker::make();
            t->setID("tk"_spr);
            addChild(t);
        }

        auto btm = getChildByID("bottom-menu");
        if (!btm) return true;

        auto lbl = CCLabelBMFont::create("T", "bigFont.fnt");
        lbl->setScale(.55f);

        auto circle = CircleButtonSprite::create(lbl, CircleBaseColor::Green, CircleBaseSize::Medium);

        if (auto ref = btm->getChildByID("settings-button")) {
            float sc = ref->getScale();
            circle->setScale(sc);
        }

        auto btn = CCMenuItemSpriteExtra::create(
            circle, this, menu_selector(MyMenu::showTime)
        );
        btn->setID("st-btn"_spr);
        btm->addChild(btn);
        btm->updateLayout();

        return true;
    }

    void showTime(CCObject*) {
        flush();
        int64_t tot = stored();
        if (g_sess > 0)
            tot += (int64_t)std::time(nullptr) - g_sess;

        FLAlertLayer::create(
            "Screen Time",
            pretty(tot).c_str(),
            "OK"
        )->show();
    }
};
