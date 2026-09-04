# NORTHSTAR — Papercraft engine normal maps

Real scoping pass for kanban cruise-queue card `PC-06667` ("PAPERCRAFT ENGINE normal maps north
star"). Per Principle 19 ("a big, unscoped ask gets scoped, not swallowed whole"): this doc
investigates the real, current rendering pipeline and writes a phased plan — no rendering code
is changed by this pass.

## Real, checked-live current state

`apps/client/src/main.c` (1779 lines) is **deliberately legacy, fixed-function OpenGL**
(`glBegin`/`glVertex`/`glColor3f`, no shaders, no VBOs/VAOs, no per-vertex attribute buffers) —
the file's own header comment says so explicitly, and S231's cel-shading pass (2026-09-02)
confirms it directly: REDGARDEN's own cel-shading is a real GLSL fragment shader
(`apps/arena/src/main.c`), and this client's own port of that technique is *not* a shader —
it's the identical 3-band light-quantization math done in plain C (`cel_color3f`/`cel_color4f`),
called once per face right before that face's own `glColor3f` call, entirely on the CPU. Lighting
today is exactly one fixed directional light (`PC_CEL_LIGHT_X/Y/Z`), banded into 3 flat
brightness tiers against each face's own analytically-known (city/player boxes) or
cross-product-computed (jittered Paper Engine fragments, via `cel_normal_from_quad`) normal.

**Real, decisive finding that changes this doc's own initial assumption**: `paper_mesh.h` DOES
already compute real per-face UV coordinates (`paper_face_grid`/`corner_uv`) — but checked
directly, those UVs are consumed only by the paper-fragment **subdivision** grid (how many
sub-quads a destructible face breaks into, and their shape), never passed to `glTexCoord2f` or
consumed by any texture-mapping code anywhere in `apps/client`. **There is no texture mapping
in this renderer at all today** — no `glTexImage2D`, no `glBindTexture`, no texture-unit setup
of any kind. Every surface is flat-colored per face (`glColor3f`), banded by the cel-shading
pass above. This is the real, honest starting point: normal maps need texture mapping to exist
first, and texture mapping doesn't exist yet, period — not a partially-built prerequisite.

## Why this is a real architectural fork, not a drop-in feature

A normal map only means something once per-pixel (or at minimum per-fragment) lighting reads a
surface-local normal from a texture instead of using one flat per-face normal. Legacy
fixed-function OpenGL has exactly two real paths to get there, and they're genuinely different
projects:

1. **Pivot to a modern (shader) pipeline.** Real per-pixel normal mapping the way REDGARDEN's
   own cel-shading already does it (a GLSL fragment shader) — sample a normal map texture,
   transform it into world/view space via a per-vertex tangent basis, quantize against the
   light the same way `cel_color3f` already does, but per-pixel instead of per-face. This is the
   technically "right" modern answer, and it's also the bigger lift: it requires standing up
   VBOs/VAOs, a real vertex-attribute pipeline (position/normal/UV/tangent, none of which exist
   as buffers today — everything is drawn immediate-mode per `glBegin` call), a shader compile/
   link path (REDGARDEN already has one to copy from), and a texture-loading path (`stbi_load`
   or similar — also does not exist in this client yet).
2. **Stay fixed-function, use DOT3 bump mapping.** The real, historically-correct pre-shader
   technique (`GL_ARB_texture_env_dot3` / `GL_ARB_texture_env_combine`, both from the actual
   fixed-function OpenGL 1.x era, still exposed by every desktop GL driver this client already
   targets): encode a per-pixel normal as an RGB texture, bind it as a second texture unit, and
   configure the fixed-function texture-combine stage to compute `N · L` per-texel instead of
   per-face. This keeps the "deliberately legacy fixed-function" identity this client's own
   header comment calls out as an intentional choice — real per-pixel lighting detail without a
   shader pipeline — at the cost of being a genuinely more obscure, harder-to-debug corner of
   the old fixed-function API (multitexture combine stages are configured via a pile of
   `glTexEnvi` enum soup, not readable shader source).

**This doc does not pick between them — that's a real, founder-level call** (how committed is
this engine to staying fixed-function vs. eventually needing a shader pipeline for other reasons
too, e.g. real specular/rim lighting, shadow mapping, or matching REDGARDEN's own visual bar more
closely). Whichever path is chosen, Phase 0 below (plain diffuse texture mapping) is the same
first step either way, and is real, valuable progress on its own regardless of which normal-map
path gets picked afterward.

## Real, phased plan

**Phase 0 — plain diffuse texture mapping (prerequisite, not yet started).** Nothing in this
client loads or samples a texture today. Real, minimal slice: an image-loading path (`stb_image`
single-header, matching the "small, vendorable, no new heavyweight dependency" convention this
repo's own `packages/common` already favors for e.g. `hud_text.h`), `glGenTextures`/
`glTexImage2D`/`glBindTexture` wiring, and actually emitting `glTexCoord2f` per vertex using
real UVs — the existing `paper_face_grid`/`corner_uv` math in `paper_mesh.h` is the right real
starting point for city-block/paper-fragment UVs, it just needs to be plumbed into the render
loop instead of only the subdivision-shape logic that consumes it today. Acceptance: a single
real texture (even a flat color-checker placeholder) visibly maps onto a city block face with
correct orientation/tiling.

**Phase 1 — pick and build the per-pixel lighting path (the real fork named above).** Either:
- **1a (shader pivot)**: stand up a GLSL vertex+fragment shader pipeline for at least the
  city/player-box render path (Paper Engine's jittered fragments can stay immediate-mode/
  per-face longer if needed — a real, deliberate partial-migration scope, not all-or-nothing),
  porting `cel_color3f`'s own 3-band quantization into the fragment shader the same way
  REDGARDEN's `FS_SRC` already does it, now per-pixel.
- **1b (DOT3 bump mapping)**: configure `GL_ARB_texture_env_combine`/`_dot3` on a second texture
  unit, encode the fixed light direction into the combine stage, verify per-texel `N · L` output
  against a known test normal map (a flat "0,0,1" tangent-space normal map should look identical
  to Phase 0's plain diffuse result — a real, checkable regression test for the combine setup
  itself before trusting any authored normal map content).

**Phase 2 — normal map asset pipeline.** Wherever the real block/prop art comes from (this
client currently draws flat-colored primitive boxes, not textured art assets at all — a real,
separate, earlier gap this doc doesn't attempt to close), an actual authored-or-generated normal
map needs to exist per material. Real, honest, not scoped here: whether these get hand-authored,
baked from higher-poly source geometry, or AI-generated (this monorepo's own Prompt-o-verse
pipeline is a real, existing precedent for AI-generated game art elsewhere, e.g. BRAWLPIT's own
hat catalog) is a separate, later decision.

**Phase 3 — cel-shading integration.** Whichever Phase 1 path is chosen, the real, final
integration point is combining per-pixel normal-map lighting with the existing 3-band cel
quantization (Phase 1a does this naturally inside the fragment shader; Phase 1b needs the
combine-stage output banded via a second combine stage or a small lookup table texture) so the
finished result still reads as this client's own established cel-shaded art direction, not
photorealistic bump-mapped lighting that would clash with it.

## Real, honest, explicitly out of scope for this pass

No rendering code changes. No texture-loading library added. No shader files written. No normal
map assets created or sourced. The Phase 1a/1b fork itself is not resolved — that's a real,
separate decision for whoever picks this up next, informed by this doc's own real, checked
findings about what does and doesn't already exist in `apps/client/src/main.c`.
