# TouchFreeze

TouchFreeze stops accidental touchpad taps and clicks while you type. For a
short moment after each key press, mouse button presses are ignored.

This is a fork of [TouchFreeze](https://github.com/macleod-ee/touchfreeze) by
Ivan Zhakov, updated for 64-bit Windows 10/11:

- 64-bit build with current Visual Studio
- No "Tap blocked!" notification popups
- Per-user installer: no admin prompt, starts with Windows
- "Load at system startup" tray option works again

TouchFreeze blocks taps and clicks, but not pointer movement or scrolling. If
the pointer moving while you type bothers you, PowerToys' Mouse Utilities can
hide the pointer while typing.

## Install

Run `TouchFreeze-<version>-setup.exe`. It installs to
`%LOCALAPPDATA%\Programs\TouchFreeze` without asking for administrator rights.
By default it also starts TouchFreeze when you sign in, and you can turn that
off during setup. TouchFreeze runs as an icon in the notification area. Click
the icon for About, the startup option and Exit.

You can uninstall it from **Settings → Apps → Installed apps**.

> TouchFreeze runs without admin rights, so it can't block taps in windows
> that run as administrator (Windows prevents this).

## Build

You need:

- Visual Studio 2022 or newer, or Build Tools, with **Desktop development
  with C++** (MSVC and a Windows 10/11 SDK)
- To build the installer, Inno Setup 6.3 or newer:
  `winget install --id JRSoftware.InnoSetup -e`

Then run, in PowerShell:

```powershell
.\build.ps1                # x64 Release binaries + dist\TouchFreeze-<version>-setup.exe
.\build.ps1 -NoInstaller   # binaries only, in Executable\x64\Release
```

You can also open `TouchFreeze.sln` in Visual Studio.

The version number comes from `Sources\TestApp\KeyHookTest.rc`
(`ProductVersion`).

## How it works

`TouchFreeze.dll` installs low-level keyboard and mouse hooks. It ignores
Ctrl, Alt and Shift. For 700 ms after any other key press, it discards left,
middle and right button down/up/double-click events. `TouchFreeze.exe` owns
the hooks and the tray icon. Running `TouchFreeze.exe unload` stops a running
copy.

## License

GNU General Public License v2 or later. See [License.txt](License.txt).
