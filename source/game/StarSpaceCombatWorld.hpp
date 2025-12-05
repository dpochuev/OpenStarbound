#pragma once

#include "StarSpaceCombatTypes.hpp"
#include "StarUuid.hpp"
#include "StarGameTypes.hpp"
#include "StarNetPackets.hpp"

namespace Star {

STAR_CLASS(Clock);
STAR_CLASS(WorldServer);
STAR_STRUCT(Packet);

// Server-side space combat arena where multiple ships coexist
// Each ship is still represented by its full ShipWorld with tiles
// The combat world tracks positions/physics for the arena view
class SpaceCombatWorld {
public:
  SpaceCombatWorld(ClockConstPtr universeClock);
  ~SpaceCombatWorld() = default;

  // Configuration
  SpaceCombatConfig const& config() const;
  bool isEnabled() const;

  // Ship management
  void addShip(ConnectionId clientId, Uuid const& shipUuid, Vec2F initialPosition);
  void removeShip(ConnectionId clientId);
  bool hasShip(ConnectionId clientId) const;
  
  // Get ship state for networking
  Maybe<SpaceCombatShipState> shipState(ConnectionId clientId) const;
  List<ConnectionId> allShipConnections() const;

  // Input handling
  void setShipInput(ConnectionId clientId, SpaceCombatInput const& input);

  // Physics update
  void update(float dt);

  // Projectile management
  void spawnProjectile(ConnectionId ownerClient, Vec2F position, Vec2F velocity);
  List<SpaceCombatProjectileState> const& projectiles() const;

  // Networking
  List<PacketPtr> pullOutgoingPackets(ConnectionId clientId);
  void handleIncomingPacket(ConnectionId clientId, PacketPtr packet);

  // Check for projectile hits against ships
  // Returns list of (clientId, damage) pairs for ships that were hit
  List<pair<ConnectionId, float>> checkProjectileHits();

private:
  struct CombatShip {
    Uuid shipUuid;
    SpaceCombatShipState state;
    SpaceCombatInput currentInput;
    float fireCooldown = 0.0f;
  };

  void updateShipPhysics(CombatShip& ship, float dt);
  void updateProjectiles(float dt);
  void queueStateUpdates();
  
  // Calculate ship hitbox based on position (simplified rectangle for now)
  RectF shipHitbox(SpaceCombatShipState const& state) const;

  ClockConstPtr m_universeClock;
  SpaceCombatConfig m_config;
  bool m_enabled = false;

  HashMap<ConnectionId, CombatShip> m_ships;
  List<SpaceCombatProjectileState> m_projectiles;
  uint64_t m_nextProjectileId = 1;
  
  float m_timeSinceUpdate = 0.0f;
  float m_updateInterval;  // Based on tickRate

  // Outgoing packets per client
  HashMap<ConnectionId, List<PacketPtr>> m_outgoingPackets;
};

}
