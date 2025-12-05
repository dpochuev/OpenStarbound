#pragma once

#include "StarSpaceCombatTypes.hpp"
#include "StarNetPackets.hpp"
#include "StarUuid.hpp"

namespace Star {

STAR_CLASS(SpaceCombatClient);

// Client-side state tracking for space combat
// Receives state updates from server, provides interpolated positions for rendering
class SpaceCombatClient {
public:
  SpaceCombatClient();
  ~SpaceCombatClient() = default;

  // Connection state
  bool inCombat() const;
  void enterCombat(Vec2F arenaSize, List<pair<Uuid, ByteArray>> initialShips);
  void leaveCombat();

  // Ship state management
  void updateShipState(Uuid const& shipUuid, SpaceCombatShipState const& state);
  void removeShip(Uuid const& shipUuid);
  
  // Get all ships for rendering
  HashMap<Uuid, SpaceCombatShipState> const& ships() const;
  Maybe<SpaceCombatShipState> shipState(Uuid const& shipUuid) const;

  // Projectile management
  void spawnProjectile(SpaceCombatProjectileState const& projectile);
  void removeProjectile(uint64_t projectileId);
  List<SpaceCombatProjectileState> const& projectiles() const;

  // Local input (sent to server)
  void setLocalInput(SpaceCombatInput const& input);
  SpaceCombatInput const& localInput() const;

  // Client-side interpolation update
  void update(float dt);

  // Get arena size
  Vec2F arenaSize() const;

  // Own ship UUID (set when entering combat)
  void setOwnShipUuid(Uuid const& uuid);
  Maybe<Uuid> ownShipUuid() const;

private:
  bool m_inCombat = false;
  Vec2F m_arenaSize;
  Maybe<Uuid> m_ownShipUuid;

  HashMap<Uuid, SpaceCombatShipState> m_ships;
  List<SpaceCombatProjectileState> m_projectiles;
  SpaceCombatInput m_localInput;

  // For interpolation
  HashMap<Uuid, SpaceCombatShipState> m_previousShipStates;
  float m_interpolationTime = 0.0f;
  float m_interpolationDuration = 0.05f; // 50ms interpolation
};

}
