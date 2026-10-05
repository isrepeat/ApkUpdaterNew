#include <Windows.h>

#include <AndroidAppPreviewer.PluginSDK/AndroidAppPreviewerPlugin.h>

#include <stdexcept>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace AndroidAppPreviewerPluginSDK;

// Проверяем реальный ABI плагина и пиксели ANGLE, включая исходящую страницу.
int main(int argc, char** argv) {
    try {
        if (argc != 4) { throw std::runtime_error("Usage: PreviewTransitions plugin.dll font.ttf resources"); }
        const auto library = LoadLibraryExA(argv[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!library) { throw std::runtime_error("Could not load plugin"); }
        const auto getApi = reinterpret_cast<const xp_plugin_api* (*)(uint32_t)>(GetProcAddress(library, "xp_get_api"));
        if (!getApi) { throw std::runtime_error("Missing xp_get_api"); }
        const auto& api = *getApi(1);
        const auto check = [&](bool value, const char* message) {
            if (!value) { throw std::runtime_error(std::string(message) + ": " + api.metadata.last_error()); }
        };
        auto* session = api.session.create(720, 1600);
        check(session != nullptr, "create session");
        auto* surface = api.rendering.create_angle_surface(720, 1600, argv[2], argv[3]);
        check(surface != nullptr, "create surface");
        std::vector<unsigned char> pixels(720 * 1600 * 4);
        const auto render = [&] {
            check(api.session.render_angle_surface(session, surface, pixels.data(), 720 * 4,
                static_cast<int>(pixels.size())) != 0, "render");
        };
        const auto finish = [&] {
            check(api.session.set_animation_playback_rate(session, 10000.0f) != 0, "speed");
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            check(api.session.update(session) != 0, "update");
            check(!api.session.is_transitioning(session), "transition finished");
            check(api.session.set_animation_playback_rate(session, 1.0f) != 0, "reset speed");
        };
        const auto navigate = [&](const char* json) {
            check(api.metadata.navigate(session, json) != 0, "navigate");
        };
        render();
        check(pixels[3] == 128, "initial main page alpha");
        // Фон меняется симметрично при повторном открытии обеих страниц.
        for (int i = 0; i < 2; ++i) {
            navigate(R"({"transitionIds":["main-to-settings"]})");
            check(api.session.is_transitioning(session) != 0, "settings animation started");
            render();
            std::cout << "Initial alpha: " << static_cast<int>(pixels[3]) << '\n';
            check(pixels[3] >= 127 && pixels[3] <= 129, "initial background alpha");
            check(api.session.load_page(session, "MainPage") == 0, "navigation blocked during transition");
            finish();
            render();
            check(pixels[3] == 255, "final background alpha");
            navigate(R"({"transitionIds":["settings-to-main"]})");
            check(api.session.is_transitioning(session) != 0, "main animation started");
            render();
            check(pixels[3] == 255, "main starts opaque");
            finish();
            render();
            check(pixels[3] == 128, "main returns to translucent");
        }
        // Исходящая красная страница остаётся видима во время Hide.
        const char* mainMarkup = R"(<Page xmlns="urn:apkupdaternew:xaml" background="#FFFF0000">
            <Page.Storyboards><Storyboard trigger="Hide">
                <FloatAnimation property="opacity" from="1" to="0" duration="1000"/>
            </Storyboard></Page.Storyboards></Page>)";
        const char* settingsMarkup = R"(<Page xmlns="urn:apkupdaternew:xaml" id="settingsPage" background="#FF0000FF">
            <VisualStateManager.VisualStateGroups><VisualStateGroup name="NavigationDirection">
                <VisualState name="Idle"><Storyboard/></VisualState>
                <VisualState name="Forward"><Storyboard>
                    <FloatAnimation targetName="settingsPage" property="opacity" from="0" to="1" duration="1000"/>
                </Storyboard></VisualState>
                <VisualState name="Backward"><Storyboard>
                    <FloatAnimation targetName="settingsPage" property="opacity" from="1" to="0" duration="1000"/>
                </Storyboard></VisualState>
            </VisualStateGroup></VisualStateManager.VisualStateGroups></Page>)";
        check(api.session.reload_markup(session, "MainPage", mainMarkup, "MainPage.xaml") != 0, "reload main");
        check(api.session.reload_markup(session, "SettingsPage", settingsMarkup, "SettingsPage.xaml") != 0, "reload inactive page");
        navigate(R"({"transitionIds":["main-to-settings"]})");
        check(api.session.is_transitioning(session) != 0, "direction animation started");
        render();
        check(pixels[2] == 255 && pixels[0] == 0 && pixels[3] == 255, "outgoing page rendered");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        check(api.session.update(session) != 0, "advance transition");
        render();
        check(pixels[0] > 0 && pixels[2] > 0, "both pages rendered mid-transition");
        finish();
        render();
        check(pixels[0] == 255 && pixels[2] == 0, "incoming page rendered");
        navigate(R"({"transitionIds":["settings-to-main"]})");
        check(api.session.is_transitioning(session) != 0, "backward animation started");
        finish();
        navigate(R"({"transitionIds":["main-to-settings","settings-to-main","main-to-settings"]})");
        check(api.session.is_transitioning(session) != 0, "last graph transition animates");
        finish();
        // Hot reload во время перехода сбрасывает его, сохраняя активную страницу.
        navigate(R"({"transitionIds":["settings-to-main"]})");
        check(api.session.is_transitioning(session) != 0, "transition before reload");
        check(api.session.reload_markup(session, "MainPage", mainMarkup, "MainPage.xaml") != 0, "reload during transition");
        check(!api.session.is_transitioning(session), "reload settled transition");
        const char* showMarkup = R"(<Page xmlns="urn:apkupdaternew:xaml" background="#FF0000FF">
            <Page.Storyboards><Storyboard trigger="Show">
                <FloatAnimation property="opacity" from="0" to="1" duration="1000"/>
            </Storyboard></Page.Storyboards></Page>)";
        check(api.session.reload_markup(session, "SettingsPage", showMarkup, "SettingsPage.xaml") != 0, "reload Show");
        navigate(R"({"transitionIds":["main-to-settings"]})");
        check(api.session.is_transitioning(session) != 0, "Show started");
        render();
        check(pixels[2] == 255 && pixels[0] == 0, "Show initial frame");
        finish();
        render();
        check(pixels[0] == 255 && pixels[2] == 0, "Show final frame");
        api.rendering.destroy_angle_surface(surface);
        api.session.destroy(session);
        std::cout << "PASS: alpha, repeated navigation, Show/Hide, direction, two-page rendering, reload, graph path\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}