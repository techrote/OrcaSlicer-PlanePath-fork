# OrcaSlicer PlanePath — planning README

This file preserves the repository's original planning-front-page text after importing the OrcaSlicer upstream tree. The root `README.md` is intentionally left to upstream OrcaSlicer so upstream merges remain low-conflict.

## Project objective

Research and implementation campaign for a custom OrcaSlicer build with **script-defined solid-fill plane paths**, including a growing library of printable space-filling curves.

Extend OrcaSlicer's existing `FillPlanePath` architecture so user-installable, sandboxed scripts can emit geometry into Orca's native clipping/spacing/flow/toolpath pipeline. Scripts describe path topology; Orca remains authoritative for printable geometry, extrusion and G-code.

Initial scope is **internal solid fill, top surfaces and bottom surfaces**. Sparse/scripted structural infill is deliberately deferred until the solid-fill architecture is proven.

## RAG corpus

Read `docs/rag/INDEX.md` first. It routes implementation agents to the project brief, upstream seam map, code audit, architecture, ABI/sandbox, traits/config portability, curve research, CI strategy, roadmap and autonomous execution contract.

## Governing principle

A custom pattern script must never become a G-code postprocessor. It produces bounded geometric path primitives; existing OrcaSlicer code remains responsible for clipping against printable surfaces, line/flow semantics, extrusion and final G-code.

Primary upstream: https://github.com/OrcaSlicer/OrcaSlicer
