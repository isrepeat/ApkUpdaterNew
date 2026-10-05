# Проверка переходов

PreviewTransitions.cpp загружает собранный preview-плагин через его публичный ABI и проверяет реальные пиксели ANGLE. Покрыты прозрачность фона настроек, повторное открытие, Show/Hide, Forward/Backward, блокировка навигации, совместный рендеринг страниц, hot reload и многошаговый маршрут графа.

Сначала соберите плагин из корня проекта:

```powershell
./build.ps1 -Command run-android-app-previewer -Configuration Debug -BuildOnly
```

В x64 Native Tools Command Prompt for VS, из корня проекта, с установленной версией SDK:

```bat
cl /nologo /EHsc /std:c++20 /utf-8 /I Build\Packages\ApkUpdaterNew.PreviewPlugin\AndroidAppPreviewer.PluginSDK.2.0.4\build\native\include Tests\PreviewTransitions.cpp /Fe:Build\PreviewTransitions.exe /Fo:Build\PreviewTransitions.obj
```

Запустите Build\PreviewTransitions.exe с тремя аргументами: абсолютным Windows-путём к ApkUpdaterNew.PreviewPlugin.dll, путём к Roboto-Regular.ttf и каталогом ресурсов приложения. Рядом со шрифтом нужны Roboto-Bold.ttf и Roboto-Black.ttf. DLL ANGLE должны лежать рядом с плагином, как после штатной сборки. Код возврата 0 и строка PASS означают успех.