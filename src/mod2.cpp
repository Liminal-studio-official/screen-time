#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <fmod.hpp>

using namespace geode::prelude;

static constexpr const char* LOFI_STREAM_URL = "https://usa9.fastcast4u.com/proxy/jamz?mp=/1";

static bool g_lastSettingState = false;
static FMOD::System* g_fmodSystem = nullptr;
static FMOD::Sound* g_streamSound = nullptr;
static FMOD::Channel* g_streamChannel = nullptr;
static bool g_isPlaying = false;

class LofiStreamManager {
public:
    static bool initSystem() {
        if (g_fmodSystem) return true;

        FMOD_RESULT result = FMOD::System_Create(&g_fmodSystem);
        if (result != FMOD_OK || !g_fmodSystem) {
            log::error("Failed to create FMOD system");
            return false;
        }

        result = g_fmodSystem->init(32, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            log::error("Failed to init FMOD system");
            g_fmodSystem->release();
            g_fmodSystem = nullptr;
            return false;
        }

        return true;
    }

    static void start() {
        if (g_isPlaying) return;
        if (!initSystem()) return;

        if (g_streamSound) {
            g_streamSound->release();
            g_streamSound = nullptr;
        }

        FMOD_RESULT result = g_fmodSystem->createStream(
            LOFI_STREAM_URL,
            FMOD_CREATESTREAM | FMOD_NONBLOCKING,
            nullptr,
            &g_streamSound
        );

        if (result != FMOD_OK || !g_streamSound) {
            log::error("Failed to create stream from URL");
            Notification::create(
                "Failed to start lofi stream.",
                NotificationIcon::Error,
                3.0f
            )->show();
            return;
        }

        result = g_fmodSystem->playSound(g_streamSound, nullptr, false, &g_streamChannel);
        if (result != FMOD_OK) {
            log::error("Failed to play stream");
            Notification::create(
                "Failed to play lofi stream.",
                NotificationIcon::Error,
                3.0f
            )->show();
            return;
        }

        g_isPlaying = true;
    }

    static void stop() {
        if (g_streamChannel) {
            g_streamChannel->stop();
            g_streamChannel = nullptr;
        }

        if (g_streamSound) {
            g_streamSound->release();
            g_streamSound = nullptr;
        }

        g_isPlaying = false;
    }

    static void update() {
        if (g_fmodSystem) {
            g_fmodSystem->update();
        }

        if (g_isPlaying && g_streamChannel) {
            bool isPlaying = false;
            FMOD_RESULT result = g_streamChannel->isPlaying(&isPlaying);
            if (result != FMOD_OK || !isPlaying) {
                g_isPlaying = false;
                g_streamChannel = nullptr;
            }
        }
    }

    static void refresh() {
        bool enabled = Mod::get()->getSettingValue<bool>("active-mods");

        if (enabled && !g_isPlaying) {
            start();
            if (g_isPlaying) {
                Notification::create(
                    "Lofi stream enabled. Chill radio is now playing.",
                    NotificationIcon::Info,
                    4.0f
                )->show();
            }
        }
        else if (!enabled && g_isPlaying) {
            stop();
            Notification::create(
                "Lofi stream disabled.",
                NotificationIcon::Info,
                3.0f
            )->show();
        }

        g_lastSettingState = enabled;
    }

    static void cleanup() {
        stop();
        if (g_fmodSystem) {
            g_fmodSystem->close();
            g_fmodSystem->release();
            g_fmodSystem = nullptr;
        }
    }
};

struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {
    bool init() {
        if (!MenuLayer::init()) return false;

        g_lastSettingState = Mod::get()->getSettingValue<bool>("active-mods");
        this->schedule(schedule_selector(MyMenuLayer::checkSetting), 0.25f);
        this->schedule(schedule_selector(MyMenuLayer::updateFmod), 0.05f);

        LofiStreamManager::refresh();
        return true;
    }

    void checkSetting(float dt) {
        bool enabled = Mod::get()->getSettingValue<bool>("active-mods");
        if (enabled != g_lastSettingState) {
            LofiStreamManager::refresh();
        }
    }

    void updateFmod(float dt) {
        LofiStreamManager::update();
    }
};

struct MyPlayLayer : Modify<MyPlayLayer, PlayLayer> {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (Mod::get()->getSettingValue<bool>("active-mods")) {
            LofiStreamManager::start();
        }

        this->schedule(schedule_selector(MyPlayLayer::updateFmod), 0.05f);
        return true;
    }

    void updateFmod(float dt) {
        LofiStreamManager::update();
    }
};
