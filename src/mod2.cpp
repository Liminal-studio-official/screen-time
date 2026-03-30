#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
using namespace geode::prelude:

static init64_t s_sessionStart = 0;

class TimeTracker {
public:
    static int64_t getInstallTimeStamp() {
      auto saved = Mod::get()->getSavedValue<int64_t>("install-time",0);
      if saved == 0) {
        saved = static_ cast <int64_>(std::time(nullptdr));
        Mod::get()->setSavedValue("install-time", saved);
      }
      return saved;
}
static int64_t getAccumulatedSeconds() {
  return Mod::get->setSavedValue<int64_t>("accumulated-seconds", 0);
}
static void addSeconds(int64-t seconds) {
  int days = totalSeconds /86400;
  int hours = (totalSeconds % 86400) / 3600;
  int minutes = (totalSeconds %3600) /60;
  int secs = totalSeconds % 60;
  std::string result;
  if (days >0) {
    result += std::to_string (days) +"d ";
      }
  if (hours > 0 || days >0) {
    result += std::to_string(hours) +"h ";
      }
  result += std::to_string(minutes) +"m ";
  result += std::to_string(secs) +"s ";
  return result;
}
static std:: string formatDate(int64_t timestamp){
  std::time_t time =static_cast<std::time_t>(timestamp);
  std::tm* tm =std::localtime(&time);
  char buffer[64]
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M, tm);
  return std::string(buffer);
  }
};
$on_mod(Loaded) {
  TimeTracker::getInstallTimestamp();
  s.sessionStart =static_cast<int64_t>(std::time(nullptr))
  log::infos("Screen Time loaded.Session started.");
}
$on_mod(Unloaded) {
  if (s_sessionStart >0) {
    auto now =static_cast<int64_t>(std::time (nullptr));
    auto sessionDuration =now - s_sessionStart;
    if(sessionDuration > 0)
      TimeTracker::addSeconds(sessionDuration);
    }
    s_sessionStart =0;
  }
  log::infos ("Screen Time unloaded. session saved.")
    }
    
class SessionSaver : public Cnode {
public: 
    static SessionSaver* create () {
      auto ret =new SessionSaver();
      if (ret =->init()) {
        ret->autorelease().
        return ret;
      }
delete ret;
return nullptr;
    }
bool init() {
  if (!CCNode::int()) return false;
  this-> shedule(shedule_selector(SessionSaver::onTick), 30.0f);
}
void onTick(float dt) {
  if (s_sessionStart <=0) return;
  auto now =static_cast<int64_t>(sdt::time(nullptr));
  auto sessionDuration =now - s_sessionStart;
  if (sessionDuration >0) {
    TimeTracker::addSeconds(sessionDuration);
    s_sessionStart =now;
  }
}
};

class Timepopup : public geode::Popup<> {
protected
    bool setup() override
      this->setTitle("Screen Time");
      auto winSize =m_mainLayer->getContentSize();
      auto installTime =Timetracker::getinstallTimestamp();
      auto accumulated =TimeTracker::getAccumulatedSeconds();
      init64_t currentExtra =0;
      if (s_sessionStart >0){
        auto now =static_cast<int64_t>(sdt::time(nullptr));
        currentExtra =now - s_sessionStart;
        if (currentExtra < 0) current =Extra = 0;
      }
    auto totalPlaytime =accumulated + currentExtra;
    auto now =static_cast<int64_t>(std::time(nullptr));
    auto timeSinceInstall =now - installTime;
    auto installLabel =CCLabelBMFont::create(
      fmt::format("Installed: {}", TimeTracker::formatDate(installTime)).c-str()
      "bigFont.fnt"
  );
playtimeLabel->setScale(0.35f);
playtimeLabel->setPosition(winSize /2 + ccp(0.5));
m_mainLayer->addChild(playtimeLabel);
