# Проверка переходов

PreviewTransitions.cpp загружает собранный preview-плагин через его публичный ABI и проверяет реальные пиксели ANGLE. Покрыты Show/Hide, Forward/Backward, совместный рендеринг страниц, hot reload и многошаговый маршрут графа на тестовой разметке.

Сначала соберите плагин из корня проекта:

```powershell
./build.ps1 -Command run-android-app-previewer -Configuration Debug -BuildOnly
```

В x64 Native Tools Command Prompt for VS, из корня проекта, с установленной версией SDK:

```bat
cl /nologo /EHsc /std:c++20 /utf-8 /I Build\Packages\ApkUpdaterNew.PreviewPlugin\AndroidAppPreviewer.PluginSDK.2.0.4\build\native\include Tests\PreviewTransitions.cpp /Fe:Build\PreviewTransitions.exe /Fo:Build\PreviewTransitions.obj
```

Запустите Build\PreviewTransitions.exe с тремя аргументами: абсолютным Windows-путём к ApkUpdaterNew.PreviewPlugin.dll, путём к Roboto-Regular.ttf и каталогом ресурсов приложения. Рядом со шрифтом нужны Roboto-Bold.ttf и Roboto-Black.ttf. DLL ANGLE должны лежать рядом с плагином, как после штатной сборки. Код возврата 0 и строка PASS означают успех.

## Симуляция обновления

UpdateSimulation.cpp проверяет все шесть сценариев с управляемым временем, отмену на двух подтверждениях, сброс отложенной работы, ожидание непрозрачного кадра и отличие 100% от успеха. Интеграционная часть нажимает настоящие XAML-кнопки, рендерит ANGLE, проверяет сегменты прогресса и hot reload во время установки. Снимки сохраняются в BMP.

После Debug-сборки плагина выполните в x64 Native Tools Command Prompt из каталога Build:

```bat
cl /nologo /EHsc /std:c++20 /utf-8 /MDd /DANDROID_APP_PREVIEWER /I Packages\ApkUpdaterNew.AndroidHost\XamlRuntime.1.0.25.10\build\native\include ..\Tests\UpdateSimulation.cpp ..\ApkUpdaterNew.PreviewPlugin\Session\PreviewSession.cpp ..\ApkUpdaterNew.PreviewPlugin\Session\PreviewNavigationController.cpp ..\ApkUpdaterNew.PreviewPlugin\Session\preview_PackageInstaller.cpp ..\ApkUpdaterNew.PreviewPlugin\Rendering\AngleRenderSurface.cpp /Fe:UpdateSimulation.exe /link ApkUpdaterNew.PreviewPlugin\Intermediate\CMake\ApkUpdaterNew.Application\apkupdaternew_application.lib /LIBPATH:Packages\ApkUpdaterNew.AndroidHost\XamlRuntime.1.0.25.10\runtimes\win-x64\native\Debug XamlRuntime.lib OpenGLESRenderer.lib Helpers.Logging.lib libEGL.lib libGLESv2.lib
```

Добавьте каталог DLL ANGLE для Debug в PATH и запустите UpdateSimulation.exe с четырьмя аргументами: путь к Roboto-Regular.ttf, каталог ресурсов приложения, исходный MainPage.xaml, префикс пути для снимков. Все артефакты теста сохраняйте в игнорируемом Build.