# Telemetry Plan

## Available variables

``` cpp
int IsRaceOn

float VelocityX
float VelocityY
float VelocityZ

float AngularVelocityX
float AngularVelocityY
float AngularVelocityZ

float Yaw
float Pitch
float Roll

float NormalizedSuspensionTravelFrontLeft
float NormalizedSuspensionTravelFrontRight
float NormalizedSuspensionTravelRearLeft
float NormalizedSuspensionTravelRearRight

float TireSlipRatioFrontLeft
float TireSlipRatioFrontRight
float TireSlipRatioRearLeft
float TireSlipRatioRearRight

float WheelRotationSpeedFrontLeft
float WheelRotationSpeedFrontRight
float WheelRotationSpeedRearLeft
float WheelRotationSpeedRearRight

int WheelOnRumbleStripFrontLeft
int WheelOnRumbleStripFrontRight
int WheelOnRumbleStripRearLeft
int WheelOnRumbleStripRearRight

int WheelInPuddleFrontLeft
int WheelInPuddleFrontRight
int WheelInPuddleRearLeft
int WheelInPuddleRearRight

float SurfaceRumbleFrontLeft
float SurfaceRumbleFrontRight
float SurfaceRumbleRearLeft
float SurfaceRumbleRearRight

float TireSlipAngleFrontLeft
float TireSlipAngleFrontRight
float TireSlipAngleRearLeft
float TireSlipAngleRearRight

float TireCombinedSlipFrontLeft
float TireCombinedSlipFrontRight
float TireCombinedSlipRearLeft
float TireCombinedSlipRearRight

float SuspensionTravelMetersFrontLeft
float SuspensionTravelMetersFrontRight
float SuspensionTravelMetersRearLeft
float SuspensionTravelMetersRearRight

int CarOrdinal

int CarClass

int CarPerformanceIndex

int DrivetrainType

int NumCylinders

unsigned int CarGroup

float SmashableVelDiff

float SmashableMass

float Power

float Torque

float TireTempFrontLeft
float TireTempFrontRight
float TireTempRearLeft
float TireTempRearRight

float Boost

float Fuel

float DistanceTraveled

float BestLap
float LastLap
float CurrentLap

float CurrentRaceTime

unsigned short LapNumber

unsigned char RacePosition

unsigned char Accel
unsigned char Brake
unsigned char Clutch
unsigned char HandBrake

char Steer

char NormalizedDrivingLine

char NormalizedAIBrakeDifference
```

## Used resources

- ```unsigned int TimestampMS```

### Engine RPM

- ```float EngineMaxRpm```
- ```float EngineIdleRpm```
- ```float CurrentEngineRpm```
- ```float Speed```
- ```unsigned char Gear```

### G-Force

- ```float AccelerationX```
- (```float AccelerationY```)
- ```float AccelerationZ```

### Map

- ```float PositionX```
- (```float PositionY```)
- ```float PositionZ```

### Car Info

- ```int CarClass```
- ```int CarPerformanceIndex```
- ```int DrivetrainType```