//
// Created by peter on 23.05.2020.
//

#pragma once

#include "Vector.h"
#include <common_headers.h>
#include <cstdint>
constexpr auto MAX_MODEL_NAME = 21;
enum ModeInfoType : uint8_t
{
    MITYPE_NA        = 0,
    MITYPE_SIMPLE    = 1,
    MITYPE_MLO       = 2,
    MITYPE_TIME      = 3,
    MITYPE_WEAPON    = 4,
    MITYPE_CLUMP     = 5,
    MITYPE_VEHICLE   = 6,
    MITYPE_PED       = 7,
    MITYPE_XTRACOMPS = 8,
};

class BaseModelInfo
{
  public:
    void *       mVtable;
    char         mName[MAX_MODEL_NAME]{};
    ModeInfoType mType;
    uint8_t      mNum2dEffects;
    bool         mFreeCol;
    void *       mColModel;
    int16_t      m2dFxId;
    int16_t      mObjectId;
    uint16_t     mRefCount;
    int16_t      mTxdSlot;
};

static_assert( sizeof( BaseModelInfo ) == 0x28, "BaseModelInfo: error" );

class SimpleModelInfo : public BaseModelInfo
{
  public:
    // atomics[2] is often a pointer to the non-LOD modelinfo
    RpAtomic *mAtomics[3];
    // m_lodDistances[2] holds the near distance for LODs
    float    mLodDistances[3];
    uint8_t  mNumAtomics;
    uint8_t  mAlpha;
    uint16_t mFirstDamaged : 2; // 0: no damage model
    // 1: 1 and 2 are damage models
    // 2: 2 is damage model
    uint16_t         mNormalCull : 1;
    uint16_t         mIsDamaged : 1;
    uint16_t         mIsBigBuilding : 1;
    uint16_t         mNoFade : 1;
    uint16_t         mDrawLast : 1;
    uint16_t         mAdditive : 1;
    uint16_t         mIsSubway : 1;
    uint16_t         mIgnoreLight : 1;
    uint16_t         mNoZWrite : 1;
    RpAtomic *       GetAtomicFromDistance( float d ) const;
    RpAtomic *       GetFirstAtomicFromDistance( float d ) const;
    SimpleModelInfo *GetRelatedModel( void )
    {
        return (SimpleModelInfo *)mAtomics[2];
    }
    float GetNearDistance() { return mLodDistances[2] * 4.0f; }
    void  IncreaseAlpha()
    {
        if ( mAlpha >= 0xEF )
            mAlpha = 0xFF;
        else
            mAlpha += 0x10;
    }
};

static_assert( sizeof( SimpleModelInfo ) == 0x44, "SimpleModelInfo: error" );

class ModelInfo
{
  public:
    static BaseModelInfo *GetModelInfo( int id ) { return mModelInfoPtrs[id]; }

  private:
    static BaseModelInfo **mModelInfoPtrs;
};
class TimeModelInfo : public SimpleModelInfo
{
  public:
    int32_t          mTimeOn;
    int32_t          mTimeOff;
    SimpleModelInfo *mOtherTimeModel;
};
static_assert( sizeof( TimeModelInfo ) == 0x50, "TimeModelInfo: error" );

class ClumpModelInfo : public BaseModelInfo
{
  public:
    RpClump *mClump;
    union
    {
        char *m_pszAnimFileName;
        int   m_nAnimFileIndex;
    };
};

static_assert( sizeof( ClumpModelInfo ) == 0x30, "ClumpModelInfo: error" );

struct CarGizmoPosIds
{
    static constexpr size_t HeadLights = 0;
    static constexpr size_t TailLights = 1;
    static constexpr size_t FrontSeat  = 2;
    static constexpr size_t BackSeat   = 3;
    static constexpr size_t Exhaust    = 4;
};

class VehicleModelInfo : public ClumpModelInfo
{
    static constexpr size_t NUM_VEHICLE_POSITIONS = 5;

    static constexpr size_t NUM_FIRST_MATERIALS  = 24;
    static constexpr size_t NUM_SECOND_MATERIALS = 20;
    static constexpr size_t NUM_VEHICLE_COLOURS  = 8;
    static constexpr size_t NUM_VEHICLE_ENVMAPS  = 1;

  public:
    uint8_t     mLastColour1;
    uint8_t     mLastColour2;
    char        mName[10];
    int32_t     mVehicleType;
    float       mWheelScale;
    int16_t     mWheelId;
    int16_t     mHandlingId;
    uint8_t     mNumDoors;
    uint8_t     mVehicleClass;
    uint8_t     mLevel;
    uint8_t     mNumExtras;
    uint16_t    mFrq;
    uint16_t    _padding;
    Vector      mPositions[NUM_VEHICLE_POSITIONS];
    uint32_t    mCompRules;
    float       mBikeSteerAngle;
    RpMaterial *mMaterials1[NUM_FIRST_MATERIALS];
    RpMaterial *mMaterials2[NUM_SECOND_MATERIALS];
    uint8_t     mColours1[NUM_VEHICLE_COLOURS];
    uint8_t     mColours2[NUM_VEHICLE_COLOURS];
    uint8_t     mNumColours;
    uint8_t     mLastColorVariation;
    uint8_t     mCurrentColour1;
    uint8_t     mCurrentColour2;
    RpAtomic *  mComps[6];
    int32_t     m_numComps;
};

static_assert( sizeof( VehicleModelInfo ) == 0x174, "VehicleModelInfo: error" );
