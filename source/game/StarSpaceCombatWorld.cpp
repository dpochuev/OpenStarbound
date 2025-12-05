#include "StarSpaceCombatWorld.hpp"
#include "StarRoot.hpp"
#include "StarConfiguration.hpp"
#include "StarLogging.hpp"
#include "StarAssets.hpp"
#include "StarTime.hpp"

namespace Star {

SpaceCombatWorld::SpaceCombatWorld(ClockConstPtr universeClock)
  : m_universeClock(std::move(universeClock)) {
  
  // Load config from configuration
  auto& root = Root::singleton();
  auto config = root.configuration();
  
  m_enabled = config->get("spaceCombatEnabled", false).toBool();
  
  if (m_enabled) {
    try {
      auto combatConfig = root.assets()->json("/spacecombat.config");
      m_config = SpaceCombatConfig::fromJson(combatConfig);
      Logger::info("SpaceCombatWorld: Enabled with arena size {}x{}", 
                   m_config.arenaWidth, m_config.arenaHeight);
    } catch (std::exception const& e) {
      Logger::warn("SpaceCombatWorld: Failed to load config, using defaults: {}", e.what());
      m_config = SpaceCombatConfig();
    }
  }
  
  m_updateInterval = 1.0f / m_config.tickRate;
}

SpaceCombatConfig const& SpaceCombatWorld::config() const {
  return m_config;
}

bool SpaceCombatWorld::isEnabled() const {
  return m_enabled;
}

void SpaceCombatWorld::addShip(ConnectionId clientId, Uuid const& shipUuid, Vec2F initialPosition) {
  if (!m_enabled)
    return;
    
  CombatShip ship;
  ship.shipUuid = shipUuid;
  ship.state.position = initialPosition;
  ship.state.velocity = Vec2F();
  ship.state.rotation = 0.0f;
  ship.state.angularVelocity = 0.0f;
  ship.state.mass = m_config.defaultShipMass;
  ship.state.maxThrust = m_config.defaultThrust;
  ship.state.maxTurnRate = m_config.defaultTurnRate;
  ship.currentInput = SpaceCombatInput();
  ship.fireCooldown = 0.0f;
  
  m_ships[clientId] = std::move(ship);
  
  Logger::info("SpaceCombatWorld: Added ship for client {} at position {}", 
               clientId, initialPosition);
}

void SpaceCombatWorld::removeShip(ConnectionId clientId) {
  if (m_ships.remove(clientId)) {
    Logger::info("SpaceCombatWorld: Removed ship for client {}", clientId);
  }
  m_outgoingPackets.remove(clientId);
}

bool SpaceCombatWorld::hasShip(ConnectionId clientId) const {
  return m_ships.contains(clientId);
}

Maybe<SpaceCombatShipState> SpaceCombatWorld::shipState(ConnectionId clientId) const {
  if (auto ship = m_ships.ptr(clientId))
    return ship->state;
  return {};
}

List<ConnectionId> SpaceCombatWorld::allShipConnections() const {
  return m_ships.keys();
}

void SpaceCombatWorld::setShipInput(ConnectionId clientId, SpaceCombatInput const& input) {
  if (auto ship = m_ships.ptr(clientId)) {
    ship->currentInput = input;
  }
}

void SpaceCombatWorld::update(float dt) {
  if (!m_enabled || m_ships.empty())
    return;

  // Update ship physics
  for (auto& pair : m_ships) {
    updateShipPhysics(pair.second, dt);
  }

  // Update projectiles
  updateProjectiles(dt);

  // Check for hits
  auto hits = checkProjectileHits();
  for (auto const& hit : hits) {
    // In full implementation, this would apply damage to the ship's tiles
    Logger::debug("SpaceCombatWorld: Ship {} hit for {} damage", hit.first, hit.second);
  }

  // Queue state updates at tick rate
  m_timeSinceUpdate += dt;
  if (m_timeSinceUpdate >= m_updateInterval) {
    m_timeSinceUpdate = 0.0f;
    queueStateUpdates();
  }
}

void SpaceCombatWorld::updateShipPhysics(CombatShip& ship, float dt) {
  auto& state = ship.state;
  auto const& input = ship.currentInput;

  // Apply rotation
  if (input.turnLeft)
    state.angularVelocity = state.maxTurnRate;
  else if (input.turnRight)
    state.angularVelocity = -state.maxTurnRate;
  else
    state.angularVelocity = 0.0f;

  state.rotation += state.angularVelocity * dt;

  // Calculate thrust direction
  Vec2F thrustDir = Vec2F::withAngle(state.rotation);

  // Apply thrust
  Vec2F acceleration;
  if (input.thrustForward)
    acceleration = thrustDir * (state.maxThrust / state.mass);
  else if (input.thrustBackward)
    acceleration = -thrustDir * (state.maxThrust / state.mass) * 0.5f;  // Reverse is slower

  // Apply acceleration
  state.velocity += acceleration * dt;

  // Apply drag
  state.velocity *= (1.0f - m_config.shipDrag * dt);

  // Update position
  state.position += state.velocity * dt;

  // Wrap around arena (or clamp)
  state.position[0] = fmod(state.position[0] + m_config.arenaWidth, m_config.arenaWidth);
  state.position[1] = fmod(state.position[1] + m_config.arenaHeight, m_config.arenaHeight);
  if (state.position[0] < 0) state.position[0] += m_config.arenaWidth;
  if (state.position[1] < 0) state.position[1] += m_config.arenaHeight;

  // Handle firing
  if (ship.fireCooldown > 0)
    ship.fireCooldown -= dt;

  if (input.fire && ship.fireCooldown <= 0) {
    // Spawn projectile
    Vec2F projectileVel = input.aimDirection.normalized() * m_config.projectileSpeed + state.velocity;
    Vec2F spawnPos = state.position + input.aimDirection.normalized() * 20.0f;  // Offset from ship center
    
    SpaceCombatProjectileState proj;
    proj.id = m_nextProjectileId++;
    proj.ownerShipUuid = ship.shipUuid;
    proj.position = spawnPos;
    proj.velocity = projectileVel;
    proj.damage = m_config.projectileDamage;
    proj.timeToLive = 5.0f;  // 5 seconds lifetime
    
    m_projectiles.append(std::move(proj));
    ship.fireCooldown = 0.2f;  // Fire rate limit
  }
}

void SpaceCombatWorld::updateProjectiles(float dt) {
  m_projectiles.filter([&](SpaceCombatProjectileState& proj) {
    proj.position += proj.velocity * dt;
    proj.timeToLive -= dt;
    
    // Remove if expired or out of bounds
    if (proj.timeToLive <= 0)
      return false;
    if (proj.position[0] < 0 || proj.position[0] > m_config.arenaWidth ||
        proj.position[1] < 0 || proj.position[1] > m_config.arenaHeight)
      return false;
      
    return true;
  });
}

void SpaceCombatWorld::spawnProjectile(ConnectionId ownerClient, Vec2F position, Vec2F velocity) {
  auto ship = m_ships.ptr(ownerClient);
  if (!ship)
    return;

  SpaceCombatProjectileState proj;
  proj.id = m_nextProjectileId++;
  proj.ownerShipUuid = ship->shipUuid;
  proj.position = position;
  proj.velocity = velocity;
  proj.damage = m_config.projectileDamage;
  proj.timeToLive = 5.0f;
  
  m_projectiles.append(std::move(proj));
}

List<SpaceCombatProjectileState> const& SpaceCombatWorld::projectiles() const {
  return m_projectiles;
}

RectF SpaceCombatWorld::shipHitbox(SpaceCombatShipState const& state) const {
  // Simplified hitbox - in full implementation would be calculated from ship structure
  float halfWidth = 20.0f;
  float halfHeight = 10.0f;
  return RectF(state.position - Vec2F(halfWidth, halfHeight),
               state.position + Vec2F(halfWidth, halfHeight));
}

List<pair<ConnectionId, float>> SpaceCombatWorld::checkProjectileHits() {
  List<pair<ConnectionId, float>> hits;
  
  m_projectiles.filter([&](SpaceCombatProjectileState& proj) {
    for (auto& shipPair : m_ships) {
      // Don't hit own ship
      if (shipPair.second.shipUuid == proj.ownerShipUuid)
        continue;
        
      RectF hitbox = shipHitbox(shipPair.second.state);
      if (hitbox.contains(proj.position)) {
        hits.append({shipPair.first, proj.damage});
        return false;  // Remove projectile
      }
    }
    return true;  // Keep projectile
  });
  
  return hits;
}

void SpaceCombatWorld::queueStateUpdates() {
  // In full implementation, would create proper network packets
  // For now, just logging
  for (auto const& clientId : m_ships.keys()) {
    if (!m_outgoingPackets.contains(clientId))
      m_outgoingPackets[clientId] = {};
  }
}

List<PacketPtr> SpaceCombatWorld::pullOutgoingPackets(ConnectionId clientId) {
  if (auto packets = m_outgoingPackets.ptr(clientId)) {
    auto result = std::move(*packets);
    packets->clear();
    return result;
  }
  return {};
}

void SpaceCombatWorld::handleIncomingPacket(ConnectionId clientId, PacketPtr packet) {
  // Handle incoming packets - to be implemented with proper packet types
}

}
