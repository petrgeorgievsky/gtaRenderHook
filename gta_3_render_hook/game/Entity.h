//
// Created by peter on 23.05.2020.
//

#pragma once
#include "Vector.h"
#include <common_headers.h>
#include <cstdint>
class Matrix
{
  public:
    RwMatrix  m_matrix;
    RwMatrix *m_attachment;
    bool      m_hasRwMatrix; // are we the owner?
};

class Placeable
{
  public:
    void * mVtable;
    Matrix mMatrix;

    Vector GetRight( void ) { return Vector{ mMatrix.m_matrix.right }; }
    Vector GetForward( void ) { return Vector{ mMatrix.m_matrix.up }; }
    Vector GetUp( void ) { return Vector{ mMatrix.m_matrix.at }; }
};
static_assert( sizeof( Placeable ) == 0x4C, "CPlaceable: error" );

class Entity : public Placeable
{
  public:
    RwObject *mRwObject;
    uint32_t  mType : 3;
    uint32_t  mStatus : 5;

    // flagsA
    uint32_t bUsesCollision : 1;      // does entity use collision
    uint32_t bCollisionProcessed : 1; // has object been processed by a
                                      // ProcessEntityCollision function
    uint32_t bIsStatic : 1;           // is entity static
    uint32_t bHasContacted : 1; // has entity processed some contact forces
    uint32_t bPedPhysics : 1;
    uint32_t bIsStuck : 1; // is entity stuck
    uint32_t
        bIsInSafePosition : 1; // is entity in a collision free safe position
    uint32_t bUseCollisionRecords : 1;

    // flagsB
    uint32_t bWasPostponed : 1; // was entity control processing postponed
    uint32_t bExplosionProof : 1;
    uint32_t bIsVisible : 1; // is the entity visible
    uint32_t bHasCollided : 1;
    uint32_t bRenderScorched : 1;
    uint32_t bHasBlip : 1;
    uint32_t bIsBIGBuilding : 1; // Set if this entity is a big building
    // VC inserts one more flag here: if drawdist <= 2000
    uint32_t bRenderDamaged : 1; // use damaged LOD models for objects with
                                 // applicable damage

    // flagsC
    uint32_t bBulletProof : 1;
    uint32_t bFireProof : 1;
    uint32_t bCollisionProof : 1;
    uint32_t bMeleeProof : 1;
    uint32_t bOnlyDamagedByPlayer : 1;
    uint32_t bStreamingDontDelete : 1; // Dont let the streaming remove this
    uint32_t bZoneCulled : 1;
    uint32_t bZoneCulled2 : 1; // only treadables+10m

    // flagsD
    uint32_t bRemoveFromWorld : 1; // remove this entity next time it should be
                                   // processed
    uint32_t bHasHitWall : 1;      // has collided with a building (changes
                                   // subsequent collisions)
    uint32_t bImBeingRendered : 1; // don't delete me because I'm being rendered
    uint32_t bTouchingWater : 1;   // used by cBuoyancy::ProcessBuoyancy
    uint32_t bIsSubway : 1; // set when subway, but maybe different meaning?
    uint32_t bDrawLast : 1; // draw object last
    uint32_t bNoBrightHeadLights : 1;
    uint32_t bDoNotRender : 1;

    // flagsE
    uint32_t bDistanceFade : 1; // Fade entity because it is far away
    uint32_t mFlagE2 : 1;

    uint16_t mScanCode;
    uint16_t mRandomSeed;
    int16_t  mModelIndex;
    uint16_t mLevel; // int16
    void *   mFirstReference;
    void     CreateRwObject();
    void     PreRender();
    void     Render();
};
static_assert( sizeof( Entity ) == 0x64, "CEntity: error" );

class Physical : public Entity
{
    static constexpr size_t PHYSICAL_MAX_COLLISIONRECORDS = 6;

  public:
    // The not properly indented fields haven't been checked properly yet

    int32_t  mAudioEntityId;
    float    _padding_00;
    void *   mTreadable[2]; // car and ped
    uint32_t mLastTimeCollided;
    Vector   mMoveSpeed; // velocity
    Vector   mTurnSpeed; // angular velocity
    Vector   mMoveFriction;
    Vector   mTurnFriction;
    Vector   mMoveSpeedAvg;
    Vector   mTurnSpeedAvg;
    float    mMass;
    float    mTurnMass; // moment of inertia
    float    mForceMultiplier;
    float    mAirResistance;
    float    mElasticity;
    float    mBuoyancy;
    Vector   mCentreOfMass;
    void *   mEntryInfoList;
    void *   mMovingListNode;

    int8_t  _padding_01;
    uint8_t mStaticFrames;
    uint8_t mCollisionRecordCount;
    bool    mIsVehicleBeingShifted;
    Entity *mCollisionRecords[PHYSICAL_MAX_COLLISIONRECORDS];

    float m_fDistanceTravelled;

    // damaged piece
    float   mDamageImpulse;
    Entity *mDamageEntity;
    Vector  mDamageNormal;
    int16_t mDamagePieceType;

    uint8_t IsHeavy : 1;
    uint8_t AffectedByGravity : 1;
    uint8_t InfiniteMass : 1;
    uint8_t IsInWater : 1;
    uint8_t _padding_02 : 1; // unused
    uint8_t _padding_03 : 1; // unused
    uint8_t HitByTrain : 1;
    uint8_t SkipLineCol : 1;

    uint8_t m_nSurfaceTouched;
    int8_t  m_nZoneLevel;
};

static_assert( sizeof( Physical ) == 0x128, "Physical: error" );

class Vehicle;
class AutoPilot
{
    static constexpr size_t NUM_PATH_NODES_IN_AUTOPILOT = 8;

  public:
    int32_t  mCurrentRouteNode;
    int32_t  mNextRouteNode;
    int32_t  mPrevRouteNode;
    int32_t  mTimeEnteredCurve;
    int32_t  mTimeToSpendOnCurrentCurve;
    uint32_t mCurrentPathNodeInfo;
    uint32_t mNextPathNodeInfo;
    uint32_t mPreviousPathNodeInfo;
    uint32_t mAntiReverseTimer;
    uint32_t mTimeToStartMission;
    int8_t   mPreviousDirection;
    int8_t   mCurrentDirection;
    int8_t   mNextDirection;
    int8_t   mCurrentLane;
    int8_t   mNextLane;
    uint8_t  mDrivingStyle;
    uint8_t  mCarMission;
    uint8_t  mTempAction;
    uint32_t mTimeTempAction;
    float    mMaxTrafficSpeed;
    uint8_t  mCruiseSpeed;
    uint8_t  mSlowedDownBecauseOfCars : 1;
    uint8_t  mSlowedDownBecauseOfPeds : 1;
    uint8_t  mStayInCurrentLevel : 1;
    uint8_t  mStayInFastLane : 1;
    uint8_t  mIgnorePathfinding : 1;
    Vector   mDestinationCoors;
    void *   mPathFindNodesInfo[NUM_PATH_NODES_IN_AUTOPILOT];
    int16_t  mPathFindNodesCount;
    Vehicle *mTargetCar;
};

static_assert( sizeof( AutoPilot ) == 0x70, "AutoPilot: error" );

struct StoredCollPoly
{
    Vector verts[3];
    bool   valid;
};

class Vehicle : public Physical
{
  public:
    // 0x128
    void *    pHandling;
    AutoPilot AutoPilot;
    uint8_t   mCurrentColour1;
    uint8_t   mCurrentColour2;
    int8_t    mExtras[2];
    int16_t   mAlarmState;
    int16_t   mMissionValue;
    Entity *  mDriver;
    Entity *  mPassengers[8];
    uint8_t   mNumPassengers;
    int8_t    mNumGettingIn;
    int8_t    mGettingInFlags;
    int8_t    mGettingOutFlags;
    uint8_t   mNumMaxPassengers;
    float     field_1D0[4];
    Entity *  mCurrentGroundEntity;
    void *    m_pCarFire;
    float     mSteerAngle;
    float     mGasPedal;
    float     mBrakePedal;
    uint8_t   VehicleCreatedBy;

    // cf.
    // https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CVehicle.h
    // from R*
    uint8_t IsLawEnforcer : 1; // Is this guy chasing the player at the moment
    uint8_t IsAmbulanceOnDuty : 1; // Ambulance trying to get to an accident
    uint8_t IsFireTruckOnDuty : 1; // Firetruck trying to get to a fire
    uint8_t
        IsLocked : 1; // Is this guy locked by the script (cannot be removed)
    uint8_t EngineOn : 1; // For sound purposes. Parked cars have their engines
    // switched off (so do destroyed cars)
    uint8_t IsHandbrakeOn : 1; // How's the handbrake doing ?
    uint8_t LightsOn : 1;      // Are the lights switched on ?
    uint8_t Freebies : 1;      // Any freebies left in this vehicle ?

    uint8_t IsVan : 1;      // Is this vehicle a van (doors at back of vehicle)
    uint8_t IsBus : 1;      // Is this vehicle a bus
    uint8_t IsBig : 1;      // Is this vehicle a bus
    uint8_t LowVehicle : 1; // Need this for sporty type cars to use low
    // getting-in/out anims
    uint8_t ComedyControls : 1; // Will make the car hard to control (hopefully
    // in a funny way)
    uint8_t WarnedPeds : 1; // Has scan and warn peds of danger been processed?
    uint8_t CraneMessageDone : 1; // A crane message has been printed for this
    // car allready
    uint8_t ExtendedRange : 1; // This vehicle needs to be a bit further away to
    // get deleted

    uint8_t TakeLessDamage : 1; // This vehicle is stronger (takes about 1/4 of
    // damage)
    uint8_t IsDamaged : 1; // This vehicle has been damaged and is displaying
    // all its components
    uint8_t
        HasBeenOwnedByPlayer : 1; // To work out whether stealing it is a crime
    uint8_t FadeOut : 1;          // Fade vehicle out
    uint8_t IsBeingCarJacked : 1; // Fade vehicle out
    uint8_t CreateRoadBlockPeds : 1; // If this vehicle gets close enough we
    // will create peds (coppers or gang
    // members) round it
    uint8_t
        CanBeDamaged : 1; // Set to FALSE during cut scenes to avoid explosions
    uint8_t
        UsingSpecialColModel : 1; // Is player vehicle using special collision
    // model, stored in player strucure

    uint8_t OccupantsHaveBeenGenerated : 1; // Is true if the occupants have
    // already been generated.
    // (Shouldn't happen again)
    uint8_t GunSwitchedOff : 1; // Level designers can use this to switch off
    // guns on boats
    uint8_t VehicleColProcessed : 1; // Has ProcessEntityCollision been
    // processed for this car?
    uint8_t IsCarParkVehicle : 1; // Car has been created using the special
    // CAR_PARK script command
    uint8_t HasAlreadyBeenRecorded : 1; // Used for replays

    int8_t  mNumPedsUseItAsCover;
    uint8_t mAmmoInClip; // Used to make the guns on boat do a reload (20 by
    // default)
    int8_t   mPacManPickupsCarried;
    uint8_t  mRoadblockType;
    int16_t  mRoadblockNode;
    float    mHealth; // 1000.0f = full health. 250.0f = fire. 0 -> explode
    uint8_t  mCurrentGear;
    float    mChangeGearTime;
    uint32_t mGunFiringTime; // last time when gun on vehicle was fired (used on
    // boats)
    uint32_t mTimeOfDeath;
    uint16_t mTimeBlocked;
    int16_t  mBombTimer; // goes down with each frame
    Entity * mBlowUpEntity;
    float    mMapObjectHeightAhead;  // front Z?
    float    mMapObjectHeightBehind; // rear Z?
    uint32_t mDoorLock;
    int8_t   mLastWeaponDamage; // see eWeaponType, -1 if no damage
    uint8_t  mRadioStation;
    uint8_t  mRainAudioCounter;
    uint8_t  mRainSamplesCounter;
    uint8_t  mCarHornTimer;
    uint8_t  mCarHornPattern; // last horn?
    bool     mSirenOrAlarm;
    int8_t   mComedyControlState;
    StoredCollPoly
             m_aCollPolys[2]; // poly which is under front/rear part of car
    float    m_fSteerInput;
    uint32_t m_vehType;
};

static_assert( sizeof( Vehicle ) == 0x288, "Vehicle: error" );

enum eLights
{
    VEHLIGHT_FRONT_LEFT,
    VEHLIGHT_FRONT_RIGHT,
    VEHLIGHT_REAR_LEFT,
    VEHLIGHT_REAR_RIGHT,
};

inline uint32_t ldb( uint32_t p, uint32_t s, uint32_t w )
{
    return w >> p & ( 1 << s ) - 1;
}
class DamageManager
{
  public:
    float    mWheelDamageEffect;
    uint8_t  mEngineStatus;
    uint8_t  mWheelStatus[4];
    uint8_t  mDoorStatus[6];
    uint32_t mLightStatus;
    uint32_t mPanelStatus;
    uint8_t  field_18;

    int32_t GetLightStatus( uint32_t light )
    {
        return ldb( light * 2, 2, mLightStatus );
    }
};
static_assert( sizeof( DamageManager ) == 0x1C, "DamageManager: error" );

class Door
{
  public:
    float mMaxAngle;
    float mMinAngle;
    // direction of rotation for air resistance
    int8_t mDirn;
    // axis in which this door rotates
    int8_t mAxis;
    int8_t mDoorState;
    float  mAngle;
    float  mPrevAngle;
    float  mAngVel;
    Vector mSpeed;
};

struct ColPoint
{
    Vector point;
    int    pad1;
    // the surface normal on the surface of point
    Vector  normal;
    int     pad2;
    uint8_t surfaceA;
    uint8_t pieceA;
    uint8_t surfaceB;
    uint8_t pieceB;
    float   depth;
};

class Automobile : public Vehicle
{
    static constexpr size_t NUM_CAR_NODES = 20;

  public:
    // 0x288
    DamageManager Damage;
    Door          Doors[6];
    RwFrame *     mCarNodes[NUM_CAR_NODES];
    ColPoint      mWheelColPoints[4];
    float         mSuspensionSpringRatio[4];
    float         mSuspensionSpringRatioPrev[4];
    float mWheelTimer[4]; // set to 4.0 when wheel is touching ground, then
    // decremented
    float     m_auto_unused1;
    bool      mWheelSkidmarkMuddy[4];
    bool      mWheelSkidmarkBloody[4];
    float     mWheelRotation[4];
    float     mWheelPosition[4];
    float     mWheelSpeed[4];
    uint8_t   m_auto_unused2;
    uint8_t   m_bombType : 3;
    uint8_t   bTaxiLight : 1;
    uint8_t   bDriverLastFrame : 1; // for bombs
    uint8_t   bFixedColour : 1;
    uint8_t   bBigWheels : 1;
    uint8_t   bWaterTight : 1; // no damage for non-player peds
    uint8_t   bNotDamagedUpsideDown : 1;
    uint8_t   bMoreResistantToDamage : 1;
    Entity *  mBombRigger;
    int16_t   m_auto_unk1;
    uint16_t  m_hydraulicState;
    uint32_t  mBusDoorTimerEnd;
    uint32_t  mBusDoorTimerStart;
    float     mSuspensionSpringLength[4];
    float     mSuspensionLineLength[4];
    float     mHeightAboveRoad;
    float     mTraction;
    float     mVelocityChangeForAudio;
    float     mRandomValues[6]; // used for what?
    float     mFireBlowUpTimer;
    Physical *mGroundPhysical[4]; // physicals touching wheels
    Vector    mGroundOffset[4];   // from ground object to colpoint
    Entity *  mSetOnFireEntity;
    float     mWeaponDoorTimerLeft; // still don't know what exactly this is
    float     mWeaponDoorTimerRight;
    float     mCarGunLR;
    float     mCarGunUD;
    float     mPropellerRotation;
    uint8_t   stuff4[4];
    uint8_t   mWheelsOnGround;
    uint8_t   mDriveWheelsOnGround;
    uint8_t   mDriveWheelsOnGroundPrev;
    float     mGasPedalAudio;
    uint32_t  mWheelState[4];
};

static_assert( sizeof( Automobile ) == 0x5A8, "Automobile: error" );
