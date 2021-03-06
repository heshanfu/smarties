//
//  smarties
//  Copyright (c) 2018 CSE-Lab, ETH Zurich, Switzerland. All rights reserved.
//  Distributed under the terms of the MIT license.
//
//  Created by Guido Novati (novatig@ethz.ch).
//

#ifndef smarties_Sampling_h
#define smarties_Sampling_h

#include "../Core/StateAction.h"
#include <random>

namespace smarties
{

class MemoryBuffer;

class Sampling
{
 protected:
  std::vector<std::mt19937>& gens;
  MemoryBuffer* const RM;
  std::vector<Sequence>& episodes;
  const bool bSampleSequences;

  long nSequences() const;
  long nTransitions() const;
  void setMinMaxProb(const Real maxP, const Real minP);
  void updatePrefixes();
  void checkPrefixes();

 public:
  Sampling(std::vector<std::mt19937>&G, MemoryBuffer*const R, bool bSampleSeqs);
  virtual ~Sampling() {}
  virtual void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) = 0;
  virtual void prepare(std::atomic<bool>& needs_pass) = 0;
  void IDtoSeqStep_par(std::vector<Uint>& seq, std::vector<Uint>& obs,
                       const std::vector<Uint>& ret, const Uint nSeqs);
  void IDtoSeqStep(std::vector<Uint>& seq, std::vector<Uint>& obs,
                  const std::vector<Uint>& ret, const Uint nSeqs);
  virtual bool requireImportanceWeights() = 0;

  static std::unique_ptr<Sampling> prepareSampler(MemoryBuffer * const,
                                                  Settings&, DistributionInfo&);
};

class Sample_uniform : public Sampling
{
 public:
  Sample_uniform(std::vector<std::mt19937>&G, MemoryBuffer*const R, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

class Sample_impLen : public Sampling
{
  std::discrete_distribution<Uint> dist;
 public:
  Sample_impLen(std::vector<std::mt19937>&, MemoryBuffer*const, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

class TSample_shuffle : public Sampling
{
  std::vector<std::pair<unsigned, unsigned>> samples;
 public:
  TSample_shuffle(std::vector<std::mt19937>&, MemoryBuffer*const, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

class TSample_impRank : public Sampling
{
  int stepSinceISWeep = 0;
  std::discrete_distribution<Uint> distObs;
 public:
  TSample_impRank(std::vector<std::mt19937>&, MemoryBuffer*const, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

class TSample_impErr : public Sampling
{
  int stepSinceISWeep = 0;
  std::discrete_distribution<Uint> distObs;
 public:
  TSample_impErr(std::vector<std::mt19937>&, MemoryBuffer*const, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

class Sample_impSeq : public Sampling
{
  int stepSinceISWeep = 0;
  std::discrete_distribution<Uint> distObs;
 public:
  Sample_impSeq(std::vector<std::mt19937>&, MemoryBuffer*const, bool bSeqs);
  void sample(std::vector<Uint>& seq, std::vector<Uint>& obs) override;
  void prepare(std::atomic<bool>& needs_pass) override;
  bool requireImportanceWeights() override;
};

}
#endif // smarties_Sampling_h
