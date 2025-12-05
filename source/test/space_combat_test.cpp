#include "StarSpaceCombatTypes.hpp"
#include "StarSpaceCombatWorld.hpp"

#include "gtest/gtest.h"

using namespace Star;

// Test SpaceCombatShipState serialization
TEST(SpaceCombatTest, ShipStateSerialization) {
  SpaceCombatShipState original;
  original.position = Vec2F(100.0f, 200.0f);
  original.velocity = Vec2F(10.0f, -5.0f);
  original.rotation = 1.5f;
  original.angularVelocity = 0.2f;
  original.mass = 150.0f;
  original.maxThrust = 600.0f;
  original.maxTurnRate = 2.5f;

  DataStreamBuffer buffer;
  buffer << original;

  SpaceCombatShipState loaded;
  buffer.seek(0);
  buffer >> loaded;

  EXPECT_FLOAT_EQ(original.position[0], loaded.position[0]);
  EXPECT_FLOAT_EQ(original.position[1], loaded.position[1]);
  EXPECT_FLOAT_EQ(original.velocity[0], loaded.velocity[0]);
  EXPECT_FLOAT_EQ(original.velocity[1], loaded.velocity[1]);
  EXPECT_FLOAT_EQ(original.rotation, loaded.rotation);
  EXPECT_FLOAT_EQ(original.angularVelocity, loaded.angularVelocity);
  EXPECT_FLOAT_EQ(original.mass, loaded.mass);
  EXPECT_FLOAT_EQ(original.maxThrust, loaded.maxThrust);
  EXPECT_FLOAT_EQ(original.maxTurnRate, loaded.maxTurnRate);
}

// Test SpaceCombatInput serialization
TEST(SpaceCombatTest, InputSerialization) {
  SpaceCombatInput original;
  original.thrustForward = true;
  original.thrustBackward = false;
  original.turnLeft = true;
  original.turnRight = false;
  original.fire = true;
  original.aimDirection = Vec2F(0.707f, 0.707f);

  DataStreamBuffer buffer;
  buffer << original;

  SpaceCombatInput loaded;
  buffer.seek(0);
  buffer >> loaded;

  EXPECT_EQ(original.thrustForward, loaded.thrustForward);
  EXPECT_EQ(original.thrustBackward, loaded.thrustBackward);
  EXPECT_EQ(original.turnLeft, loaded.turnLeft);
  EXPECT_EQ(original.turnRight, loaded.turnRight);
  EXPECT_EQ(original.fire, loaded.fire);
  EXPECT_FLOAT_EQ(original.aimDirection[0], loaded.aimDirection[0]);
  EXPECT_FLOAT_EQ(original.aimDirection[1], loaded.aimDirection[1]);
}

// Test SpaceCombatProjectileState serialization
TEST(SpaceCombatTest, ProjectileStateSerialization) {
  SpaceCombatProjectileState original;
  original.id = 12345;
  original.ownerShipUuid = Uuid();
  original.position = Vec2F(500.0f, 600.0f);
  original.velocity = Vec2F(300.0f, 0.0f);
  original.damage = 25.0f;
  original.timeToLive = 4.5f;

  DataStreamBuffer buffer;
  buffer << original;

  SpaceCombatProjectileState loaded;
  buffer.seek(0);
  buffer >> loaded;

  EXPECT_EQ(original.id, loaded.id);
  EXPECT_EQ(original.ownerShipUuid, loaded.ownerShipUuid);
  EXPECT_FLOAT_EQ(original.position[0], loaded.position[0]);
  EXPECT_FLOAT_EQ(original.position[1], loaded.position[1]);
  EXPECT_FLOAT_EQ(original.velocity[0], loaded.velocity[0]);
  EXPECT_FLOAT_EQ(original.velocity[1], loaded.velocity[1]);
  EXPECT_FLOAT_EQ(original.damage, loaded.damage);
  EXPECT_FLOAT_EQ(original.timeToLive, loaded.timeToLive);
}

// Test SpaceCombatConfig loading from JSON
TEST(SpaceCombatTest, ConfigFromJson) {
  Json config = Json::parse(R"({
    "arenaWidth": 3000.0,
    "arenaHeight": 2500.0,
    "defaultShipMass": 200.0,
    "defaultThrust": 800.0,
    "defaultTurnRate": 3.0,
    "shipDrag": 0.15,
    "projectileSpeed": 400.0,
    "projectileDamage": 15.0,
    "tickRate": 30.0
  })");

  SpaceCombatConfig loaded = SpaceCombatConfig::fromJson(config);

  EXPECT_FLOAT_EQ(loaded.arenaWidth, 3000.0f);
  EXPECT_FLOAT_EQ(loaded.arenaHeight, 2500.0f);
  EXPECT_FLOAT_EQ(loaded.defaultShipMass, 200.0f);
  EXPECT_FLOAT_EQ(loaded.defaultThrust, 800.0f);
  EXPECT_FLOAT_EQ(loaded.defaultTurnRate, 3.0f);
  EXPECT_FLOAT_EQ(loaded.shipDrag, 0.15f);
  EXPECT_FLOAT_EQ(loaded.projectileSpeed, 400.0f);
  EXPECT_FLOAT_EQ(loaded.projectileDamage, 15.0f);
  EXPECT_FLOAT_EQ(loaded.tickRate, 30.0f);
}

// Test SpaceCombatConfig defaults for missing values
TEST(SpaceCombatTest, ConfigDefaults) {
  SpaceCombatConfig defaults = SpaceCombatConfig::fromJson(Json());
  
  EXPECT_FLOAT_EQ(defaults.arenaWidth, 2000.0f);
  EXPECT_FLOAT_EQ(defaults.arenaHeight, 2000.0f);
  EXPECT_FLOAT_EQ(defaults.defaultShipMass, 100.0f);
  EXPECT_FLOAT_EQ(defaults.defaultThrust, 500.0f);
  EXPECT_FLOAT_EQ(defaults.defaultTurnRate, 2.0f);
  EXPECT_FLOAT_EQ(defaults.shipDrag, 0.1f);
  EXPECT_FLOAT_EQ(defaults.projectileSpeed, 300.0f);
  EXPECT_FLOAT_EQ(defaults.projectileDamage, 10.0f);
  EXPECT_FLOAT_EQ(defaults.tickRate, 20.0f);
}
