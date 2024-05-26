- `rogue/`
  - `backend/`
    - rendering backends with identical APIs, to support both graphical and terminal gameplay
  - `core/`
    - core game code, divided into "modules" (not actual c++ modules). each module has its own `tests` directory containing unit tests
    - `common/`
      - utilities and services with no dependencies that aren't very game-specific
    - `core/`
      - backend interactions, input management, screen management, etc
    - `game/`
      - core gameplay code. `main` is here
    - `generation/`
      - level generation code.
  - `gdb/`
    - gdb helpers
  - `resources/`
    - game assets like sprites, fonts, sounds, etc
  - `demos/`
    - demo tests for backends, pathfinding, level generation, etc
  - `scripts/`
    - various helper scripts
  - `test-platform/`
    - custom unit testing library
  - `Taskfile.yml`
    - `go-task` task runner file for easy building/running

`core` is divided into "modules" (again, not actual c++ modules). The core architectural invariant here is that the modules' include graph should form a DAG.

Within a module, some circular dependencies between components is fine and to be expected. Between modules, however, there should never be a cyclic dependency.

If any files from module A include files from module B, then *zero* files from modele B are allowed to include files from module A.
