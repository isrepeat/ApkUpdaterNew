# ApkUpdaterNew

```powershell
./build.ps1 build-android -Configuration Debug
./build.ps1 build-and-distribute -Destination Local -Configuration Debug
```

Проект создан пакетом `AndroidBuildTools` версии `1.0.63.19`. `android-build.psd1` содержит пути к пакетам
и инструментам, `ApkUpdaterNew.Android/build.gradle.kts` — package ID и Android-ресурсы,
`ApkUpdaterNew.AndroidHost` — native-код. Gradle wrapper включён в репозиторий.
`ApkUpdaterNew.Application/UI/Page/MainPage.xaml` — минимальная нативная страница;
её `.cpp/.h` генерирует `XamlCompiler` в `!Generated` перед сборкой CMake.

При генерации проекта в `android-build.psd1` уже записан путь `SigningProperties`
к общему файлу `SecretsRoot/shared/signing.properties` вне Git. Все приложения и
ApkUpdater используют его Debug и Release ключи. Генератор создаёт их один раз.
Для CI этот путь можно
переопределить `ANDROID_SIGNING_PROPERTIES`.

Конфигурация сразу содержит секции `Xaml`, `Preview` и `Drive`, как у полноценного
приложения. `Preview.Executable.Debug` и `Preview.Executable.Release` содержат
пути к готовому AndroidAppPreviewer, а Drive использует пути к секретам,
переданные `New-AndroidApplication.ps1`.


## XAML-preview

Соберите только `PreviewPlugin.dll`, не запуская AndroidAppPreviewer:

```powershell
$projectRoot = 'C:\WORK\Android\Projects\ApkUpdaterNew'
Set-Location $projectRoot
.\build.ps1 run-android-app-previewer -BuildOnly -Configuration Debug
```

Для сборки плагина и запуска previewer выполните:

```powershell
$projectRoot = 'C:\WORK\Android\Projects\ApkUpdaterNew'
Set-Location $projectRoot
.\build.ps1 run-android-app-previewer -Configuration Debug
```

Пути к `AndroidAppPreviewer.exe` для Debug и Release находятся в параметрах
`Paths.PreviewerDebugExecutablePath` и `Paths.PreviewerReleaseExecutablePath`
ближайшего родительского `Android.SharedProps.json`.

Скрипты `Scripts/run-android-app-previewer-debug.bat` и Release используют
инкрементальную сборку: закреплённый AndroidBuildTools повторно не скачивается,
актуальность XamlRuntime из локального feed проверяется по версиям архивов,
неизменённые XAML не генерируются повторно. CMake получает готовые пути из
PowerShell, проверяет конфигурацию и PluginSDK; Ninja собирает только изменения.
Для удалённого feed или отсутствующего пакета остаётся обычный NuGet restore.

## Android-страница

`MainPage.kt` содержит единственную Activity приложения и наследуется от
`com.isrepeat.androidappkit.NativeOpenGlActivity` (AndroidAppKit 1.0.15 или новее).
Библиотека создаёт GLSurfaceView, управляет EGL, рендером, touch, Back и временем
жизни native-сессии. Отдельный MainActivity не нужен; манифест запускает MainPage.

Обработчики `onPageCreated(savedInstanceState)`, `onPageResumed()`,
`onPagePaused()`, `onPageDestroyed()` и `onPageIntent(intent)` выполняются на UI-потоке.
`onPageIntent` получает как начальный Intent, так и последующие onNewIntent.
`onPageTouchEvent(event)` и `onBackRequested()` могут вернуть true, чтобы обработать
событие в Kotlin; по умолчанию библиотека передаёт его в C++ на GL-потоке.
Для собственных JNI-команд используйте `withNativeSession { session -> ... }`:
она ставит вызов в GL-очередь и не принимает новые команды после уничтожения страницы.
JNI-методы остаются в MainPage, поэтому имена экспортов C++ сохраняются.

Для прозрачного окна переопределите `translucentSurface = true` и используйте
прозрачную тему Activity. Библиотека запросит RGBA8888 и прозрачный Surface;
альфу содержимого задаёт XAML. По умолчанию поверхность непрозрачна.

## Логирование Android

Кнопка `Send logs` после `Update` отправляет журнал текущего сеанса в Google Drive,
в папку `Android/ApkUpdaterNew` рядом с APK. Результат отображается на главной
странице; при необходимости Google запрашивает доступ к Drive.

При запуске `MainPage` создаётся журнал сеанса в
`Downloads/com.isrepeat/ApkUpdaterNew`. Kotlin передаёт открытый file descriptor
в native host через `NativeSessionLog` из AndroidAppKit. Android host настраивает
`Helpers.Logging` на этот файл, поэтому записи `LOG_INFO`, `LOG_WARNING` и
`LOG_ERROR` из общей C++ библиотеки попадают в журнал. PreviewPlugin настраивает
тот же logger через `xp_configure_logging`, поэтому эти записи также видны в
журнале previewer. Имя Android-журнала содержит дату и время запуска; перед
уничтожением native-сессии logger принудительно сбрасывается на диск.

## Update из Google Drive

Стартовый `MainPage` содержит кнопку **Update**. Она вызывает
`androidappkit.update.GoogleDriveUpdateController`, который ищет versioned APK в
`Android/ApkUpdaterNew` на Google Drive. Кнопка использует Android OAuth client
этого package ID и сертификата подписи.

Команда проходит по цепочке: `RequestApplicationUpdateCommand` →
`AppSessionController` → `AndroidCommandDispatcher` (JNI) →
`NativeCommandDispatcher` (UI-поток) → `GoogleDriveUpdateController.start()`.
`ApplicationSession` владеет контроллером; `PageContext.hostCommands` предоставляет
страницам интерфейс отправки команд. Числовые IDs `HostCommand` в C++ и Kotlin
должны совпадать. В JNI строки передаются как UTF-8 byte arrays.

Статус проверки и загрузки возвращается через `MainPage.setStatus()` и очередь
GL-потока в `AppSessionController`, затем в XAML binding `Status`. Операции с C++ UI
выполняются на GL-потоке. При уничтожении Activity отключается Kotlin handler,
после обработки очереди удаляется native-сессия и освобождается JNI global reference.
В preview Update показывает сообщение о доступности обновления в Android host.

Контроллер из AndroidAppKit обрабатывает авторизацию, повторный запуск, загрузку,
проверку package ID и версии. Для установленной версии host запрашивает подтверждение
переустановки. Передача APK в ApkUpdater ещё не означает успешную установку.

Для установки найденного APK нужен установленный `ApkUpdater`. Он должен быть
подписан тем же Debug или Release сертификатом, что и приложение, а его опубликованный
список разрешённых пакетов должен содержать `com.isrepeat.apkupdaternew`. Секреты Desktop OAuth
для загрузки APK остаются вне Git и добавляются в секцию `Drive` проекта, когда
нужна команда `build-and-distribute -Destination Drive`.

## Удаление тестового проекта

Удаление выполняет скрипт, скопированный в проект генератором:

```powershell
& ".\Scripts\PowerShell\Remove-AndroidProject.ps1" -ProjectRoot (Get-Location)
```

Скрипт запросит подтверждение `Y/N`, после чего удалит корень
проекта и связанный каталог secrets.
## Прозрачное окно OpenGLES

MainPage использует прозрачную тему, GLSurfaceView — RGBA8888 и
PixelFormat.TRANSLUCENT. Surface располагается поверх содержимого собственного
окна; это не SYSTEM_ALERT_WINDOW и отдельное разрешение overlay не требуется.
На создании поверхности проверяется наличие восьми бит альфы framebuffer.

Внешний вид задаётся в C++/XamlRuntime: фон MainPage.xaml #801E1E1E примерно
наполовину непрозрачен, текст и кнопки сохраняют собственную альфу.
SettingsPage остаётся непрозрачной для проверки навигации в той же Activity.
Замените альфу фона на 00 для прозрачности или FF для непрозрачного экрана.
Прозрачность не означает пропуск нажатий к приложению под окном.

OpenGLESRenderer использует отдельные коэффициенты смешивания RGB и альфы:
RGB = Cs * As + Cd * (1 - As), A = As + Ad * (1 - As).
Это сохраняет premultiplied RGB и корректную альфу для композиции Android.
Исправление включено в локальный пакет XamlRuntime начиная с 1.0.25.8.
Настройка Android-поверхности находится в NativeOpenGlActivity из AndroidAppKit.

Проверка на устройстве: открыть приложение поверх рабочего стола, убедиться,
что фон просвечивает; перейти в Settings (фон непрозрачен) и вернуться.
Повторить после сворачивания и поворота экрана. В журнале должна быть строка
OpenGLES framebuffer alpha bits: 8. Проверка на реальном устройстве ещё необходима:
desktop-тест EGL проверяет композицию пикселей, но не Android WindowManager.