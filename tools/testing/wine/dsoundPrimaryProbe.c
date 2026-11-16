/* DirectSound primary-buffer ownership regression. Run on Windows and Wine.
 * i686-w64-mingw32-gcc -O2 -Wall dsoundPrimaryProbe.c -o dsoundPrimaryProbe.exe \
 *     -ldsound -ldxguid -lole32
 * Optional argument: output file. No sound is played or volume changed.
 */
#define COBJMACROS
#include <windows.h>
#include <dsound.h>
#include <stdio.h>

static FILE *output;
static unsigned checks, failures;
#define CHECK(condition, ...) do { ++checks; if (!(condition)) { \
    ++failures; fprintf(output, "FAIL line %u: ", __LINE__); \
    fprintf(output, __VA_ARGS__); fputc('\n', output); } } while (0)

static IDirectSound *create_sound(HWND window, BOOL version8)
{
    IDirectSound *sound = NULL;
    HRESULT hr = version8 ? DirectSoundCreate8(NULL, (IDirectSound8 **)&sound, NULL)
                         : DirectSoundCreate(NULL, &sound, NULL);
    CHECK(hr == DS_OK, "DirectSoundCreate%s: %#lx", version8 ? "8" : "", hr);
    if (!sound) return NULL;
    hr = IDirectSound_SetCooperativeLevel(sound, window, DSSCL_EXCLUSIVE);
    CHECK(hr == DS_OK, "SetCooperativeLevel: %#lx", hr);
    return sound;
}

static IDirectSoundBuffer *create_primary(IDirectSound *sound, DWORD flags)
{
    DSBUFFERDESC desc = {0};
    IDirectSoundBuffer *buffer = NULL;
    HRESULT hr;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER | flags;
    hr = IDirectSound_CreateSoundBuffer(sound, &desc, &buffer, NULL);
    CHECK(hr == DS_OK && buffer, "CreateSoundBuffer: %#lx", hr);
    return buffer;
}

static void check_controls(IDirectSoundBuffer *buffer, BOOL volume)
{
    DSBCAPS caps = {0};
    LONG value;
    HRESULT hr;
    caps.dwSize = sizeof(caps);
    hr = IDirectSoundBuffer_GetCaps(buffer, &caps);
    CHECK(hr == DS_OK, "GetCaps: %#lx", hr);
    CHECK(!!(caps.dwFlags & DSBCAPS_CTRLVOLUME) == volume,
            "volume capability: flags %#lx, expected %u", caps.dwFlags, volume);
    hr = IDirectSoundBuffer_GetVolume(buffer, &value);
    CHECK(hr == (volume ? DS_OK : DSERR_CONTROLUNAVAIL),
            "GetVolume: %#lx, volume control %u", hr, volume);
}

static void test_primary(HWND window, BOOL version8, BOOL same, BOOL reverse)
{
    IDirectSound *a = NULL, *b = NULL;
    IDirectSoundBuffer *first = NULL, *second = NULL, *again = NULL, *duplicate = NULL;
    HRESULT hr;
    ULONG ref;
    fprintf(output, "CASE ds8=%u same=%u reverse=%u\n", version8, same, reverse);
    a = create_sound(window, version8);
    if (!a) goto done;
    if (same) { b = a; IDirectSound_AddRef(b); }
    else b = create_sound(window, version8);
    if (!b) goto done;
    first = create_primary(a, reverse ? DSBCAPS_CTRLVOLUME : 0);
    if (!first) goto done;
    check_controls(first, reverse);
    second = create_primary(b, reverse ? 0 : DSBCAPS_CTRLVOLUME);
    if (!second) goto done;
    CHECK((first == second) == same, "primary identity: equal %u, same object %u", first == second, same);
    check_controls(first, reverse);
    check_controls(second, same ? reverse : !reverse);
    hr = IDirectSound_DuplicateSoundBuffer(a, second, &duplicate);
    CHECK(hr == DSERR_INVALIDCALL && !duplicate, "DuplicateSoundBuffer(primary): %#lx", hr);
    if (duplicate) { IDirectSoundBuffer_Release(duplicate); duplicate = NULL; }

    /* Once all interfaces have been released, creation accepts new controls. */
    IDirectSoundBuffer_Release(first);
    first = NULL;
    ref = IDirectSoundBuffer_Release(second);
    CHECK(ref == 0, "last primary interface release: %lu", ref);
    again = create_primary(b, same ? (reverse ? 0 : DSBCAPS_CTRLVOLUME)
                                 : (reverse ? DSBCAPS_CTRLVOLUME : 0));
    second = NULL;
    if (!again) goto done;
    check_controls(again, same ? !reverse : reverse);

    /* Destroy one owner while the other still uses its primary. */
    IDirectSound_Release(a);
    a = NULL;
    check_controls(again, same ? !reverse : reverse);
done:
    if (again) IDirectSoundBuffer_Release(again);
    if (second) IDirectSoundBuffer_Release(second);
    if (first) IDirectSoundBuffer_Release(first);
    if (b) IDirectSound_Release(b);
    if (a) IDirectSound_Release(a);
}

static void test_secondary_without_primary(HWND window)
{
    IDirectSound *sound = create_sound(window, FALSE);
    IDirectSoundBuffer *buffer = NULL, *duplicate = NULL;
    WAVEFORMATEX format = {WAVE_FORMAT_PCM, 1, 22050, 44100, 2, 16, 0};
    DSBUFFERDESC desc = {0};
    HRESULT hr;
    if (!sound) return;
    desc.dwSize = sizeof(desc);
    desc.dwBufferBytes = 1024;
    desc.lpwfxFormat = &format;
    hr = IDirectSound_CreateSoundBuffer(sound, &desc, &buffer, NULL);
    CHECK(hr == DS_OK, "CreateSoundBuffer(secondary): %#lx", hr);
    if (buffer) {
        hr = IDirectSound_DuplicateSoundBuffer(sound, buffer, &duplicate);
        CHECK(hr == DS_OK && duplicate, "DuplicateSoundBuffer(secondary): %#lx", hr);
        if (duplicate) IDirectSoundBuffer_Release(duplicate);
        IDirectSoundBuffer_Release(buffer);
    }
    IDirectSound_Release(sound);
}

static void test_listener(HWND window)
{
    IDirectSound *a = create_sound(window, FALSE), *b = create_sound(window, FALSE);
    IDirectSoundBuffer *first = NULL, *second = NULL, *again = NULL;
    IDirectSound3DListener *listener = NULL;
    D3DVECTOR position;
    HRESULT hr;
    fprintf(output, "CASE retained 3D listener\n");
    if (!a || !b) goto done;
    first = create_primary(a, DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME);
    if (!first) goto done;
    hr = IDirectSoundBuffer_QueryInterface(first, &IID_IDirectSound3DListener, (void **)&listener);
    CHECK(hr == DS_OK && listener, "QueryInterface(listener): %#lx", hr);
    if (!listener) goto done;
    hr = IDirectSound3DListener_SetPosition(listener, 1, 2, 3, DS3D_IMMEDIATE);
    CHECK(hr == DS_OK, "SetPosition: %#lx", hr);
    second = create_primary(b, 0);
    hr = IDirectSound3DListener_GetPosition(listener, &position);
    CHECK(hr == DS_OK && position.x == 1 && position.y == 2 && position.z == 3,
            "another primary reset the listener: %#lx (%g,%g,%g)",
            hr, position.x, position.y, position.z);
    IDirectSoundBuffer_Release(first);
    /* The listener still owns an interface on this primary. */
    again = create_primary(a, 0);
    CHECK(again == first, "primary changed while listener held");
    first = NULL;
    if (again) check_controls(again, TRUE);
done:
    if (again) IDirectSoundBuffer_Release(again);
    if (listener) IDirectSound3DListener_Release(listener);
    if (second) IDirectSoundBuffer_Release(second);
    if (first) IDirectSoundBuffer_Release(first);
    if (b) IDirectSound_Release(b);
    if (a) IDirectSound_Release(a);
}

int main(int argc, char **argv)
{
    HWND window;
    unsigned ds8, same, reverse;
    output = argc > 1 ? fopen(argv[1], "w") : stdout;
    if (!output) return 2;
    window = CreateWindowExA(0, "STATIC", "DirectSound primary buffer probe", WS_POPUP,
            0, 0, 32, 32, NULL, NULL, GetModuleHandleA(NULL), NULL);
    CHECK(window != NULL, "CreateWindow: %lu", GetLastError());
    if (window) {
        for (ds8 = 0; ds8 < 2; ++ds8)
            for (same = 0; same < 2; ++same)
                for (reverse = 0; reverse < 2; ++reverse)
                    test_primary(window, ds8, same, reverse);
        test_secondary_without_primary(window);
        test_listener(window);
        DestroyWindow(window);
    }
    fprintf(output, "%s: %u checks, %u failures\n", failures ? "FAIL" : "PASS", checks, failures);
    if (output != stdout) fclose(output);
    return failures ? 1 : 0;
}
