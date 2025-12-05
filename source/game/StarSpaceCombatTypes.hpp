#pragma once

#include "StarUuid.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

// Forward declarations
STAR_CLASS(SpaceCombatShip);
STAR_CLASS(SpaceCombatProjectile);
STAR_CLASS(SpaceCombatWorld);

// Combat ship state - position and physics in the combat arena
struct SpaceCombatShipState {
  Vec2F position;       // Position in combat arena coordinates
  Vec2F velocity;       // Current velocity
  float rotation;       // Ship rotation in radians
  float angularVelocity; // Rotation speed

  // Calculated from ship structure
  float mass;           // Affects acceleration
  float maxThrust;      // Maximum thrust force
  float maxTurnRate;    // Maximum rotation speed
};

// Player input for ship control
struct SpaceCombatInput {
  bool thrustForward = false;
  bool thrustBackward = false;
  bool turnLeft = false;
  bool turnRight = false;
  bool fire = false;
  Vec2F aimDirection = {1.0f, 0.0f};  // Normalized aim direction
};

// Configuration for space combat
struct SpaceCombatConfig {
  static SpaceCombatConfig fromJson(Json const& config);
  
  float arenaWidth = 2000.0f;
  float arenaHeight = 2000.0f;
  float defaultShipMass = 100.0f;
  float defaultThrust = 500.0f;
  float defaultTurnRate = 2.0f;
  float shipDrag = 0.1f;
  float projectileSpeed = 300.0f;
  float projectileDamage = 10.0f;
  float tickRate = 20.0f;  // Updates per second
};

// Simple projectile for space combat
// Note: For the full implementation, these will interact with ShipWorld tiles
struct SpaceCombatProjectileState {
  uint64_t id;
  Uuid ownerShipUuid;
  Vec2F position;
  Vec2F velocity;
  float damage;
  float timeToLive;
};

DataStream& operator>>(DataStream& ds, SpaceCombatShipState& state);
DataStream& operator<<(DataStream& ds, SpaceCombatShipState const& state);

DataStream& operator>>(DataStream& ds, SpaceCombatInput& input);
DataStream& operator<<(DataStream& ds, SpaceCombatInput const& input);

DataStream& operator>>(DataStream& ds, SpaceCombatProjectileState& state);
DataStream& operator<<(DataStream& ds, SpaceCombatProjectileState const& state);

}
