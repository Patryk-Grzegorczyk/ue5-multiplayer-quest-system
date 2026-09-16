# UE5 Multiplayer Player-Specific Quest System

A simplified portfolio extraction of the quest architecture developed for **DeepAnomaly**, a cooperative horror extraction game built with Unreal Engine 5 and C++.

The production system integrates with the game's character, inventory, UI and other gameplay systems. Those dependencies are intentionally removed from this sample so the focus remains on multiplayer quest state and player-specific world interactions.

## Features

- Server-authoritative quest progression
- Replicated active and finished quest state
- Multi-step quest progress
- Player-specific QuestActor visibility
- Client RPCs for player-specific presentation
- Per-player labour timers
- Location, special-item and labour interactions
- Separation between player quest state and world actors

## Architecture

```text
                    SERVER
                      |
          +-----------+-----------+
          |                       |
          v                       v
 PlayerQuestComponent         QuestActor
          |                       |
 Active / Finished               |
    Quest State                   |
          |                       |
          +-----------+-----------+
                      |
                Client RPC
                      |
          +-----------+-----------+
          |                       |
       Player A                Player B
          |                       |
    Actor hidden              Actor visible
    for Player A              for Player B
```

## Player-Specific World State

A QuestActor can remain a shared, server-managed world actor while its presentation differs between players.

When a player completes a special-item objective, the server sends a Client RPC to that player's connection. The client hides the actor locally instead of destroying the shared actor.

```text
                    QuestActor
                       |
                 Server-managed
                       |
             +---------+---------+
             |                   |
          Player A            Player B
             |                   |
        Quest complete        Quest active
             |                   |
        Actor hidden          Actor visible
```

This allows cooperative players to interact with the same world while maintaining independent quest progress.

## Per-Player Labour Timers

Labour objectives can require a player to remain in an interaction for a configurable amount of time.

The QuestActor maintains a separate timer for each interacting player:

```cpp
TMap<APlayerController*, FTimerHandle> ActivePlayerTimers;
```

A timer can be started or cancelled for one player without modifying another player's active interaction.

## Quest Progress

A quest can contain multiple objective entries:

```cpp
TArray<bool> QuestProgress;
```

Completing an objective updates its corresponding index. The quest becomes completable only after every progress entry is satisfied.

## Networking

Quest state is stored on a replicated player-owned component.

```text
Client interaction
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
Quest presentation
```

Quest progression is performed on the server. `OnRep_ActiveQuests` and `OnRep_FinishedQuests` provide client-side state-change hooks.

## Why Client-Specific Visibility?

Destroying a shared replicated actor would affect the actor's replicated lifetime for all relevant clients.

For player-specific quest objectives, the actor can remain in the world while a Client RPC changes its local visibility and collision state for the relevant player.

This is useful when different cooperative players can have different quest progress while sharing the same level.

## Design Decisions

### Player-Owned Quest State

Active and finished quests belong to a player's `UPlayerQuestComponent`.

This keeps quest progression independent between cooperative players.

### World Actor + Player State

`AQuestActor` handles world interaction, while `UPlayerQuestComponent` owns the player's quest state.

This separates:

- world interaction
- player quest state
- multiplayer replication
- presentation

### Per-Player Timers

Labour interactions use a timer map keyed by `APlayerController*`, allowing multiple players to use the same actor independently.

## Source Relationship

This repository is **not the complete DeepAnomaly quest implementation**.

It is an intentionally simplified extraction of the architecture developed for the game, adapted to remove game-specific dependencies and focus on the multiplayer quest concepts demonstrated here.

## Unreal Engine

Designed for **Unreal Engine 5 / C++** projects.

The generated module API macro used in this sample is:

```cpp
QUESTSYSTEM_API
```

Replace it with the API macro generated for the Unreal module where these classes are integrated.

## License

Portfolio-oriented code sample derived from a system developed for DeepAnomaly.
