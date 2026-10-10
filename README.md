# Project Zombite

Two-player co-op indirect-control zombie outbreak game, built with Unreal Engine 5.7 (ArenaGame C++ module; Lab13_StarterKit baseline).

## Rules
- Two human players operate observer cameras; no player possesses a zombie.
- Two starter zombie AI, one assigned to each player's PlayerState.
- Players emit noise; the owning starter zombie prioritizes its player's noise, but seeing a human takes precedence.
- Bitten humans convert to independent zombie AI. Infect all humans before the round timer expires.
- The listen server alone decides AI, infection, spawning, timing, win/loss.

## Team work
- `feature/zombie-interaction`: zombie AI, owner priority, observer and noise input.
- `feature/human-gameplay`: human AI, infection, match rules/UI.
- `main`: integrated, tested builds only.

**Do not rename ArenaGame.uproject / ArenaGame C++ module yet.** The binary Lab 13 starter Source/Content base must be added locally using Git LFS before compiling. This repository is not yet a playable build.
