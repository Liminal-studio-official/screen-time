#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
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
  auto totalSeconds = getAccumulatedSeconds() + seconds;
  Mod::get()->setSavedValue("accumulated-seconds", totalSeconds);
}
static std::string formatDuration(int64_t totalSeconds) {
  int days = totalSeconds / 86400;
  int hours = (totalSeconds % 86400) / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int secs = totalSeconds % 60;
  std::string result;
  if (days > 0) {
    result += std::to_string(days) + "d ";
  }
  if (hours > 0 || days > 0) {
    result += std::to_string(hours) + "h ";
  }
  result += std::to_string(minutes) + "m ";
  result += std::to_string(secs) + "s ";
  return result;
}
static std::string formatDate(int64_t timestamp){
  std::time_t time = static_cast<std::time_t>(timestamp);
  std::tm* tm = std::localtime(&time);
  char buffer[64];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm);
  return std::string(buffer);
  }
};

$on_mod(Loaded) {
  TimeTracker::getInstallTimeStamp();
  s_sessionStart = static_cast<int64_t>(std::time(nullptr));
  log::info("Screen Time loaded. Session started.");
}

$on_mod(Disabled) {
  if (s_sessionStart > 0) {
    auto now = static_cast<int64_t>(std::time(nullptr));
    auto sessionDuration = now - s_sessionStart;
    if (sessionDuration > 0)
      TimeTracker::addSeconds(sessionDuration);
    }
    s_sessionStart = 0;
  log::info("Screen Time unloaded. session saved.");
}

class SessionSaver : public CCNode {
public: 
    static SessionSaver* create () {
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
void onTick(float dt) {
  if (s_sessionStart <= 0) return;
  auto now = static_cast<int64_t>(std::time(nullptr));
  auto sessionDuration = now - s_sessionStart;
  if (sessionDuration > 0) {
    TimeTracker::addSeconds(sessionDuration);
    s_sessionStart = now;
  }
}
};

class TimePopup : public geode::Popup<> {
protected:
    bool setup() override {
      this->setTitle("Screen Time");
      auto winSize = m_mainLayer->getContentSize();
      auto installTime = TimeTracker::getInstallTimeStamp();
      auto accumulated = TimeTracker::getAccumulatedSeconds();
      int64_t currentExtra = 0;
      if (s_sessionStart > 0){
        auto now = static_cast<int64_t>(std::time(nullptr));
        currentExtra = now - s_sessionStart;
        if (currentExtra < 0) currentExtra = 0;
      }
    auto totalPlaytime = accumulated + currentExtra;
    auto now = static_cast<int64_t>(std::time(nullptr));
    auto timeSinceInstall = now - installTime;

    auto installLabel = CCLabelBMFont::create(
      fmt::format("Installed: {}", TimeTracker::formatDate(installTime)).c_str(),
      "bigFont.fnt"
  );
    installLabel->setScale(0.35f);
    installLabel->setPosition(winSize / 2 + ccp(0, 20));
    m_mainLayer->addChild(installLabel);

    auto playtimeLabel = CCLabelBMFont::create(
        fmt::format("Playtime: {}", TimeTracker::formatDuration(totalPlaytime)).c_str(),
        "bigFont.fnt"
    );
    playtimeLabel->setScale(0.35f);
    playtimeLabel->setPosition(winSize / 2 + ccp(0, 0));
    m_mainLayer->addChild(playtimeLabel);
        
    auto sinceInstallLabel = CCLabelBMFont::create(
        fmt::format("Time since install: {}",
        TimeTracker::formatDuration(timeSinceInstall)).c_str(),
        "bigFont.fnt"
     );
      sinceInstallLabel->setScale(0.35f);
      sinceInstallLabel->setPosition(winSize / 2 + ccp(0, -20));
      m_mainLayer->addChild(sinceInstallLabel);
        
      return true;
    }
public:
    static TimePopup* create() {
        auto ret = new TimePopup();
        if (ret && ret->initAnchored(300.f, 200.f)){
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(MyMenuLayer, MenuLayer){
    bool init() {
        if (!MenuLayer::init()) return false;
        if (!this->getChildByID("screentime-saver"_spr)){
            auto saver = SessionSaver::create();
            saver->setID("screentime-saver"_spr);
            this->addChild(saver);
        }
      auto bottomMenu = this->getChildByID("bottom-menu");
      if (!bottomMenu){
          log::warn("Could not find bottom-menu");
          return true;
      }
      auto sprite = CircleButtonSprite::create(
        CCLabelBMFont::create("Time", "bigFont.fnt"),
        CircleBaseColor::Green,
        CircleBaseSize::Medium
      );
      if (auto label = sprite->getChildByType<CCLabelBMFont>(0)){
          label->setScale(0.4f);
      }
      auto btn = CCMenuItemSpriteExtra::create(
          sprite,
          this,
          menu_selector(MyMenuLayer::onTimeButton)
      );
      btn->setID("time-button"_spr);
      bottomMenu->addChild(btn);
      bottomMenu->updateLayout();
      return true;
    }
    void onTimeButton(CCObject* sender){
        TimePopup::create()->show();
    }
};
