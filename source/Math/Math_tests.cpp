//
//  smarties
//  Copyright (c) 2018 CSE-Lab, ETH Zurich, Switzerland. All rights reserved.
//  Distributed under the terms of the MIT license.
//
//  Created by Guido Novati (novatig@ethz.ch).
//

#include "../Learners/AlgoFactory.h"
#include "Discrete_advantage.h"
#include "Gaussian_policy.h"
#include "Quadratic_advantage.h"
#include "Gaus_advantage.h"
#include <fstream>

namespace smarties
{

void Gaussian_policy::test(const Rvec& act, const Rvec& beta) const
{
  Rvec _grad(netOutputs.size(), 0);
  //const Rvec cntrolgrad = control_grad(a, -1);
  const Rvec div_klgrad = div_kl_grad(beta);
  const Rvec policygrad = policy_grad(act, 1);
  std::ofstream fout("mathtest.log", std::ios::app);
  for(Uint i = 0; i<2*nA; ++i)
  {
    Rvec out_1 = netOutputs, out_2 = netOutputs;
    if(i>=nA && start_prec == 0) continue;
    const Uint index = i>=nA ? start_prec+i-nA : start_mean+i;
    out_1[index] -= nnEPS;
    out_2[index] += nnEPS;
    Gaussian_policy p1(std::vector<Uint>{start_mean, start_prec}, aInfo, out_1);
    Gaussian_policy p2(std::vector<Uint>{start_mean, start_prec}, aInfo, out_2);

    const Real p_1 = p1.evalLogProbability(act);
    const Real p_2 = p2.evalLogProbability(act);
    const Real d_1 = p1.kl_divergence(beta);
    const Real d_2 = p2.kl_divergence(beta);
    {
      finalize_grad(policygrad, _grad);
      const double diffVal = (p_2-p_1)/(2*nnEPS);
      const double gradVal = _grad[index];
      const double errVal  = std::fabs(_grad[index]-(p_2-p_1)/(2*nnEPS));
      fout<<"LogPol var grad "<<i<<" finite differences "
      <<diffVal<<" analytic "<<gradVal<<" error "<<errVal<<"\n";
    }

    {
      finalize_grad(div_klgrad, _grad);
      const double diffVal = (d_2-d_1)/(2*nnEPS);
      const double gradVal = _grad[index];
      const double errVal  = std::fabs(_grad[index]-(d_2-d_1)/(2*nnEPS));
      fout<<"DivKL var grad "<<i<<" finite differences "
      <<diffVal<<" analytic "<<gradVal<<" error "<<errVal<<"\n";
    }
  }
  fout.close();
}


void Discrete_policy::test(const Uint act, const Rvec& beta) const
{
  Rvec _grad(netOutputs.size());
  //const Rvec cntrolgrad = control_grad(-1);
  const Rvec div_klgrad = div_kl_grad(beta);
  const Rvec policygrad = policy_grad(act, 1);
  std::ofstream fout("mathtest.log", std::ios::app);
  //values_grad(act, 1, _grad);
  for(Uint i = 0; i<nA; ++i)
  {
    Rvec out_1 = netOutputs, out_2 = netOutputs;
    const Uint index = start_prob+i;
    out_1[index] -= nnEPS;
    out_2[index] += nnEPS;
    Discrete_policy p1(std::vector<Uint>{start_prob}, aInfo, out_1);
    Discrete_policy p2(std::vector<Uint>{start_prob}, aInfo, out_2);
    //const Real A_1 = p1.computeAdvantage(act);
    //const Real A_2 = p2.computeAdvantage(act);
    const Real p_1 = p1.evalLogProbability(act);
    const Real p_2 = p2.evalLogProbability(act);
    const Real d_1 = p1.kl_divergence(beta);
    const Real d_2 = p2.kl_divergence(beta);

    finalize_grad(div_klgrad, _grad);
    {
      const double diffVal = (d_2-d_1)/(2*nnEPS);
      const double gradVal = _grad[index];
      const double errVal  = std::fabs(_grad[index]-(d_2-d_1)/(2*nnEPS));
      fout<<"DivKL var grad "<<i<<" "<<act<<" finite differences "
          <<diffVal<<" analytic "<<gradVal<<" error "<<errVal<<"\n";
    }

    // finalize_grad(cntrolgrad, _grad);
    //if(fabs(_grad[index]-(A_2-A_1)/(2*nnEPS))>1e-7)
    // _die("Control grad %u %u: finite differences %g analytic %g error %g \n",
    //i,act,(A_2-A_1)/(2*nnEPS),_grad[index],fabs(_grad[index]-(A_2-A_1)/(2*nnEPS)));

    finalize_grad(policygrad, _grad);
    {
      const double diffVal = (p_2-p_1)/(2*nnEPS);
      const double gradVal = _grad[index];
      const double errVal  = std::fabs(_grad[index]-(p_2-p_1)/(2*nnEPS));
      fout<<"LogPol var grad "<<i<<" "<<act<<" finite differences "
          <<diffVal<<" analytic "<<gradVal<<" error "<<errVal<<"\n";
    }
  }
  fout.close();
}


void Gaussian_advantage::test(const Rvec& act, std::mt19937*const gen) const
{
  const Uint numNetOutputs = netOutputs.size();
  Rvec _grad(numNetOutputs, 0);
  grad(act, 1, _grad);
  std::ofstream fout("mathtest.log", std::ios::app);
  for(Uint i = 0; i<nL; ++i)
  {
    Rvec out_1 = netOutputs, out_2 = netOutputs;
    const Uint index = start_coefs+i;
    out_1[index] -= 0.0001; out_2[index] += 0.0001;

    Gaussian_advantage a1(std::vector<Uint>{start_coefs}, aInfo, out_1, policy);
    Gaussian_advantage a2(std::vector<Uint>{start_coefs}, aInfo, out_2, policy);
    const Real A_1 = a1.computeAdvantage(act), A_2 = a2.computeAdvantage(act);
    const Real fdiff =(A_2-A_1)/.0002, abserr = std::fabs(_grad[index]-fdiff);
    const Real scale = std::max(std::fabs(fdiff), std::fabs(_grad[index]));
    //if(abserr>1e-7 && abserr/scale>1e-4)
    {
      fout<<"Adv grad "<<i<<" finite differences "<<fdiff<<" analytic "
        <<_grad[index]<<" error "<<abserr<<" "<<abserr/scale<<"\n";
    }
  }
  fout.close();
}


void Quadratic_advantage::test(const Rvec& act, std::mt19937*const gen) const
{
  Rvec _grad(netOutputs.size(), 0);
  grad(act, 1, _grad);
  std::ofstream fout("mathtest.log", std::ios::app);
  for(Uint i = 0; i<nL+nA; ++i)
  {
    Rvec out_1 = netOutputs, out_2 = netOutputs;
    if(i>=nL && start_mean == 0) continue;
    const Uint index = i>=nL ? start_mean+i-nL : start_matrix+i;
    out_1[index] -= nnEPS;
    out_2[index] += nnEPS;

    Quadratic_advantage a1 = Quadratic_advantage(std::vector<Uint>{start_matrix,
      start_mean}, aInfo, out_1, policy);

    Quadratic_advantage a2 = Quadratic_advantage(std::vector<Uint>{start_matrix,
      start_mean}, aInfo, out_2, policy);

    const Real A_1 = a1.computeAdvantage(act);
    const Real A_2 = a2.computeAdvantage(act);
    {
      const double diffVal = (A_2-A_1)/(2*nnEPS);
      const double gradVal = _grad[index];
      const double errVal  = std::fabs(_grad[index]-(A_2-A_1)/(2*nnEPS));
      fout<<"Advantage grad "<<i<<" finite differences "
          <<diffVal<<" analytic "<<gradVal<<" error "<<errVal<<"\n";
    }
  }
  fout.close();
}

} // end namespace smarties
