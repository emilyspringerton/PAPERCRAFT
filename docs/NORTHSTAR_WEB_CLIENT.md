# NORTHSTAR — Web Client (native TypeScript + WebGL, no Emscripten)

## Where this came from

Founder real-time, 2026-08-30, immediately after MISHRI's own full TypeScript upgrade
(`MISHRI/CHANGELOG.md`) and PARENA's own new TypeScript emitter (`PARENA/src/emit_ts.c`,
`STDLIB.md`'s own "real TypeScript-target proving ground" section) shipped:

> "how do we translate sdl2 to ts opengl? can we publish papercraft as a native js client no
> weird mscrimpten?" / "if we route more of papercraft through parena plugins we can start to eat
> at the interface boundaries" / "yes start the northstar scoping pass"

**Real, direct answer given before this doc was written** (not re-litigated here, just carried
forward as the real starting premise): there is no mechanical "translation" from SDL2 C code to
TypeScript+WebGL — they are different APIs in different languages. A real web client means a
genuine **rewrite** of the rendering/input layer, not a port of `apps/client/src/main.c`.
Emscripten (compile the existing C/SDL2/OpenGL client to WASM via its own real SDL2→Canvas/WebGL
shim) is the standard, well-trodden way to get existing C game code into a browser — and is
exactly what the founder asked to avoid ("no weird emscripten"). This doc scopes the other real
path: a genuine, from-scratch, native TypeScript client.

**Scoping only, this pass.** No client or bridge code exists yet. This document exists so the
real architecture is decided *before* any of it gets written, same real discipline every other
NORTHSTAR doc in this monorepo already follows.

## The pitch

A real, playable PAPERCRAFT client that runs in an ordinary browser tab — no install, no
download, no WASM blob, `npm start`-able the same way MISHRI already is. Real TypeScript source a
person can actually read and edit, not opaque Emscripten output. Connects to the exact same real,
already-running `papercraft_server` — not a second, parallel game server implementation (that
would mean hand-duplicating every real gameplay decision this session's own PARENA mods already
centralize: `xp_award`, `item_drop`, `inventory`, `pickup`, `phone_message`).

## Real, unavoidable blocker: browsers cannot open raw UDP sockets

Not a design preference — a real, permanent Web Platform restriction with no workaround, ever.
`papercraft_protocol.h`'s own real wire format (`PcHeader` + `sendto`/`recvfrom` over a plain UDP
socket, `apps/server/src/main.c`) cannot be spoken directly from any browser JavaScript/TypeScript
context. A real bridge is mandatory. Two real candidates, evaluated honestly:

- **WebSocket (TCP-based)** — the simpler, more commonly-reached-for option. Real, serious cost
  for this specific game: TCP's own in-order, reliable delivery means one real dropped/delayed
  packet blocks every packet queued behind it ("head-of-line blocking") — exactly the kind of
  real, added latency a fast-tick, real-time UDP game like this one was deliberately designed to
  avoid (`PC_SNAPSHOT_HZ`'s own real bandwidth tuning this session, S206-72, would be undermined
  by a transport that trades bandwidth efficiency for ordering guarantees the game doesn't need).
- **WebRTC DataChannel** — **the real, recommended choice.** Configurable to `ordered: false,
  maxRetransmits: 0`, giving real unreliable, unordered delivery — the same real semantics
  `PcSnapshotPacket`'s own design already assumes (a dropped/late snapshot is a real, expected,
  harmless case; the next one supersedes it, `PC_CLIENT_WEAK_MS`/`PC_CLIENT_STALE_MS` already
  handle the client-side detection side of that). Real, honest cost: DataChannel runs over SCTP,
  which itself needs a real ICE/STUN handshake to establish a peer connection even for a real,
  simple "browser talks to one specific known server" case — genuinely more setup complexity than
  a plain WebSocket `connect()`, not free.

**Real, deliberate bridge shape**: a new, small, dumb relay process — NOT a second game server —
sitting between a browser's real WebRTC DataChannel and the real, existing `papercraft_server`'s
own UDP socket, translating packets in both directions byte-for-byte (same real header/struct
layout, no reinterpretation). All real game logic — movement integration, PARENA mod dispatch,
snapshot broadcast — stays exactly where it already lives, in the one real, existing C server.
This is the real, load-bearing design constraint that keeps this initiative from silently
becoming "build a second game server," a real, much bigger, much riskier undertaking than
"build a second client."

## Real architecture

```
Browser tab                              Existing infra (unchanged)
┌─────────────────────────────┐          ┌──────────────────────────┐
│ apps/web-client (new, TS)    │          │ apps/server (C, unchanged)│
│  - WebGL2 canvas renderer    │  WebRTC  │  - real UDP socket        │
│  - real input (keyboard/     │ DataChan │  - real game loop, mods,  │
│    mouse/gamepad via the     │◄────────►│    snapshot broadcast     │
│    real Gamepad API)         │   real   │        ▲                 │
│  - decodes the real, same    │  bridge  │        │ real, unchanged │
│    binary PcHeader/Pc*Packet │  process │        │ UDP             │
│    structs papercraft_       │◄────────►│ apps/web-bridge (new, Go  │
│    protocol.h defines        │          │  or C) -- dumb packet     │
└─────────────────────────────┘          │  relay only, no game logic│
                                          └──────────────────────────┘
```

**Real repo placement decision, deliberate and revisitable**: `apps/web-client` and
`apps/web-bridge` live inside the **existing PAPERCRAFT repo**, not a new, separate top-level
repo — unlike SAND/JEWEL/TTT/EXODUS's own real precedent of spinning up new repos for genuinely
separate codebases. Real reasoning: this client's own single source of truth for the wire format
is `packages/common/papercraft_protocol.h`, already living in this repo; keeping the web client
alongside it means that contract is always visible and co-versioned, not duplicated/synced across
repos the way a separate repo would require. `apps/client` (the existing SDL2 client) and
`apps/mapeditor` already establish "multiple real client apps, one repo" as this repo's own
existing convention — `apps/web-client` is a real, third instance of that same pattern, not a new
one. Revisit this call if the web client's own real build tooling (`npm`/`tsc`, distinct from
`apps/client`'s own Bazel/gcc toolchain) ever proves genuinely awkward to keep co-located — not a
concern found yet, just named honestly.

## Real "eat the interface boundary" tie-in (the founder's own second real point)

As more of PAPERCRAFT's own real gameplay decision logic moves into PARENA mods — already true
for `xp_award`/`item_drop`/`inventory`/`pickup`/`phone_message` (S206-65 through S206-67) — and
now that PARENA has a real TypeScript emitter too (`src/emit_ts.c`, proven against MISHRI's own
`bezier-interp`/`chance`/`random-int`/`gaussian-noise`, S206-71/S206-73), the real, honest
long-term shape this initiative points toward: `apps/web-client` could compile and call the
*exact same* PARENA-compiled decision logic (as real TypeScript) the C server already calls (as
real C) for anything genuinely shared — client-side prediction, cosmetic decisions the client can
render ahead of the next real snapshot, anything currently hand-duplicated per platform. **Real,
honest, NOT decided by this scoping pass**: which specific decisions, if any, actually need
client-side duplication at all (most of PAPERCRAFT's own real logic is server-authoritative by
design, `NORTHSTAR.md`'s own "smallest real proof point" for Phase 0 explicitly deferred client
prediction) — this is a real, promising DIRECTION this initiative opens up, not a concrete Phase 0
requirement.

## Real, phased plan

**Real Phase 0 — "a browser tab renders a triangle and echoes one real UDP round trip through the
bridge, nothing else"** — matching every other real Phase 0 in this monorepo's own "smallest real
proof point" convention (`NORTHSTAR.md`'s own original Phase 0: "a player can log in and spawn in
the real persistent city, nothing else"):
- `apps/web-bridge`: a real, minimal relay — accept one WebRTC DataChannel connection, forward
  raw bytes to/from `papercraft_server`'s own real UDP socket, nothing else (no auth, no
  multi-client fan-out yet).
- `apps/web-client`: a real WebGL2 context clearing to a solid color, a real
  `RTCPeerConnection`/`RTCDataChannel` connecting to the bridge, sending a real `PC_PACKET_CONNECT`
  and rendering SOMETHING different on screen once a real `PC_PACKET_WELCOME` comes back.
  Deliberately not real 3D rendering yet — proving the real network path end to end is the whole
  point of this phase, matching how `apps/client`'s own real Phase 0 proved connect/spawn before
  any of the real 3D city rendering existed.

**Real, later phases (design only, not detailed here)**: real city-chunk rendering (the same real
`worldapi` fetch `apps/client` already does, ported to `fetch()`), real player movement +
input, real Paper Engine destructible-geometry rendering, real HUD parity (ping/inventory/phone
notification — all real, already-proven server-side features this session shipped), real
multi-client fan-out in the bridge (today's Phase 0 bridge is deliberately single-connection).

## Real risks and open questions, named honestly

- **NAT traversal without a real TURN relay**: `apps/web-bridge` running on the same real,
  already-known-reachable box `papercraft_server` itself runs on (a real server with a real,
  stable public address, not two peer browsers behind separate home NATs) is the one real case
  STUN-only WebRTC setup handles cleanly — this is NOT the harder "two random browsers find each
  other" case WebRTC is usually reached for. Real, low risk for this specific topology, not
  zero — flagged, not dismissed.
- **WebGL2 vs. the existing client's own desktop OpenGL** feature/shader parity — real, unaudited
  gap; `apps/client/src/main.c`'s own real shader/rendering calls haven't been inventoried against
  WebGL2's own real, narrower capability set yet.
- **Browser tab backgrounding/throttling** — real browsers deliberately slow down or pause
  `requestAnimationFrame`/timers in a backgrounded tab; a real, live multiplayer session behaves
  differently there than a native SDL2 window ever would. Real, known browser-platform behavior,
  not something this initiative can configure away — a real UX/expectations question for later,
  not a Phase 0 blocker.
- **`apps/web-bridge`'s own implementation language** — not decided by this pass. Go (matching
  IDUNA/emily.cli/GoblinFoxDragon's own real Go convention elsewhere in this monorepo, and Go's
  own real, mature `pion/webrtc` DataChannel library) is the likely real candidate over a new C
  WebRTC implementation from scratch — flagged as the probable real choice, not committed to
  here.

## Explicitly not scoped yet

No real client/bridge code (this is a scoping-only pass, per the founder's own explicit "yes
start the northstar scoping pass," not "start building"). No decision on hosting/deployment for
`apps/web-bridge` (same box as `papercraft_server`, presumably, but not confirmed). No mobile/
touch-input story. No decision on whether `apps/web-client` ever reaches feature parity with
`apps/client`, or stays a deliberately lighter "quick look in a browser" experience permanently —
a real, open product question, not a technical one, left for the founder's own later call.

## Related

- `NORTHSTAR.md` — this repo's own main scoping doc; "Real, cross-platform client + map editor
  CI" section is the real precedent for "multiple real client build targets, one repo."
- `packages/common/papercraft_protocol.h` — the real, single source of truth for the wire format
  both `apps/web-bridge` and `apps/web-client` must speak byte-for-byte.
- `MISHRI/` — the real, first TypeScript codebase in this monorepo; its own real `tsconfig.json`/
  Bazel-wraps-npm convention (`MISHRI/BUILD.bazel`'s own doc comments) is the real template
  `apps/web-client`'s own build setup should start from, not reinvent.
- `PARENA/src/emit_ts.c`, `PARENA/STDLIB.md` — the real TypeScript emitter and its own
  `mishri` package tree; the real, concrete mechanism behind this doc's own "eat the interface
  boundary" section above.
