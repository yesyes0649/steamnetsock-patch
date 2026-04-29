# Slay the Spire 2 Multiplayer Patch

Patch to fix multiplayer when using [SLSSteam](https://github.com/AceSLS/SLSSteam/)'s [FakeAppIds](https://github.com/AceSLS/SLSsteam/wiki/Configuration#fakeappids-map-of-positive-number--positive-number) feature for Slay the Spire 2.

# To use

- Download the fix from releases, or build it from source
  - To build, run `gcc -O3 -shared -fPIC -o fix.so steamclient_audit.c -ldl`

- Copy the `fix.so` file to the game folder

- Edit steam launch options
  - if you're running the linux native build: `LD_AUDIT=./fix.so %command%`
  - or if you're running via Proton: `LD_AUDIT=./fix.so %command% --rendering-driver vulkan`

- Multiplayer will now work. Note you can only play with users that have the same fake AppID as yourself. Online-Fix players have fake AppID 480

**Note:** A future steamclient update may break the fix. So, if you're having issues, check the `audit_patch.log` file. If it says "pattern not found", please open an issue.

# Explanation

The "Cert is not authorized for appid 2868840, only 480" error is not caused by StS2, but is part of steamclient.

steamclient itself embeds gamenetworkingsockets, which has [this](<https://github.com/ValveSoftware/GameNetworkingSockets/blob/903161cac02458f5325b2d9fd70e019a185430c9/src/steamnetworkingsockets/steamnetworkingsockets_certstore.cpp#L760>).

This fix simply edits that function to immediately return 1.

# Credits

- [AceSLS](https://github.com/AceSLS/) for [SLSSteam](https://github.com/AceSLS/SLSSteam/) and also figuring out the game coldloads steamclient.so
- [niwia](https://github.com/niwia) for finding the issue, testing the fixes and sacrificing his save file... sorry!
