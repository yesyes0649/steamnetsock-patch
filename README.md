# SLSsteam SteamNetworkingSockets Patch

Patch to fix multiplayer in a small number of games when using [SLSsteam](https://github.com/AceSLS/SLSsteam/)'s [FakeAppIds](https://github.com/AceSLS/SLSsteam/wiki/Configuration#fakeappids-map-of-positive-number--positive-number) feature.

# To use

**WARNING:** Do not use this fix with games that have anti-cheat. This fix scans and modifies game memory, which any anti-cheat solution will easily detect. You have been warned.

- Download the fix from releases, or build it from source
  - To build, run `gcc -O3 -shared -fPIC -o fix.so steamclient_audit.c -ldl`

- Copy the `fix.so` file to the game folder

- Edit steam launch options: `LD_AUDIT=./fix.so %command%`

- Multiplayer will now work. Note you can only play with users that have the same fake AppID as yourself. Online-Fix players usually have fake AppID 480.

**Note:** A future steamclient update may break the fix. So, if you're having issues, check the `audit_patch.log` file. If it says "pattern not found", please open an issue.

# Compatible Games

Games that use "SteamNetworkingSockets" and work properly via GBE or OnlineFix should also be functional when using this fix.

Confirmed to work:
- Slay the Spire 2 (2868840)
- Enshrouded (1203620)
- RV There Yet (3949040)
- Teardown (1167630)
- Lethal Company (1966720)
- Tabletop Simulator (286160)
- Schedule I (3164500)
- DARK SOULS REMASTERED (570940) with [Seamless Co-op mod](https://www.nexusmods.com/darksoulsremastered/mods/899)
- ... and more (you can find more games listed in the SLS Discord)

# Explanation

The "Cert is not authorized for appid 2868840, only 480" error is not caused by StS2, but is part of steamclient.

steamclient itself embeds gamenetworkingsockets, which has [this](<https://github.com/ValveSoftware/GameNetworkingSockets/blob/903161cac02458f5325b2d9fd70e019a185430c9/src/steamnetworkingsockets/steamnetworkingsockets_certstore.cpp#L760>).

This fix simply edits that function to immediately return 1.

# Credits

- [AceSLS](https://github.com/AceSLS/) for [SLSsteam](https://github.com/AceSLS/SLSsteam/) and also figuring out the game coldloads steamclient.so
- [niwia](https://github.com/niwia) for finding the issue, testing the fixes and sacrificing his save file... sorry!
- [Aguga](https://github.com/Aguga2201) for testing a ton of games
