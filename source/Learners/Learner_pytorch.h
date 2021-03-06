//
//  smarties
//  Copyright (c) 2018 CSE-Lab, ETH Zurich, Switzerland. All rights reserved.
//  Distributed under the terms of the MIT license.
//
//  Created by Guido Novati (novatig@ethz.ch).
//

#ifndef smarties_Learner_pytorch_h
#define smarties_Learner_pytorch_h

#include "Learner.h"
#include "../Utils/ParameterBlob.h"

namespace pybind11
{
struct object;
}

namespace smarties
{

class Learner_pytorch: public Learner
{
 public:
  const bool bSampleSequences = settings.bSampleSequences;
  // hyper-parameters:
  const Uint batchSize = settings.batchSize_local;
  const Uint ESpopSize = settings.ESpopSize;
  const Real learnR = settings.learnrate;
  const Real explNoise = settings.explNoise;


 protected:

  void spawnTrainTasks();
  std::vector<pybind11::object> Nets;

 public:
  Learner_pytorch(MDPdescriptor& MDP_, Settings& S_, DistributionInfo& D_);

  static Uint getnDimPolicy(const ActionInfo*const aI)
  {
    return 2*aI->dim();
  }
  
  void select(Agent& ) override;
  void setupTasks(TaskQueue& tasks) override;
  virtual ~Learner_pytorch() override;

  virtual void getMetrics(std::ostringstream& buff) const override;
  virtual void getHeaders(std::ostringstream& buff) const override;
  virtual void save() override;
  virtual void restart() override;

};

}
#endif
