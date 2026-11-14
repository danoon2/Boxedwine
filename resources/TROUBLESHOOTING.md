# Shared troubleshooting tips

The Windows, macOS and Linux UIs display the additional compatibility tips in
`project/mac-xcode/Boxedwine/BoxedwineUI/Resources/troubleshooting.json`.
Each entry has a stable `id`, a user-facing `title` and `advice`, and a
`knownGames` list displayed as “Known games affected”. Update this shared file
when adding a tip or another confirmed affected game.

Only list versions actually tested. Pharaoh's 1999 demo skips sound initialization
when Windows reports an NT platform. Reporting Windows 98 restores audio; this
was verified with native audio capture and the browser ST JIT build. The full
game has not been tested. Its demo catalogue recipe also sets Windows 98 so new
installations receive the working setting automatically.
