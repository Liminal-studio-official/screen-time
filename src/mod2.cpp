#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

static int64_t sess = 0;

static int64_t load() {
    return Mod::get()->getSavedValue<int64_t>("pt", 0);
}

static void save() {
    if (!sess) return;
    auto now = (int64_t)std::time(0);
    auto dt = now - sess;
    if (dt < 1) return;
    Mod::get()->setSavedValue("pt", load() + dt);
    sess = now;
}


static std::string pretty(int64_t s) {
    int h = s / 3600;
    int m = (s / 60) % 60;
    int sec = s % 60;
    
    std::string r = "";
    if (h) r += std::to_string(h) + "h ";
    r += std::to_string(m) + "m ";
    r += std::to_string(sec) + "s";
    return r;
}

class Tick : public CCNode {
public:
    static Tick* create() {
        auto t = new Tick;
        if (t && t->init()) {
            t->autorelease();
            return t;
        }
        delete t;
        return nullptr;
    }
    
    bool init() {
        if (!CCNode::init()) return false;
        schedule(schedule_selector(Tick::upd), 30.f);
        return true;
    }
    
    void upd(float) { save(); }
};

$on_mod(Loaded) {
    sess = (int64_t)std::time(0);
}

// Nom explicite pour la classe modifiée
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        if (!getChildByID("t"_spr)) {
            auto tk = Tick::create();
            tk->setID("t"_spr);
            addChild(tk);
        }
        
        auto menu = getChildByID("bottom-menu");
        if (!menu) return true;
        
        auto lbl = CCLabelBMFont::create("T", "bigFont.fnt");
        auto circ = CircleButtonSprite::create(
            lbl, CircleBaseColor::Green, CircleBaseSize::Medium
        );
        circ->setScale(1.1f);
        
        auto btn = CCMenuItemSpriteExtra::create(
            circ, this, menu_selector(MyMenuLayer::onTime)  
        );
        btn->setID("tbtn"_spr);
        menu->addChild(btn);
        menu->updateLayout();
        
        return true;
    }
    
    void onTime(CCObject*) {
        save();
        auto total = load();
        if (sess > 0)
            total += (int64_t)std::time(0) - sess;
        
        FLAlertLayer::create(
            "Temps de jeu",
            pretty(total).c_str(),  
            "OK"
        )->show();
    }
};
