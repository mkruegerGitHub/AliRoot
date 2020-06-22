//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUTPCCompressionKernels.h
/// \author David Rohr

#ifndef GPUTPCCONMPRESSIONKERNELS_H
#define GPUTPCCONMPRESSIONKERNELS_H

#include "GPUGeneralKernels.h"

namespace o2
{
namespace tpc
{
struct ClusterNative;
}
} // namespace o2

namespace GPUCA_NAMESPACE
{
namespace gpu
{
class GPUTPCCompressionKernels : public GPUKernelTemplate
{
 public:
  GPUhdi() CONSTEXPR static GPUDataTypes::RecoStep GetRecoStep() { return GPUDataTypes::RecoStep::TPCCompression; }

  enum K : int {
    step0attached = 0,
    step1unattached = 1,
    step2gather = 2,
  };

#if GPUCA_GET_THREAD_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step1unattached) > GPUCA_GET_THREAD_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step2gather)
#define GPUCA_COMPRESSION_SCAN_MAX_THREADS GPUCA_GET_THREAD_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step1unattached)
#else
#define GPUCA_COMPRESSION_SCAN_MAX_THREADS GPUCA_GET_THREAD_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step2gather)
#endif

  struct GPUSharedMemory : public GPUKernelTemplate::GPUSharedMemoryScan64<int, GPUCA_COMPRESSION_SCAN_MAX_THREADS> {
    union {
      struct {
        GPUAtomic(unsigned int) nCount;
        unsigned int lastIndex;
        unsigned int sortBuffer[GPUCA_TPC_COMP_CHUNK_SIZE];
      } step1;
      struct {
        unsigned int warpOffset[GPUCA_GET_WARP_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step2gather)];
        unsigned int sizes[GPUCA_GET_WARP_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step2gather)][GPUCA_WARP_SIZE];
        unsigned int srcOffsets[GPUCA_GET_WARP_COUNT(GPUCA_LB_GPUTPCCompressionKernels_step2gather)][GPUCA_WARP_SIZE];
      } step2;
    };
  };

  using Vec16 = unsigned short;
  using Vec32 = unsigned int;
  using Vec64 = unsigned long long int;
  using Vec128 = uint4;

  template <typename Scalar, typename BaseVector>
  union CpyVector {
    enum {
      Size = sizeof(BaseVector) / sizeof(Scalar),
    };
    BaseVector all;
    Scalar elems[Size];
  };

  template <int iKernel = defaultKernel>
  GPUd() static void Thread(int nBlocks, int nThreads, int iBlock, int iThread, GPUsharedref() GPUSharedMemory& GPUrestrict() smem, processorType& GPUrestrict() processors);

  template <typename T>
  GPUdi() static bool isAlignedTo(const void* ptr);

  template <typename T>
  GPUdi() static void compressorMemcpy(GPUgeneric() T* dst, GPUgeneric() const T* src, unsigned int size, int nThreads, int iThread);

  template <typename Scalar, typename Vector>
  GPUdi() static void compressorMemcpyVectorised(Scalar* dst, const Scalar* src, unsigned int size, int nThreads, int iThread);

  template <typename T>
  GPUdi() static void compressorMemcpyBasic(T* dst, const T* src, unsigned int size, int nThreads, int iThread, int nBlocks = 1, int iBlock = 0);

  template <typename T>
  GPUdi() static unsigned int calculateWarpOffsets(GPUSharedMemory& smem, T* nums, unsigned int start, unsigned int end, int iWarp, int nLanes, int iLane);

 public:
  template <int I>
  class GPUTPCCompressionKernels_Compare
  {
   public:
    GPUhdi() GPUTPCCompressionKernels_Compare(const o2::tpc::ClusterNative* p) : mClsPtr(p) {}
    GPUd() bool operator()(unsigned int a, unsigned int b) const;

   protected:
    const o2::tpc::ClusterNative* mClsPtr;
  };
};
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif