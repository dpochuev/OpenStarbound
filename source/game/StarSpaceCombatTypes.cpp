#include "StarSpaceCombatTypes.hpp"
#include "StarDataStream.hpp"

namespace Star {

SpaceCombatConfig SpaceCombatConfig::fromJson(Json const& config) {
  SpaceCombatConfig result;
  if (config.isType(Json::Type::Object)) {
    result.arenaWidth = config.getFloat("arenaWidth", result.arenaWidth);
    result.arenaHeight = config.getFloat("arenaHeight", result.arenaHeight);
    result.defaultShipMass = config.getFloat("defaultShipMass", result.defaultShipMass);
    result.defaultThrust = config.getFloat("defaultThrust", result.defaultThrust);
    result.defaultTurnRate = config.getFloat("defaultTurnRate", result.defaultTurnRate);
    result.shipDrag = config.getFloat("shipDrag", result.shipDrag);
    result.projectileSpeed = config.getFloat("projectileSpeed", result.projectileSpeed);
    result.projectileDamage = config.getFloat("projectileDamage", result.projectileDamage);
    result.tickRate = config.getFloat("tickRate", result.tickRate);
  }
  return result;
}

DataStream& operator>>(DataStream& ds, SpaceCombatShipState& state) {
  ds >> state.position;
  ds >> state.velocity;
  ds >> state.rotation;
  ds >> state.angularVelocity;
  ds >> state.mass;
  ds >> state.maxThrust;
  ds >> state.maxTurnRate;
  return ds;
}

DataStream& operator<<(DataStream& ds, SpaceCombatShipState const& state) {
  ds << state.position;
  ds << state.velocity;
  ds << state.rotation;
  ds << state.angularVelocity;
  ds << state.mass;
  ds << state.maxThrust;
  ds << state.maxTurnRate;
  return ds;
}

DataStream& operator>>(DataStream& ds, SpaceCombatInput& input) {
  ds >> input.thrustForward;
  ds >> input.thrustBackward;
  ds >> input.turnLeft;
  ds >> input.turnRight;
  ds >> input.fire;
  ds >> input.aimDirection;
  return ds;
}

DataStream& operator<<(DataStream& ds, SpaceCombatInput const& input) {
  ds << input.thrustForward;
  ds << input.thrustBackward;
  ds << input.turnLeft;
  ds << input.turnRight;
  ds << input.fire;
  ds << input.aimDirection;
  return ds;
}

DataStream& operator>>(DataStream& ds, SpaceCombatProjectileState& state) {
  ds >> state.id;
  ds >> state.ownerShipUuid;
  ds >> state.position;
  ds >> state.velocity;
  ds >> state.damage;
  ds >> state.timeToLive;
  return ds;
}

DataStream& operator<<(DataStream& ds, SpaceCombatProjectileState const& state) {
  ds << state.id;
  ds << state.ownerShipUuid;
  ds << state.position;
  ds << state.velocity;
  ds << state.damage;
  ds << state.timeToLive;
  return ds;
}

}
