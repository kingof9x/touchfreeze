# TouchFreeze

TouchFreeze stops accidental touchpad taps and clicks while you're typing.
It runs quietly in the notification area of Windows 10 and 11.

## Download

**[Download the latest installer](https://github.com/kingof9x/touchfreeze/releases/latest)**:
under **Assets**, pick `TouchFreeze-x.y.z-setup.exe`.

- The installer doesn't need administrator rights.
- By default, TouchFreeze starts when you sign in. You can turn this off
  during setup or from the tray icon.
- To uninstall, go to **Settings → Apps → Installed apps**.

The installer isn't code-signed, so Windows SmartScreen may warn you. Click
**More info → Run anyway**.

## Notes

- TouchFreeze blocks taps and clicks, not pointer movement or scrolling. To
  hide the pointer while you type, try PowerToys' Mouse Utilities.
- It can't block clicks in apps that run as administrator.

## Credits and license

This is a fork of [TouchFreeze](https://github.com/macleod-ee/touchfreeze)
by Ivan Zhakov, updated for 64-bit Windows: no notification popups, a
per-user installer and a working startup option. GitHub Actions builds it
automatically (see `.github/workflows/build.yml`). It's licensed under the
GNU GPL v2. See [License.txt](License.txt).
