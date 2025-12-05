#include "StarSpaceCombatClient.hpp"
#include "StarDataStream.hpp"
#include "StarLogging.hpp"

namespace Star {

SpaceCombatClient::SpaceCombatClient() {}

bool SpaceCombatClient::inCombat() const {
  return m_inCombat;
}

void SpaceCombatClient::enterCombat(Vec2F arenaSize, List<pair<Uuid, ByteArray>> initialShips) {
  m_inCombat = true;
  m_arenaSize = arenaSize;
  m_ships.clear();
  m_projectiles.clear();
  m_previousShipStates.clear();
  m_interpolationTime = 0.0f;

  // Load initial ship states
  for (auto const& shipData : initialShips) {
    DataStreamBuffer ds(shipData.second);
    SpaceCombatShipState state;
    ds >> state;
    m_ships[shipData.first] = state;
  }

  Logger::info("SpaceCombatClient: Entered combat arena {}x{} with {} ships", 
               arenaSize[0], arenaSize[1], initialShips.size());
}

void SpaceCombatClient::leaveCombat() {
  m_inCombat = false;
  m_ships.clear();
  m_projectiles.clear();
  m_previousShipStates.clear();
  m_ownShipUuid = {};
  Logger::info("SpaceCombatClient: Left combat arena");
}

void SpaceCombatClient::updateShipState(Uuid const& shipUuid, SpaceCombatShipState const& state) {
  // Store previous state for interpolation
  if (auto existing = m_ships.ptr(shipUuid))
    m_previousShipStates[shipUuid] = *existing;
  
  m_ships[shipUuid] = state;
  m_interpolationTime = 0.0f;
}

void SpaceCombatClient::removeShip(Uuid const& shipUuid) {
  m_ships.remove(shipUuid);
  m_previousShipStates.remove(shipUuid);
}

HashMap<Uuid, SpaceCombatShipState> const& SpaceCombatClient::ships() const {
  return m_ships;
}

Maybe<SpaceCombatShipState> SpaceCombatClient::shipState(Uuid const& shipUuid) const {
  if (auto ship = m_ships.ptr(shipUuid))
    return *ship;
  return {};
}

void SpaceCombatClient::spawnProjectile(SpaceCombatProjectileState const& projectile) {
  m_projectiles.append(projectile);
}

void SpaceCombatClient::removeProjectile(uint64_t projectileId) {
  m_projectiles.filter([projectileId](SpaceCombatProjectileState const& p) {
    return p.id != projectileId;
  });
}

List<SpaceCombatProjectileState> const& SpaceCombatClient::projectiles() const {
  return m_projectiles;
}

void SpaceCombatClient::setLocalInput(SpaceCombatInput const& input) {
  m_localInput = input;
}

SpaceCombatInput const& SpaceCombatClient::localInput() const {
  return m_localInput;
}

void SpaceCombatClient::update(float dt) {
  if (!m_inCombat)
    return;

  // Update interpolation
  m_interpolationTime += dt;

  // Client-side projectile prediction (simple linear movement)
  for (auto& proj : m_projectiles) {
    proj.position += proj.velocity * dt;
    proj.timeToLive -= dt;
  }

  // Remove expired projectiles
  m_projectiles.filter([](SpaceCombatProjectileState const& p) {
    return p.timeToLive > 0;
  });
}

Vec2F SpaceCombatClient::arenaSize() const {
  return m_arenaSize;
}

void SpaceCombatClient::setOwnShipUuid(Uuid const& uuid) {
  m_ownShipUuid = uuid;
}

Maybe<Uuid> SpaceCombatClient::ownShipUuid() const {
  return m_ownShipUuid;
}

}
