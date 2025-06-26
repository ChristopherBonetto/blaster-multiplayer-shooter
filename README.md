<div align="right">
  <a href="https://gitlab.com/Christopher.Bonetto/blaster_multiplayer">🗂️ Repository</a> • 
  <a href="https://www.linkedin.com/in/christopher-bonetto-547876221">💼 LinkedIn</a>
</div>

# 🎯 Blaster Multiplayer

> A stylized multiplayer arena shooter built in Unreal Engine 5, designed to learn and explore advanced gameplay programming, networking, and system architecture for online PvP combat.

![Main Menu](repo_assets/main-menu.png)

## 🎮 Gameplay
![Join Match](repo_assets/join-match.gif)  
![Combat Example](repo_assets/combat-example.gif)  
![Elimination + Score](repo_assets/elimination-score.gif)

## 🕹️ How to Play
- ⌨️ **WASD** – Move  
- ␣ **Space** – Jump  
- 🧎 **Ctrl** – Crouch  
- 🔄 **R** – Reload  
- 🖱️ **Left Click** – Fire  
- 🎯 **Right Click** – Aim

- 🖥️ **Host Game**: Click `Host` to start a multiplayer match.  
- 🌐 **Join Game**: Click `Join` to enter a hosted match.

## 🛠️ Build
_Build download link coming soon._

## ✅ Features
- Player movement using UE5's Enhanced Input System
- IK Animations and full locomotion system
- Health & Ammo management
- Game hosting and joining (Online Subsystem)
- Multiplayer HUD (Health bar, ammo, match info)
- Player elimination, respawn logic
- Weapon equip/drop, with multiple weapon types
- Initial implementation of pickups, match states, lag compensation
- Score and kill count system
- Custom materials for effects (e.g., dissolve effect on elimination)

## 🧠 Game Architecture
- **Input System**: Enhanced Input with modular input actions and mappings  
- **Animation System**: IK-based movement with blend spaces for various locomotion states  
- **Match Flow**:
  - Match states managed through replicated GameMode state machine  
  - Respawn system using PlayerController and SpawnPoint logic  
- **Networking**:
  - Fully multiplayer-focused with Online Subsystem support  
  - Replication implemented where necessary and supported by RPCs for server-authoritative logic  
  - Uses a custom Steam plugin to enable online sessions  
- **UI**: UMG HUDs synced to display dynamic player info  
- **Weapons**:
  - Base C++ weapon class extended via Blueprints  
  - Fire rate, ammo, cooldowns and visuals handled in C++

## 📸 Showcases
![Join Match](repo_assets/join-match.gif)  
![Combat Example](repo_assets/combat-example.gif)  
![Elimination + Score](repo_assets/elimination-score.gif)

## 🚧 In Development
- Weapon variety and balancing
- Lag compensation improvements
- Pickup item variety (boosts, ammo crates)
- Visual polish & VFX
- Steam-ready build packaging

## 📌 Notes
This is a work-in-progress personal project created to deepen my knowledge of Unreal Engine 5’s multiplayer systems, game architecture, replication, and online services.

---

Made with ❤️ by Christopher Bonetto