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
static_assert( sizeof( Matrix ) == 0x48, "Matrix: error" );

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
    uint32_t bIsBIGBuilding : 1;     // Set if this entity is a big building
    uint32_t bStreamBIGBuilding : 1; // Set if this entity is a big building

    // VC inserts one more flag here: if drawdist <= 2000
    uint32_t bRenderDamaged : 1; // use damaged LOD models for objects with
                                 // applicable damage
    uint32_t bBulletProof : 1;
    uint32_t bFireProof : 1;
    uint32_t bCollisionProof : 1;
    uint32_t bMeleeProof : 1;
    uint32_t bOnlyDamagedByPlayer : 1;
    uint32_t bStreamingDontDelete : 1; // Dont let the streaming remove this
    uint32_t bRemoveFromWorld : 1; // remove this entity next time it should be
                                   // processed
    uint32_t bHasHitWall : 1;      // has collided with a building (changes
                                   // subsequent collisions)

    uint32_t bImBeingRendered : 1; // don't delete me because I'm being rendered

    uint32_t bTouchingWater : 1; // used by cBuoyancy::ProcessBuoyancy

    uint32_t bIsSubway : 1; // set when subway, but maybe different meaning?

    uint32_t bDrawLast : 1; // draw object last
    uint32_t bNoBrightHeadLights : 1;
    uint32_t bDoNotRender : 1;
    uint32_t bDistanceFade : 1; // Fade entity because it is far away

    uint32_t mFlagE1 : 1;
    uint32_t mFlagE2 : 1;
    uint32_t bOffscreen : 1; // offscreen flag. This can only be trusted when it
                             // is set to true
    uint32_t
        bIsStaticWaitingForCollision : 1; // this is used by script created
                                          // entities - they are static until
                                          // the collision is loaded below them
    uint32_t bDontStream : 1;             // tell the streaming not to stream me
    uint32_t bUnderwater : 1; // this object is underwater change drawing order
    uint32_t bHasPreRenderEffects : 1; // Object has a prerender effects
                                       // attached to it

    uint16_t mScanCode;
    uint16_t mRandomSeed;
    int16_t  mModelIndex;
    uint8_t  mLevel; // int16
    uint8_t  mArea;  // int16
    void *   mFirstReference;
    void     CreateRwObject();
    void     PreRender();
    void     Render();
};
static_assert( sizeof( Entity ) == 0x64, "Entity: error" );

class Physical : public Entity
{
    static constexpr size_t PHYSICAL_MAX_COLLISIONRECORDS = 6;

  public:
    // The not properly indented fields haven't been checked properly yet

    int32_t mAudioEntityId;
    float   _padding_00;
    float   _padding_01;
    Vector  mMoveSpeed; // velocity
    Vector  mTurnSpeed; // angular velocity
    Vector  mMoveFriction;
    Vector  mTurnFriction;
    Vector  mMoveSpeedAvg;
    Vector  mTurnSpeedAvg;
    float   mMass;
    float   mTurnMass; // moment of inertia
    float   mForceMultiplier;
    float   mAirResistance;
    float   mElasticity;
    float   mBuoyancy;
    Vector  mCentreOfMass;
    void *  mEntryInfoList;
    void *  mMovingListNode;

    uint8_t mCollideExtra;
    uint8_t mCollideInfo;
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
    uint8_t _padding_02 : 1; // unused
    uint8_t IsInWater : 1;
    uint8_t _padding_03 : 1; // unused
    uint8_t HitByTrain : 1;
    uint8_t SkipLineCol : 1;

    uint8_t bIsFrozen : 1;
    uint8_t bDontLoadCollision : 1;
    uint8_t m_bIsVehicleBeingShifted : 1; // wrong name - also used on but never
                                          // set for peds
    uint8_t bJustCheckCollision : 1;      // just see if there is a collision

    uint8_t m_nSurfaceTouched;
    int8_t  m_nZoneLevel;
};

static_assert( sizeof( Physical ) == 0x120, "Physical: error" );

class Vehicle;
class AutoPilot
{
    static constexpr size_t NUM_PATH_NODES_IN_AUTOPILOT = 8;

  public:
    uint32_t      m_currentAddress;
    uint32_t      m_startingRouteNode;
    uint32_t      m_PreviousRouteNode;
    unsigned int  m_nTotalSpeedScaleFactor;
    unsigned int  m_nSpeedScaleFactor;
    unsigned int  m_nCurrentPathNodeInfo;
    unsigned int  m_nNextPathNodeInfo;
    unsigned int  m_nPreviousPathNodeInfo;
    unsigned int  m_nTimeToStartMission;
    unsigned int  m_nTimeSwitchedToRealPhysics;
    char          m_nPreviousDirection;
    char          m_nCurrentDirecton;
    char          m_nNextDirection;
    char          m_nCurrentLane;
    char          m_nNextLane;
    char          m_nDrivingStyle;
    char          m_nCarMission;
    char          m_nAnimationId;
    unsigned int  m_nAnimationTime;
    float         m_fMaxTrafficSpeed;
    unsigned char m_nCruiseSpeed;

    char     unknown[11];
    Vector   m_vecDestinationCoors;
    uint32_t m_aPathFindNodesInfo[8];
    short    m_nPathFindNodesCount;

  private:
    char _pad[2];
};

static_assert( sizeof( AutoPilot ) == 0x74, "AutoPilot: error" );

struct StoredCollPoly
{
    Vector verts[3];
    bool   valid;
};

class Vehicle : public Physical
{
  public:
    // 0x128
    void *        pHandling;
    void *        pFlyingHandling;
    AutoPilot     AutoPilot;
    Vehicle *     m_pVehicleToRam;
    uint8_t       mCurrentColour1;
    uint8_t       mCurrentColour2;
    int8_t        mExtras[2];
    int16_t       mAlarmState;
    int16_t       mRouteSeed;
    Entity *      mDriver;
    Entity *      mPassengers[8];
    unsigned char m_nNumPassengers;
    unsigned char m_nNumGettingIn;
    unsigned char m_nGettingInFlags;
    unsigned char m_nGettingOutFlags;
    unsigned char m_nMaxPassengers;
    char          __f01CD[3];
    int           field_1D4;
    Vector        field_1D8;
    Entity *      m_pEntityWeAreOn;
    class CFire * m_pFire;
    float         m_fSteerAngle;
    float         m_fGasPedal;
    float         m_fBreakPedal;
    unsigned char m_nCreatedBy; // see eVehicleCreatedBy
    // cf.
    // https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CVehicle.h
    // from R*
    uint8_t IsLawEnforcer : 1;
    uint8_t IsAmbulanceOnDuty : 1;
    uint8_t IsFireTruckOnDuty : 1;
    uint8_t IsLocked : 1;

    uint8_t EngineOn : 1;
    uint8_t IsHandbrakeOn : 1;
    uint8_t LightsOn : 1;
    uint8_t Freebies : 1;

    uint8_t IsVan : 1;
    uint8_t IsBus : 1;
    uint8_t IsBig : 1;
    uint8_t LowVehicle : 1;

    uint8_t ComedyControls : 1;
    uint8_t WarnedPeds : 1;
    uint8_t CraneMessageDone : 1;
    uint8_t ExtendedRange : 1;

    // 16

    uint8_t TakeLessDamage : 1;
    uint8_t IsDamaged : 1;
    uint8_t HasBeenOwnedByPlayer : 1;
    uint8_t FadeOut : 1;

    uint8_t IsBeingCarJacked : 1;
    uint8_t CreateRoadBlockPeds : 1;
    uint8_t CanBeDamaged : 1;
    uint8_t UsingSpecialColModel : 1;

    uint8_t OccupantsHaveBeenGenerated : 1;
    uint8_t GunSwitchedOff : 1;
    uint8_t VehicleColProcessed : 1;
    uint8_t IsCarParkVehicle : 1;

    uint8_t HasAlreadyBeenRecorded : 1;
    uint8_t bPartOfConvoy : 1;
    uint8_t bHeliMinimumTilt : 1;
    uint8_t bAudioChangingGear : 1;

    // 32

    uint8_t bIsDrowning : 1;
    uint8_t bTyresDontBurst : 1;
    uint8_t bCreatedAsPoliceVehicle : 1;
    uint8_t bRestingOnPhysical : 1;

    uint8_t bParking : 1;
    uint8_t bCanPark : 1;
    uint8_t m_bombType : 2;

    uint8_t bUnk0 : 1;
    uint8_t bDriverLastFrame : 1;
    uint8_t bUnk1 : 1;
    uint8_t bUnk2 : 1;

    int8_t mNumPedsUseItAsCover;

    unsigned char  m_nAmmoInClip;
    char           field_201;
    float          m_fHealth;
    unsigned char  m_nCurrentGear;
    char           __f0205[3];
    int            field_20C;
    int            field_210;
    int            m_nTimeTillWeNeedThisCar;
    int            field_218;
    int            m_nTimeOfDeath;
    short          field_220;
    short          m_wBombTimer; // goes down with each frame
    int            field_224;
    int            field_228;
    int            field_22C;
    unsigned int   m_nLockStatus;
    unsigned char  m_nLastWeaponDamage;
    char           __f0231[3];
    Entity *       pLastDamEntity;
    unsigned char  m_nRadioStation;
    char           field_23D;
    char           field_23E;
    unsigned int   m_bHornEnabled;
    char           field_244;
    unsigned char  m_nSirenOrAlarm;
    unsigned char  m_nSirenExtra;
    char           field_247;
    StoredCollPoly m_frontCollPoly; // poly which is under front part of car
    StoredCollPoly m_rearCollPoly;  // poly which is under rear part of car
    float          m_fSteerRatio;
    unsigned int   m_nVehicleClass; // see enum eVehicleType
};

static_assert( sizeof( Vehicle ) == 0x2A0, "Vehicle: error" );

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

    int32_t GetLightStatus( uint32_t light )
    {
        return ldb( light * 2, 2, mLightStatus );
    }
};
static_assert( sizeof( DamageManager ) == 0x18, "DamageManager: error" );

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
  public:
    // 0x288
    DamageManager Damage;
};
