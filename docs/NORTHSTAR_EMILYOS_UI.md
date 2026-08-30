# NORTHSTAR — EmilyOS UI in PAPERCRAFT (the DUNG-native in-game experience)

## Where this came from

Founder real-time, 2026-08-30, immediately after BURROW's own Phases 1-4 shipped and DUNG's own
UX foundation was adopted from `EmilyOS/docs/legacy-archive/gui-v0.1-design-capture.md`:

> "okok so lets start building emily os into papercraft i want the DUNG native experience" →
> "like we can start to have workspaces per account" → "papercraft notes mod" → "papercraft bash
> mod" → "parena"

**Real, direct reading**: bring EmilyOS's own real UX design — the SAME "tmux × i3 hybrid" tile
system, EGSHELL visual language, and double-click-speed interaction contract `DUNG` already
adopted directly from that same real doc — into PAPERCRAFT as a real, in-game UI layer, with a
per-account workspace, a notes mod, and a bash/terminal mod, all built as real PARENA mods
("parena," the founder's own one-word confirmation of the implementation approach), matching this
whole session's own "mods first everything" discipline.

**Scoping only, this pass**, with one real, concrete first slice attempted alongside it (the notes
mod — see "Real, phased plan" below for why that one and not the others, this same pass).

## Real, existing foundation this builds on directly

Checked before designing anything new, not assumed: PAPERCRAFT already has a real, working,
proven "in-game OS-adjacent UI" precedent — the phone system
(`PARENA/stdlib/papercraft/phone_mod.prn`, `TYLER/engine/tyler_phone_mechanics.md`'s own Phase 1).
Its real, established shape is exactly the template this doc follows:

- **Real PARENA mod owns the decision** (`on-papercraft-phone-message-for-event`: given an event
  code, which message-id, if any, should fire) — pure I32 dispatch, the same real, narrow VS0
  ceiling every mod in this monorepo respects.
- **Real host C owns everything VS0 genuinely can't do**: the actual message TEXT (a shared,
  hardcoded `PC_PHONE_MESSAGE_TABLE` lookup, identical on client and server — real, deliberate
  departure from `tyler_phone_mechanics.md`'s own JSON wire format, since PAPERCRAFT's own
  established convention is fixed-size binary structs, not JSON-over-UDP, and VS0 can't produce a
  JSON string itself regardless), building and sending the real packet
  (`PcPhoneMessagePacket`/`PC_PACKET_PHONE_MESSAGE`), and rendering (`draw_phone_notification`, a
  real, small, non-disruptive top-of-screen banner, timed via `PC_PHONE_BANNER_MS`).
- **Real, proven end to end**: server-side trigger → mod decision → packet → client render, a real,
  live, working slice already shipped.

The notes/bash/workspace systems this doc scopes are the real, natural next real UI surfaces on
top of that exact same proven shape — not a new architecture, an extension of one already working.

## Real UX foundation — the same one `DUNG` already adopted, not a second, separate design

`EmilyOS/docs/legacy-archive/gui-v0.1-design-capture.md` is adopted here directly too, for the
same real reason it was adopted for `DUNG`: "I want the DUNG native experience" is the founder's
own explicit instruction to reuse it, not invent a parallel visual language for PAPERCRAFT. The
real, load-bearing pieces (full detail in that doc, not repeated in full here — see `DUNG/
NORTHSTAR.md`'s own real summary of the same spec for the parallel, already-written version):

- Deep navy/near-black background, EGSHELL (white-ish) reserved for directory/container tiles,
  colored (never white) button tiles, all-caps blocky typography, single-shot activation flashes.
- No single-click actions: fast double-click (≤220ms) activates, slow double-click (350-800ms)
  edits a label.
- The real "tmux × i3 hybrid" tiling layout — a tree of panes, real keyboard-first split/focus/
  resize operations.
- Posture-aware, non-modal denial feedback (a real, quiet, one-frame "deny flash," never a modal
  dialog or toast) — real, direct relevance here too: PAPERCRAFT already has its own real
  server-authoritative permission model (who can interact with what), the same real shape a
  posture-style gate would sit behind.

**Real, new design signal, not yet reconciled with anything above**: "and whatever the browser is
doing to make this file viewable is like cliutch it works so good" (immediately followed by a link
to `EMILY/BACKLOG.md`'s own raw GitHub view) — a real, live aspiration for the notes mod's own
rendering quality (nicely-formatted text display, not a raw dump), named honestly here as a real
design bar to aim for, not yet a concrete rendering spec.

## Real per-account workspaces

"Like we can start to have workspaces per account" — each real player gets their own real
workspace (an i3-style pane tree, scoped to that one account), the same real per-account
separation PAPERCRAFT's own existing systems already establish (each player's own inventory,
phone, faction standing are already per-account state, server-authoritative). A workspace is the
real, natural per-account CONTAINER for the notes/bash/other panes this doc scopes — not a new
multiplayer-shared UI surface, a real, private, per-player one, matching how the existing phone
notification banner is already only ever shown to the one real player it's addressed to.

## Real notes mod

A real, in-game notes system — the closest real analog to a file/directory tile in the adopted
EmilyOS spec, and the real, safest, most tractable first slice (no command execution, no real
security surface, a real, direct extension of the phone system's own already-proven "mod decides
content-id, host renders text" shape).

**Real, attempted this same pass** (see `PARENA/stdlib/papercraft/notes_mod.prn`): real, narrow
v0 scope, matching `phone_mod.prn`'s own exact real ABI shape — a real note has an owning player,
a real, bounded set of decision points (can this player create/read/delete a given note slot) a
PARENA mod can own, while the actual TEXT content, storage, and rendering stay real, honest host-C
concerns (VS0 has no real string-storage/persistence story beyond what `phone_mod.prn`'s own
"shared hardcoded table" convention already proves out for FIXED content — a real, USER-authored,
variable-length note is a genuinely different, bigger real problem than a fixed message table,
flagged honestly below, not glossed over).

## Real bash mod — scoped, NOT built this pass, real security concern named directly

A real, in-game terminal/command-line pane — matching `EmilyOS`'s own real "command-line / verb-bar
panel" pane type, and `DUNG`'s own terminal-emulator half. **Real, deliberate decision: not
attempted as code this pass.** Real reason, named honestly, not silently deferred: this is a real,
live, multiplayer game server — a "bash mod" implies real command execution, and PAPERCRAFT has no
existing real analog to a permission/sandboxing model for that (the phone/notes systems are pure
data-decision dispatch, never code execution). `EmilyOS`'s own real posture-gated design
(`NORMAL`/`SIEGE`/`MERCY`/`INCIDENT`/`GAME`, capability-checked verbs, tamper-evident audit
logging) is the real, right reference architecture for this specific piece — a real, direct
integration point, not a coincidence given EmilyOS's own UX is already being adopted here. **Real,
concrete next step before any bash-mod code**: design what a real, in-game "verb" actually means
here (almost certainly NOT arbitrary shell execution — a real, curated, capability-gated command
set, matching EmilyOS's own real verb model: `ENTER`/`PAUSE`/`RESUME`/etc., not `bash -c`) — a real,
separate scoping pass, not rushed into this doc.

## Real, phased plan

**Phase 0 — this doc.** Real UX foundation adopted, real existing precedent (`phone_mod.prn`)
identified, real security concern for the bash mod named and deliberately deferred.

**Phase 1 — the notes mod (real, attempted this same pass)**: `PARENA/stdlib/papercraft/
notes_mod.prn`, real decision logic only (matching `phone_mod.prn`'s own real scope), no real host
wiring yet (same real "compiled, tested, NOT yet wired into a live host" status `xp_award_mod.prn`/
`level_mod.prn` carried before their own host wiring landed) — real host wiring (a new
`PC_PACKET_NOTE_*` wire format, client-side rendering) is real, separate, next work.

**Phase 2 — the real workspace/tile UI framework** (design only, not detailed here): the real
i3-primitive pane system this doc's own "per-account workspaces" section names, shared
infrastructure the notes/future-bash panes both render inside — a real, bigger, client-side
rendering undertaking (SDL2, matching `PAPERCRAFT`'s own existing client stack), not scoped at the
implementation level yet.

**Phase 3 — the bash mod** (design only, explicitly deferred pending a real, separate security/
verb-model scoping pass — see above).

## Real risks and open questions, named honestly

- **Variable-length, user-authored note TEXT has no real storage/persistence story yet** —
  `phone_mod.prn`'s own "shared hardcoded table" convention only works for a small, FIXED,
  developer-authored message set; a real note a player actually writes is a genuinely different,
  bigger problem (real storage, real per-account persistence, real wire-format for arbitrary-
  length text over PAPERCRAFT's own fixed-size-struct convention) — flagged, not solved by Phase 1.
- **The bash mod's own real security model is completely unscoped** — deliberately, see above.
- **The workspace/tile UI framework (Phase 2) is a real, substantial client-side rendering
  undertaking** — no real estimate given here.
- **The "cliutch" rendering-quality bar is real but unspecified** — named as a real aspiration,
  not yet a concrete spec.

## Related

- `PARENA/stdlib/papercraft/phone_mod.prn`, `TYLER/engine/tyler_phone_mechanics.md` — the real,
  proven "mod decides, host renders" template this whole doc follows.
- `EmilyOS/docs/legacy-archive/gui-v0.1-design-capture.md` — the real, authoritative UX spec, the
  same one `DUNG` already adopted.
- `EmilyOS/internal/posture/`, `EmilyOS/internal/verb/` — the real, right reference architecture
  for the bash mod's own eventual real security/verb model.
- `DUNG/NORTHSTAR.md` — the real, parallel "DUNG native experience" this doc brings into
  PAPERCRAFT; both draw on the identical real UX foundation, deliberately not two separate designs.
