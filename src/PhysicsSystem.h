#pragma once

#include "PxPhysicsAPI.h"
#include "EventSystem.h"
#include "PhysicsComponent.h"

class AssetManager;

#define MAX_NUM_4W_VEHICLES 10




enum
{
    COLLISION_FLAG_GROUND = 1 << 0,
    COLLISION_FLAG_WHEEL = 1 << 1,
    COLLISION_FLAG_CHASSIS = 1 << 2,
    COLLISION_FLAG_OBSTACLE = 1 << 3,
    COLLISION_FLAG_DRIVABLE_OBSTACLE = 1 << 4,

    COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_WHEEL_AGAINST = COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
    COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
    COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

//Id of drivable surface (used by suspension raycast filtering).
enum
{
    DRIVABLE_SURFACE_ID = 0xffffffff
};

//Drivable surface types.
enum
{
    SURFACE_TYPE_MUD = 0,
    SURFACE_TYPE_TARMAC,
    SURFACE_TYPE_SNOW,
    SURFACE_TYPE_GRASS,
    MAX_NUM_SURFACE_TYPES
};

//Tire types.
enum
{
    TIRE_TYPE_WETS = 0,
    TIRE_TYPE_SLICKS,
    TIRE_TYPE_ICE,
    TIRE_TYPE_MUD,
    MAX_NUM_TIRE_TYPES
};

enum
{
    SAMPLEVEHICLE_DRIVABLE_SURFACE = 0xffff0000,
    SAMPLEVEHICLE_UNDRIVABLE_SURFACE = 0x0000ffff
};


using namespace physx;
class PhysicsSystem : public EventSystem<PhysicsSystem> {
public:
    PhysicsSystem(AssetManager&);
    ~PhysicsSystem();

    void update();

private:
    void handle_add_example_ship(const Event& e);
    void handle_add_terrain(const Event& e);
    void handle_key_press(const Event& e);

    void create4WVehicle
    (PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
        const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
        const PxTransform& startTransform, const bool useAutoGearFlag);

    physx::PxDefaultAllocator gAllocator_;
    physx::PxDefaultErrorCallback gErrorCallback_;
    physx::PxTolerancesScale gScale_;
    physx::PxFoundation* gFoundation_;
    physx::PxPhysics* gPhysics_;
    physx::PxCooking* gCooking_;
    physx::PxScene* gScene_;

    AssetManager& asset_manager_;

    std::vector<PhysicsComponent<false>> dynamic_objects_;
    std::vector<PhysicsComponent<true>> static_objects_;

    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* frictionPairs;
    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const physx::PxMaterial* defaultMaterial);

    float forwardDrive, horizontalDrive;




    //Array of all cars and report data for each car.
    PxVehicleWheels* mVehicles[MAX_NUM_4W_VEHICLES];
    PxVehicleWheelQueryResult mVehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
    PxU32 mNumVehicles;

    //sdk raycasts (for the suspension lines).

    class SampleVehicleSceneQueryData
    {
    public:

        //Allocate scene query data for up to maxNumWheels suspension raycasts.
        static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

        //Free allocated buffer for scene queries of suspension raycasts.
        void free();

        //Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
        PxBatchQuery* setUpBatchedSceneQuery(PxScene* scene);

        //Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
        PxRaycastQueryResult* getRaycastQueryResultBuffer() { return mSqResults; }

        //Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
        PxU32 getRaycastQueryResultBufferSize() const { return mNumQueries; }

        //Set the pre-filter shader 
        void setPreFilterShader(PxBatchQueryPreFilterShader preFilterShader) { mPreFilterShader = preFilterShader; }

    private:

        //One result for each wheel.
        PxRaycastQueryResult* mSqResults;
        PxU32 mNbSqResults;

        //One hit for each wheel.
        PxRaycastHit* mSqHitBuffer;

        //Filter shader used to filter drivable and non-drivable surfaces
        PxBatchQueryPreFilterShader mPreFilterShader;

        //Maximum number of suspension raycasts that can be supported by the allocated buffers 
        //assuming a single query and hit per suspension line.
        PxU32 mNumQueries;

        void init()
        {
            mPreFilterShader = [](
                PxFilterData filterData0,
                PxFilterData filterData1,
                const void* constantBlock, PxU32 constantBlockSize,
                PxHitFlags& queryFlags)->PxQueryHitType::Enum
            {
                //filterData0 is the vehicle suspension raycast.
                //filterData1 is the shape potentially hit by the raycast.
                PX_UNUSED(queryFlags);
                PX_UNUSED(constantBlockSize);
                PX_UNUSED(constantBlock);
                PX_UNUSED(filterData0);
                return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);
            };
        }

        SampleVehicleSceneQueryData()
        {
            init();
        }

        ~SampleVehicleSceneQueryData()
        {
        }
    };
    SampleVehicleSceneQueryData* mSqData;











    PxBatchQuery* mSqWheelRaycastBatchQuery;

    //Reports for each wheel.
    class SampleVehicleWheelQueryResults
    {
    public:

        //Allocate a buffer of wheel query results for up to maxNumWheels.
        static SampleVehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

        //Free allocated buffer.
        void free();

        PxWheelQueryResult* addVehicle(const PxU32 numWheels);

    private:

        //One result for each wheel.
        PxWheelQueryResult* mWheelQueryResults;

        //Maximum number of wheels.
        PxU32 mMaxNumWheels;

        //Number of wheels 
        PxU32 mNumWheels;


        SampleVehicleWheelQueryResults()
            : mWheelQueryResults(NULL), mMaxNumWheels(0), mNumWheels(0)
        {
            init();
        }

        ~SampleVehicleWheelQueryResults()
        {
        }

        void init()
        {
            mWheelQueryResults = NULL;
            mMaxNumWheels = 0;
            mNumWheels = 0;
        }
    };


    SampleVehicleWheelQueryResults* mWheelQueryResults;

    //Cached simulation data of focus vehicle in 4W mode.
    PxVehicleWheelsSimData* mWheelsSimData4W;
    PxVehicleDriveSimData4W mDriveSimData4W;
    bool mIsIn3WMode;

    //Friction from combinations of tire and surface types.
    PxVehicleDrivableSurfaceToTireFrictionPairs* mSurfaceTirePairs;


    //Initialise a car back to its start transform and state.
    //void resetNWCar(const PxTransform& startTransform, PxVehicleWheels* car);

};