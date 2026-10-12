# Git Commit Conventions

Format: `type(scope): description`

## Types

| Type | When to use |
|---|---|
| `feat` | New feature or behavior |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Formatting, no logic change |
| `refactor` | Restructuring without changing behavior |
| `test` | Adding or fixing tests |
| `chore` | Maintenance, dependencies, tooling |
| `build` | Build system, CMake, compiler flags |
| `revert` | Revert a previous commit |
| `perf` | Performance improvement |

## Rules

- **Lowercase** everything
- **Imperative mood** in the description: "add X", not "added X"
- **Subject line under 72 characters**
- Optional body separated by a blank line for extra detail

## Examples

```
feat(GameState): add faction standing map
fix(GameState): correct modifyCounter assignment bug
build(cmake): add initial CMakeLists.txt
docs(design): update GameState category table
refactor(GameState): remove redundant this-> usage
```
