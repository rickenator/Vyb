# vyb-traffic

`vyb-traffic` archives GitHub traffic for `rickenator/Vyb` in a local SQLite
database. GitHub exposes repository clone and view traffic for only a rolling
14-day window. Running this utility daily retains each per-day bucket and keeps
historical snapshots of referrers and popular content after GitHub stops
returning them.

SQLite is the source of truth. CSV files and SVG graphs are derived exports and
can be deleted and regenerated at any time.

The tool is written in Vyb end to end: `traffic.vyb` is the engine (config,
collection, storage, report, export, graphs) and `vyb-traffic.vyb` is the CLI.
There is no Python in this directory.

## Accounting model

Each collection stores both kinds of data returned by GitHub:

- The aggregate clone/view counts and uniques for GitHub's current rolling
  14-day window, as a timestamped snapshot.
- Every individual clone/view date bucket. A date is unique in each daily
  table, and later observations update that date with SQLite `UPSERT` semantics.

Summing the archived daily `count` values gives exact **archived clone events**
and **archived view events** for the period that has been collected.

GitHub does **not** expose stable anonymous identities for cloners or viewers.
Consequently, `vyb-traffic` cannot calculate a true lifetime number of unique
people. The rolling 14-day unique totals and daily unique observations are
stored, but summing daily uniques is explicitly labeled as observations—not
lifetime unique users. The same person may be counted on multiple days.

## Requirements and installation

- Linux with a built Vyb toolchain: `build/vyb` and `stdlib/` in the repository
- A GitHub token for an account with push access to the repository

Nothing needs to be installed. Build Vyb once from the repository root and run
the checked-in launcher, which locates the build and sets `VYB_STDLIB`:

```sh
cmake -S . -B build && cmake --build build -j
./vyb-traffic/vyb-traffic --help
```

The launcher execs `build/vyb vyb-traffic/vyb-traffic.vyb --module-path bindings`.
Set `VYB` to use another compiler build. Charts are SVG, so no plotting library
is required.

## Token setup

Create a token appropriate for repository traffic access and place it in the
environment. Avoid putting it directly on a command line or committing it:

```sh
read -rsp 'GitHub token: ' GITHUB_TOKEN
export GITHUB_TOKEN
printf '\n'
```

The token is used only in the `Authorization: Bearer` request header. It is
never written to the database, printed, or logged.

Optional environment variables:

| Variable | Default |
| --- | --- |
| `GITHUB_TOKEN` | required for `collect` |
| `VYB_TRAFFIC_OWNER` | `rickenator` |
| `VYB_TRAFFIC_REPO` | `Vyb` |
| `VYB_TRAFFIC_DB` | `~/.local/share/vyb-traffic/traffic.db` |

The export and graph locations can additionally be changed with
`VYB_TRAFFIC_EXPORT_DIR` and `VYB_TRAFFIC_GRAPHS_DIR`, or their CLI options.

## Commands

Collect all four GitHub traffic endpoints:

```sh
./vyb-traffic/vyb-traffic collect
```

Each endpoint is fetched independently. If one fails, successful responses are
stored atomically and the command exits `2`; unavailable rolling values remain
`NULL`, not false zeroes. A complete run exits `0`, and a fatal configuration or
SQLite error exits `1`. Concise diagnostics go to stderr.

Print the current rolling window and archived event totals:

```sh
./vyb-traffic/vyb-traffic report
./vyb-traffic/vyb-traffic report --recent 30
```

Export CSV views to `~/.local/share/vyb-traffic/export/`:

```sh
./vyb-traffic/vyb-traffic export
```

This creates `summary.csv`, `clone_daily.csv`, `view_daily.csv`,
`referrers.csv`, and `popular_paths.csv`.

Generate SVG charts under `~/.local/share/vyb-traffic/graphs/`:

```sh
./vyb-traffic/vyb-traffic graph
```

The output is `daily-clones.svg`, `daily-views.svg`, `cumulative-clones.svg`,
and `cumulative-views.svg`. Cumulative charts sum daily event counts only;
daily unique observations are never summed into them.

Global overrides may appear anywhere on the command line, for example:

```sh
./vyb-traffic/vyb-traffic report --db /srv/private/vyb-traffic.db
./vyb-traffic/vyb-traffic export --export-dir ./csv
```

## Daily scheduling with a systemd user timer

The supplied user timer runs every day at 03:15 local time and catches up after
a missed run. Install it after checking out the repository:

```sh
mkdir -p ~/.config/systemd/user ~/.config/vyb-traffic
install -m 0644 systemd/vyb-traffic.service ~/.config/systemd/user/
install -m 0644 systemd/vyb-traffic.timer ~/.config/systemd/user/
printf 'GITHUB_TOKEN=%s\n' "$GITHUB_TOKEN" > ~/.config/vyb-traffic/env
chmod 600 ~/.config/vyb-traffic/env
systemctl --user daemon-reload
systemctl --user enable --now vyb-traffic.timer
```

Optional `VYB_TRAFFIC_*` assignments may be added to the same environment file.
The service runs the checked-in launcher, so build Vyb before enabling the
timer. Check the schedule and recent result with:

```sh
systemctl --user list-timers vyb-traffic.timer
journalctl --user -u vyb-traffic.service
```

### Cron alternative

Systemd user timers are preferred. For cron, the same protected environment
file can be sourced explicitly:

```cron
15 3 * * * . "$HOME/.config/vyb-traffic/env" && export GITHUB_TOKEN VYB_TRAFFIC_OWNER VYB_TRAFFIC_REPO VYB_TRAFFIC_DB && "$HOME/Projects/Vyb/vyb-traffic/vyb-traffic" collect
```

## Database schema and durability

The normalized tables are:

- `snapshots`: one row per collection timestamp with rolling clone/view totals
  and endpoint-success flags.
- `clone_daily` and `view_daily`: one row per date with `count`, `uniques`,
  `first_seen`, and `last_seen`.
- `referrers` and `popular_paths`: timestamped rows tied to each snapshot.

Writes use transactions, foreign keys, WAL journaling, and date-keyed UPSERTs.
Repeating a collection timestamp is idempotent. Referrer and path rows are
replaced only when that endpoint succeeds, while snapshots from older
collection times are retained.

## Testing

The tests are Vyb programs under `test/traffic/`. They use fixed payloads and a
scratch database in `$TMPDIR` and never contact the network:

```sh
for t in test/traffic/test_traffic_*.vyb; do
    VYB_STDLIB="$PWD/stdlib" ./build/vyb "$t" \
        --module-path vyb-traffic --module-path bindings
done
```

The full suite (`test/run_tests.vyb`) runs them too.
