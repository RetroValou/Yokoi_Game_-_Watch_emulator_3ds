"""Lightweight timing utilities.

This module is intentionally standalone (no dependencies) so it can be imported
from scripts like convert_3ds.py when/if we decide to add first-class timing
output again.

Example:
    from source.timing import TimingCollector

    timing = TimingCollector()

    with timing.scope("load_inputs"):
        load_stuff()

    with timing.scope("build_game"):
        build_game()

    print(timing.format_report())

Notes:
- Uses time.perf_counter() for high-resolution wall-clock timing.
- Supports repeated scopes with the same name (accumulates total + count).
- Scope names are free-form strings; hierarchical names like "game/foo/step" work
  nicely for later grouping.
"""

from __future__ import annotations

from dataclasses import dataclass
from time import perf_counter
from typing import Dict, Iterable, Iterator, Optional


@dataclass
class TimingStat:
    total_s: float = 0.0
    count: int = 0

    @property
    def avg_s(self) -> float:
        return self.total_s / self.count if self.count else 0.0


class _TimingScope:
    def __init__(self, collector: "TimingCollector", name: str):
        self._collector = collector
        self._name = name
        self._start: Optional[float] = None

    def __enter__(self) -> "_TimingScope":
        self._start = perf_counter()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        if self._start is None:
            return
        elapsed = perf_counter() - self._start
        self._collector.add(self._name, elapsed)


class TimingCollector:
    """Collects coarse timing stats keyed by a string name."""

    def __init__(self) -> None:
        self._stats: Dict[str, TimingStat] = {}

    def add(self, name: str, seconds: float) -> None:
        stat = self._stats.get(name)
        if stat is None:
            stat = TimingStat()
            self._stats[name] = stat
        stat.total_s += float(seconds)
        stat.count += 1

    def scope(self, name: str) -> _TimingScope:
        """Context manager scope that records elapsed wall time under `name`."""

        return _TimingScope(self, name)

    def names(self) -> Iterable[str]:
        return self._stats.keys()

    def get(self, name: str) -> TimingStat:
        return self._stats.get(name, TimingStat())

    def clear(self) -> None:
        self._stats.clear()

    def format_report(
        self,
        *,
        sort: str = "total",
        limit: Optional[int] = None,
        title: str = "Timing",
    ) -> str:
        """Return a human-readable timing table.

        Args:
            sort: "total" (default), "avg", "count", or "name".
            limit: If provided, only show the top N rows.
            title: Optional header line.
        """

        items = list(self._stats.items())

        if sort == "name":
            items.sort(key=lambda kv: kv[0])
        elif sort == "count":
            items.sort(key=lambda kv: kv[1].count, reverse=True)
        elif sort == "avg":
            items.sort(key=lambda kv: kv[1].avg_s, reverse=True)
        else:  # "total"
            items.sort(key=lambda kv: kv[1].total_s, reverse=True)

        if limit is not None:
            items = items[: max(0, int(limit))]

        if not items:
            return f"{title}: (no timings)"

        total_all = sum(stat.total_s for _, stat in items)

        # Column widths
        name_w = max(len(name) for name, _ in items)
        lines = [title, "=" * max(10, len(title))]
        lines.append(f"{'name'.ljust(name_w)}  {'total(s)':>10}  {'avg(ms)':>10}  {'count':>7}")
        lines.append(f"{'-' * name_w}  {'-' * 10}  {'-' * 10}  {'-' * 7}")

        for name, stat in items:
            lines.append(
                f"{name.ljust(name_w)}  {stat.total_s:10.3f}  {stat.avg_s * 1000.0:10.1f}  {stat.count:7d}"
            )

        lines.append("")
        lines.append(f"shown total: {total_all:.3f}s (scopes may overlap if nested)")
        return "\n".join(lines)
