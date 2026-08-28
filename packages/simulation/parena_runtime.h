#ifndef PAPERCRAFT_PARENA_RUNTIME_H
#define PAPERCRAFT_PARENA_RUNTIME_H

/* parena_runtime.h -- every `parena build`-generated .c file in this repo #includes this. Same
 * real, deliberately-minimal approach WEAKNIGHT_BEDROCK_RACERS' own packages/simulation/
 * parena_runtime.h already established (see that file's own header comment for the full
 * reasoning): PARENA's real, canonical runtime (PARENA/runtime/parena_runtime.h) is 1600+ lines
 * because it backs the full language; this repo's own first mod (level_mod.prn) is pure I32
 * scalar arithmetic + recursion -- no Arena, no String, no Vec, no host FFI -- so it needs
 * nothing beyond what the generated file already #includes directly. Confirmed by actually
 * compiling against this empty stub, not assumed.
 *
 * Grow this file for real, matching ECOWAR's own eventual growth, the moment a future mod
 * genuinely needs Arena/String/Vec/host-glue support.
 */

#endif
