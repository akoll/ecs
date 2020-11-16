#include <iostream>
#include <cstdlib>

#include "ecs/scaffold/ecs.hpp"

#include "../util/frame_limit_system.hpp"
#if defined BENCHMARK_FRAMETIME
#include "../util/frame_time_system.hpp"
#endif
#if defined BENCHMARK_MEMORY
#include "../util/memory_system.hpp"
#endif

#if defined SCHEDULER_SEQUENTIAL
#include "ecs/scheduler/sequential.hpp"
#elif defined SCHEDULER_PARALLEL
#include "ecs/scheduler/parallel.hpp"
#else
static_assert("No scheduler option set.");
#endif

#if defined STORAGE_TOV
#include "ecs/storage/tuple_of_vectors.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::TupleOfVectors,
  #if defined SCHEDULER_SEQUENTIAL
  ecs::scheduler::Sequential
  #elif defined SCHEDULER_PARALLEL
  ecs::scheduler::Parallel
  #endif
>;
#elif defined STORAGE_VOT
#include "ecs/storage/vector_of_tuples.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::VectorOfTuples,
  #if defined SCHEDULER_SEQUENTIAL
  ecs::scheduler::Sequential
  #elif defined SCHEDULER_PARALLEL
  ecs::scheduler::Parallel
  #endif
>;
#elif defined STORAGE_SCATTERED
#include "ecs/storage/scattered.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::ScatteredCustom
    #ifdef STORAGE_SCATTERED_SMART
    ::WithSmartPointers
    #endif
    #ifdef STORAGE_SCATTERED_SET
      ::WithEntitySet
    #endif
    ::Storage
  ,
  #if defined SCHEDULER_SEQUENTIAL
  ecs::scheduler::Sequential
  #elif defined SCHEDULER_PARALLEL
  ecs::scheduler::Parallel
  #endif
>;
#elif defined STORAGE_ENTT
#include "ecs/storage/entt.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::Entt,
  #ifdef OUTER_PARALLEL
  ecs::scheduler::Parallel
  #else
  ecs::scheduler::Sequential
  #endif
>;
#else
static_assert(false, "No storage strategy set.");
#endif

#ifndef FRAME_COUNT
static_assert(false, "No frame count set.");
#endif

namespace benchmark {

template<typename... TSystems>
class Scene {
public:
  using BaseScene = ECS::Scene<
    TSystems...,
    #if defined BENCHMARK_FRAMETIME
    benchmark::FrameTimeSystem<FRAME_COUNT>,
    #endif
    #if defined BENCHMARK_MEMORY
    benchmark::MemoryUsageSystem<FRAME_COUNT>,
    #endif
    benchmark::FrameLimitSystem<FRAME_COUNT>
  >;

  Scene(TSystems&&... systems) :
    _scene(
      std::forward<TSystems>(systems)...,
      #if defined BENCHMARK_FRAMETIME
      benchmark::FrameTimeSystem<FRAME_COUNT>(),
      #endif
      #if defined BENCHMARK_MEMORY
      benchmark::MemoryUsageSystem<FRAME_COUNT>(),
      #endif
      benchmark::FrameLimitSystem<FRAME_COUNT>([this]() { _running = false; })
    )
  {}

  void run() {
    // while (_running) _scene.update();
    for (auto i{0u}; i < FRAME_COUNT + 1; ++i)
      _scene.update();
  }

  bool update() {
    _scene.update();
    return _running;
  }

  operator BaseScene&() {
    return _scene;
  }

  BaseScene* operator->() {
    return &_scene;
  }

private:
  BaseScene _scene;
  bool _running = true;
};

template<typename... TSystems>
Scene(TSystems&&...) -> Scene<TSystems...>;

}
