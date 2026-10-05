#include <Windows.h>
#undef interface

#include <XamlRuntime/Animation.h>

#include "../ApkUpdaterNew.PreviewPlugin/Rendering/AngleRenderSurface.h"
#include "../ApkUpdaterNew.PreviewPlugin/Session/preview_PackageInstaller.h"
#include "../ApkUpdaterNew.PreviewPlugin/Session/PreviewSession.h"

#include <stdexcept>
#include <iostream>
#include <iterator>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace apkupdaternew;

void Check(bool value, const char* message) {
    if (!value) {
        throw std::runtime_error(message);
    }
}

xaml::Element* Find(xaml::Element& root, std::string_view id) {
    if (root.Id() == id) {
        return &root;
    }
    for (const auto& child : root.Children()) {
        if (auto* found = Find(*child, id)) {
            return found;
        }
    }
    return nullptr;
}

void TestBackend() {
    using application::core::UpdatePhase;
    for (const auto* scenario : {"success", "reinstall", "cancel", "invalid-apk", "install-error", "launch-error"}) {
        application::core::UpdateController controller;
        preview::session::preview_PackageInstaller packageInstaller(controller);
        packageInstaller.Configure(scenario);
        controller.Start();
        controller.Start();
        packageInstaller.Advance(std::chrono::milliseconds{699});
        Check(controller.State().phase == UpdatePhase::preparing, "Preparation delay");
        packageInstaller.Advance(std::chrono::milliseconds{1});
        const std::string name(scenario);
        if (name == "invalid-apk") {
            Check(controller.State().phase == UpdatePhase::failed && !controller.State().opaque, "Invalid APK stays translucent");
            controller.Cancel();
            Check(controller.State().phase == UpdatePhase::closed, "Close validation error");
            continue;
        }
        if (name == "cancel" || name == "reinstall") {
            packageInstaller.Advance(std::chrono::milliseconds{60000});
            Check(controller.State().phase == UpdatePhase::confirmation, "Confirmation waits for user");
            if (name == "cancel") {
                controller.Cancel();
                packageInstaller.Advance(std::chrono::milliseconds{60000});
                Check(controller.State().phase == UpdatePhase::closed && !controller.State().installed, "Cancel stops work");
                continue;
            }
            controller.Accept();
        }
        packageInstaller.Advance(std::chrono::milliseconds{60000});
        Check(controller.State().phase == UpdatePhase::covering, "Must wait for opaque frame");
        controller.OpaqueFramePresented();
        packageInstaller.Advance(std::chrono::milliseconds{60000});
        Check(controller.State().phase == UpdatePhase::systemConfirmation, "System confirmation waits");
        controller.Accept();
        packageInstaller.Advance(std::chrono::milliseconds{2500});
        if (name == "install-error") {
            Check(controller.State().phase == UpdatePhase::failed && controller.State().opaque, "Installation error stays opaque");
            continue;
        }
        controller.Cancel();
        Check(controller.State().phase == UpdatePhase::installing, "Committed install cannot be cancelled");
        packageInstaller.Advance(std::chrono::milliseconds{2000});
        Check(controller.State().progress == 100 && !controller.State().installed, "100 percent is not success");
        packageInstaller.Advance(std::chrono::milliseconds{700});
        Check(controller.State().phase == UpdatePhase::launching, "Launch only after success");
        packageInstaller.Advance(std::chrono::milliseconds{1000});
        Check(controller.State().phase == (name == "launch-error" ? UpdatePhase::failed : UpdatePhase::closed), "Final result");
        Check(controller.State().installed, "Installed result retained");
    }
    application::core::UpdateController controller;
    preview::session::preview_PackageInstaller packageInstaller(controller);
    controller.Start();
    packageInstaller.Advance(std::chrono::milliseconds{700});
    controller.OpaqueFramePresented();
    controller.Cancel();
    Check(controller.State().phase == UpdatePhase::closed, "System cancellation");
    controller.Start();
    packageInstaller.Advance(std::chrono::milliseconds{700});
    packageInstaller.Configure("success");
    packageInstaller.Advance(std::chrono::milliseconds{10000});
    Check(controller.State().phase == UpdatePhase::idle, "Reset drops pending work");
}

int main(int argc, char** argv) {
    try {
        Check(argc == 5, "Usage: UpdateSimulation font resources markup output-prefix");
        TestBackend();
        preview::session::PreviewSession session(720, 1600);
        preview::rendering::AngleRenderSurface surface(720, 1600, argv[1], argv[2]);
        std::vector<unsigned char> pixels(720 * 1600 * 4);
        const auto render = [&] {
            surface.Render([&](xaml::IRenderBackend& backend) {
                session.Render(backend);
            }, pixels.data(), 720 * 4);
        };
        const auto snapshot = [&](const char* name) {
            render();
            BITMAPFILEHEADER file{};
            file.bfType = 0x4D42;
            file.bfOffBits = sizeof(file) + sizeof(BITMAPINFOHEADER);
            file.bfSize = file.bfOffBits + static_cast<DWORD>(pixels.size());
            BITMAPINFOHEADER info{};
            info.biSize = sizeof(info);
            info.biWidth = 720;
            info.biHeight = -1600;
            info.biPlanes = 1;
            info.biBitCount = 32;
            std::ofstream output(std::string(argv[4]) + name + ".bmp", std::ios::binary);
            output.write(reinterpret_cast<const char*>(&file), sizeof(file));
            output.write(reinterpret_cast<const char*>(&info), sizeof(info));
            output.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        };
        const auto click = [&](const char* id) {
            auto* element = Find(session.Root(), id);
            Check(element != nullptr, "Button exists");
            const auto bounds = element->Bounds();
            session.PointerDown(bounds.x + bounds.width / 2, bounds.y + bounds.height / 2);
            session.PointerUp(bounds.x + bounds.width / 2, bounds.y + bounds.height / 2);
            session.Update();
        };
        const auto advance = [&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            session.Update();
        };
        session.Update();
        snapshot("idle");
        session.SetAnimationPlaybackRate(20);
        click("startUpdateButton");
        advance();
        advance();
        render();
        session.Update();
        Check(Find(session.Root(), "updateHeading")->Text() == "Установить обновление?", "Opaque frame opens system mock");
        Check(session.Root().Background().alpha == 1, "Opaque before installation");
        snapshot("confirmation");
        click("acceptUpdateButton");
        advance();
        snapshot("installing");
        Check(pixels[0] == 30 && pixels[1] == 30 && pixels[2] == 30, "Opaque pixels hide stopped application");
        Check(Find(session.Root(), "progressSegment10")->Bounds().height > 0, "Progress segment has height");
        const auto segment = Find(session.Root(), "progressSegment10")->Bounds();
        const auto pixel = (static_cast<int>(segment.y + 6) * 720 + static_cast<int>(segment.x + 2)) * 4;
        Check(pixels[pixel] > 200 && pixels[pixel + 2] < 150, "Progress segment is blue");
        // Hot reload сохраняет процесс, а новые привязки получают актуальное состояние.
        std::ifstream source(argv[3]);
        const std::string markup{std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>()};
        std::string error;
        Check(session.ReloadMarkup("MainPage", markup, argv[3], error), error.c_str());
        session.Update();
        render();
        Check(session.Root().Background().alpha == 1, "Reload retains opaque background");
        for (int i = 0; i < 5; ++i) {
            advance();
            render();
        }
        snapshot("closed");
        Check(session.Root().Background().alpha == 0, "Updater disappears after success");
        session.ApplyScenario(R"({"scenario":"invalid-apk"})");
        session.Update();
        click("startUpdateButton");
        advance();
        snapshot("error");
        Check(Find(session.Root(), "updateHeading")->Text() == "Не удалось завершить обновление", "Scenario error visible");
        bool rejected = false;
        try {
            session.ApplyScenario(R"({"scenario":"unknown"})");
        } catch (...) {
            rejected = true;
        }
        Check(rejected, "Unknown scenario rejected");
        std::cout << "PASS: six scenarios, cancellation, frame barrier, progress, real clicks, ANGLE, hot reload\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}