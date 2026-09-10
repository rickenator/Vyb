// SPDX-License-Identifier: Apache-2.0

// Interactive Vyb REPL (`vyb repl`, issue #154 / Testing & Tooling).
//
// Statement/declaration-oriented read-eval-print loop backed by the existing ORC
// JIT executor: each submission is appended to an accumulated session (top-level
// declarations + a synthetic `main() -> { ... }` body) and evaluated by spawning
// the same `vyb` binary on a temp file (JIT in a subprocess). This reuses the
// exact runtime/JIT path of `vyb file.vyb`, gives clean per-eval stdout/stderr
// capture, and isolates a crashing program from the REPL process.
//
// A bare single-line expression is auto-wrapped as `println(<expr>)` so its value
// is shown (println is generic over scalar types); declarations, assignments, and
// control statements run verbatim.

#ifndef VYB_REPL_HPP
#define VYB_REPL_HPP

#include <string>

namespace vyb {
namespace repl {

// Runs the interactive loop on stdin/stdout until :quit / :exit / EOF.
// selfExe must be an absolute path to this `vyb` binary (used to spawn evals).
int runRepl(const std::string& selfExe);

} // namespace repl
} // namespace vyb

#endif // VYB_REPL_HPP
