#ifndef FH6_DATA_HPP
#define FH6_DATA_HPP

// https://support.forza.net/hc/en-us/articles/51744149102611-Forza-Horizon-6-Data-Out-Documentation
// Total packet size: 324 bytes.
constexpr unsigned short TELEMETRY_SIZE = 324;

struct fh6_data {

// = 1 when race is on. = 0 when in menus/race stopped.
// NOTE: race also includes open world driving...
int IsRaceOn = 0;

// Can overflow to 0 eventually
unsigned int TimestampMS = 0;

// Engine RPM values
float EngineMaxRpm = 0;
float EngineIdleRpm = 0;
float CurrentEngineRpm = 0;

// In the car's local space; X = right, Y = up, Z = forward
float AccelerationX = 0;
float AccelerationY = 0;
float AccelerationZ = 0;

// In the car's local space; X = right, Y = up, Z = forward
float VelocityX = 0;
float VelocityY = 0;
float VelocityZ = 0;

// Angular velocity in the car's local space (rad/s); X = pitch, Y = yaw, Z = roll
float AngularVelocityX = 0;
float AngularVelocityY = 0;
float AngularVelocityZ = 0;

// Car orientation (radians)
float Yaw = 0;
float Pitch = 0;
float Roll = 0;

// Suspension travel normalized: 0.0f = max stretch; 1.0 = max compression
float NormalizedSuspensionTravelFrontLeft = 0;
float NormalizedSuspensionTravelFrontRight = 0;
float NormalizedSuspensionTravelRearLeft = 0;
float NormalizedSuspensionTravelRearRight = 0;

// Tire normalized slip ratio, = 0 means 100% grip and |ratio| > 1.0 means loss of grip.
float TireSlipRatioFrontLeft = 0;
float TireSlipRatioFrontRight = 0;
float TireSlipRatioRearLeft = 0;
float TireSlipRatioRearRight = 0;

// Wheel rotation speed radians/sec.
float WheelRotationSpeedFrontLeft = 0;
float WheelRotationSpeedFrontRight = 0;
float WheelRotationSpeedRearLeft = 0;
float WheelRotationSpeedRearRight = 0;

// = 1 when wheel is on rumble strip, = 0 when off.
int WheelOnRumbleStripFrontLeft = 0;
int WheelOnRumbleStripFrontRight = 0;
int WheelOnRumbleStripRearLeft = 0;
int WheelOnRumbleStripRearRight = 0;

// = 1 when wheel is in a puddle, = 0 when not.
int WheelInPuddleFrontLeft = 0;
int WheelInPuddleFrontRight = 0;
int WheelInPuddleRearLeft = 0;
int WheelInPuddleRearRight = 0;

// Non-dimensional surface rumble values passed to controller force feedback
float SurfaceRumbleFrontLeft = 0;
float SurfaceRumbleFrontRight = 0;
float SurfaceRumbleRearLeft = 0;
float SurfaceRumbleRearRight = 0;

// Tire normalized slip angle, = 0 means 100% grip and |angle| > 1.0 means loss of grip.
float TireSlipAngleFrontLeft = 0;
float TireSlipAngleFrontRight = 0;
float TireSlipAngleRearLeft = 0;
float TireSlipAngleRearRight = 0;

// Tire normalized combined slip, = 0 means 100% grip and |slip| > 1.0 means loss of grip.
float TireCombinedSlipFrontLeft = 0;
float TireCombinedSlipFrontRight = 0;
float TireCombinedSlipRearLeft = 0;
float TireCombinedSlipRearRight = 0;

// Actual suspension travel in meters
float SuspensionTravelMetersFrontLeft = 0;
float SuspensionTravelMetersFrontRight = 0;
float SuspensionTravelMetersRearLeft = 0;
float SuspensionTravelMetersRearRight = 0;

// Unique ID of the car make/model
int CarOrdinal = 0;

// Between 0 (D -- worst cars) and 7 (X class -- best cars) inclusive
int CarClass = 0;

// Between 100 (worst car) and 999 (best car) inclusive
int CarPerformanceIndex = 0;

// 0 = FWD, 1 = RWD, 2 = AWD
int DrivetrainType = 0;

// Number of cylinders in the engine
int NumCylinders = 0;

// Car group identifier
unsigned int CarGroup = 0;

// Velocity loss from smashable object collision (m/s)
float SmashableVelDiff = 0;

// Mass of recently hit smashable object (kg)
float SmashableMass = 0;

// Position in world space (meters)
float PositionX = 0;
float PositionY = 0;
float PositionZ = 0;

// Speed in meters per second
float Speed = 0;

// Power in watts
float Power = 0;

// Torque in newton-meters
float Torque = 0;

// Tire temperature
float TireTempFrontLeft = 0;
float TireTempFrontRight = 0;
float TireTempRearLeft = 0;
float TireTempRearRight = 0;

// Turbo/supercharger boost (PSI above atmospheric)
float Boost = 0;

// Fuel level (0.0 = empty, 1.0 = full)
float Fuel = 0;

// Total distance traveled (meters)
float DistanceTraveled = 0;

// Lap times (seconds); 0.0 if not applicable
float BestLap = 0;
float LastLap = 0;
float CurrentLap = 0;

// Total race time (seconds since driving started)
float CurrentRaceTime = 0;

// Number of laps completed
unsigned short LapNumber = 0;

// Current race position
unsigned char RacePosition = 0;

// Player inputs (0 to 255)
unsigned char Accel = 0;
unsigned char Brake = 0;
unsigned char Clutch = 0;
unsigned char HandBrake = 0;

// Current gear
unsigned char Gear = 0;

// Steering input (-127 = full left, 0 = center, 127 = full right)
char Steer = 0;

// Normalized driving line position (-127 to 127)
char NormalizedDrivingLine = 0;

// Normalized AI braking difference (-127 to 127)
char NormalizedAIBrakeDifference = 0;

};

#endif
