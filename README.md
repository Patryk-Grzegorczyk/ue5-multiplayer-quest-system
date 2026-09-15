# UE5 Multiplayer Player-Specific Quest System

A simplified portfolio extraction of the quest architecture developed for **DeepAnomaly**, a cooperative horror extraction game built with Unreal Engine 5 and C++.

The original production system integrates with the game's character, inventory, UI, notifications and other gameplay systems. Those dependencies are intentionally removed here so the example focuses on multiplayer quest state and player-specific world interactions.

## Features

- Server-authoritative quest progression
- Replicated active and finished quest state
- Multi-step quest progress
- Player-specific QuestActor visibility
- Client RPCs for presentation changes
- Per-player labour timers
- Location, special-item and labour quest interactions
- Separation between quest state and world interaction

## Architecture

```text
                    Server
                      |
          +-----------+-----------+
          |                       |
          v                       v
 PlayerQuestComponent         QuestActor
          |                       |
   Active / Finished              |
      Quests                      |
          |                       |
          +-----------+-----------+
                      |
                 Player-specific
                    RPC
                      |
          +-----------+-----------+
          |                       |
       Player A                Player B
          |                       |
     Actor visible          Actor hidden
     / hidden                independently
```

## Player-Specific World State

QuestActors are shared world actors, but their presentation can differ between players.

When a player receives or completes a quest, the server can request a client-specific visibility change through a Client RPC instead of destroying the shared actor globally.

For example:

```text
                 QuestActor
                     |
              Server-managed
                     |
          +----------+----------+
          |                     |
       Player A              Player B
          |                     |
      Quest done             Quest active
          |                     |
      Actor hidden            Actor visible
```

This allows a single world actor to represent a quest objective while each player can have an independent view of that objective.

## Per-Player Timers

Labour objectives can require a player to remain in an interaction for a configurable amount of time.

The actor maintains a separate timer handle for each interacting player:

```cpp
TMap<APlayerController*, FTimerHandle> ActivePlayerTimers;
```

This means multiple players can interact with the same QuestActor independently without sharing one global timer.

A player can also cancel their own active timer without affecting another player's interaction.

## Quest Progress

A quest can contain multiple progress entries:

```cpp
TArray<bool> QuestProgress;
```

Completing one objective updates its corresponding index. The quest becomes completable only after all progress entries have been satisfied.

This supports objectives composed of multiple world interactions without requiring a separate actor for every quest state.

## Networking

Quest state is stored on a replicated player-owned component:

```text
Client input
     |
     v
Server
     |
     | validate / update
     v
PlayerQuestComponent
     |
     | replicated state
     v
Owning client
     |
     v
Quest UI / presentation
```

World interactions are processed on the server. Player-specific visual changes are sent back through Client RPCs.

## Why Client-Specific Visibility?

Destroying a shared replicated actor would affect all clients.

For player-specific objectives, the actor can instead remain part of the shared world while its local presentation is changed for the relevant player.

This approach is useful when different players can have different quest progress while exploring the same multiplayer level.

## Design Decisions

### Player-Owned Quest State

Active and finished quests belong to the player's quest component rather than being stored as global world state.

This keeps quest progression independent between cooperative players.

### World Actor + Player State

QuestActor handles interaction with the world, while PlayerQuestComponent owns the player's quest progress.

This separates:

- world interaction
- player quest state
- multiplayer replication
- presentation

### Simplified Portfolio Extraction

The original DeepAnomaly implementation contains additional integrations for inventory rewards, notifications, quest UI, item requirements and other gameplay systems.

Those systems are intentionally excluded from this repository to keep the example focused on the multiplayer quest architecture.

## Source Relationship

This repository is **not the complete DeepAnomaly quest implementation**.

It is an intentionally simplified extraction of the architecture developed for the game, adapted to remove game-specific dependencies and focus on the networking and quest-state concepts demonstrated here.

## Unreal Engine

Designed for **Unreal Engine 5 / C++** projects.

The generated module API macro used in this sample is:

```cpp
QUESTSYSTEM_API
```

When integrating these files into another Unreal Engine module, replace it with that module's generated API macro.

## License

Portfolio-oriented code sample derived from a system developed for DeepAnomaly.
