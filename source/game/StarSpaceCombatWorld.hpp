#pragma once

#include "StarSpaceCombatTypes.hpp"
#include "StarUuid.hpp"
#include "StarGameTypes.hpp"
#include "StarNetPackets.hpp"
#include "StarWorldServerThread.hpp"
#include "StarTileDamage.hpp"

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

  // Ship management - now with WorldServerThread reference for tile access
  void addShip(ConnectionId clientId, Uuid const& shipUuid, Vec2F initialPosition, WorldServerThreadPtr shipWorld = nullptr);
  void removeShip(ConnectionId clientId);
  bool hasShip(ConnectionId clientId) const;
  
  // Update ship's WorldServerThread reference (called when ship world becomes available)
  void setShipWorld(ConnectionId clientId, WorldServerThreadPtr shipWorld);
  
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

  // Check for projectile hits against ships and apply tile damage
  // Returns list of (clientId, damage, hitPosition) tuples for ships that were hit
  List<tuple<ConnectionId, float, Vec2F>> checkProjectileHitsAndApplyDamage();

  // Calculate ship characteristics from its tile structure
  struct ShipCharacteristics {
    float mass = 100.0f;       // Calculated from total tile count/weight
    float maxThrust = 500.0f;  // From engine blocks
    float maxTurnRate = 2.0f;  // From thruster placement
    RectF hitbox;              // Bounding box of ship tiles
    int tileCount = 0;         // Total tiles (for HP calculation)
  };
  Maybe<ShipCharacteristics> calculateShipCharacteristics(ConnectionId clientId);

private:
  struct CombatShip {
    Uuid shipUuid;
    SpaceCombatShipState state;
    SpaceCombatInput currentInput;
    float fireCooldown = 0.0f;
    WorldServerThreadPtr shipWorld;  // Reference to the actual ShipWorld
    RectF cachedHitbox;              // Cached ship hitbox in local coordinates
    int cachedTileCount = 0;         // Cached tile count for characteristic updates
  };

  void updateShipPhysics(CombatShip& ship, float dt);
  void updateProjectiles(float dt);
  void queueStateUpdates();
  
  // Calculate ship hitbox based on position and cached local hitbox
  RectF shipHitbox(CombatShip const& ship) const;
  
  // Apply tile damage to a ship at the given local position
  void applyTileDamage(CombatShip& ship, Vec2F localHitPos, float damage);
  
  // Convert arena position to ship-local tile position
  Vec2F arenaToShipLocal(CombatShip const& ship, Vec2F arenaPos) const;

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
