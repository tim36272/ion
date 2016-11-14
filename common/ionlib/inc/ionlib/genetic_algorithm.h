/*
This file is part of Ionlib.  Copyright (C) 2016  Tim Sweet

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ionlib.If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ION_GENETIC_ALGORITHM_H_
#define ION_GENETIC_ALGORITHM_H_
#include <vector>
#include <algorithm>
#include <numeric>
#include "ionlib/math.h"
#include "ionlib/log.h"
namespace ion
{
	template <class T>
	class GeneticAlgorithm
	{
	public:
		GeneticAlgorithm(size_t num_members, size_t chromosome_length, double mutation_probability, double crossover_probability)
		{
			if (chromosome_length > RAND_MAX || num_members > RAND_MAX)
			{
				LOGFATAL("Your chromosome or population size is greater than RAND_MAX, so the random number generator will never select some members");
			}
			mutation_probability_ = mutation_probability;
			crossover_probability_ = crossover_probability;
			population_.reserve(num_members);
			fitness_.resize(num_members);
			num_evaluations_ = 0;
			for (uint32_t member_index = 0; member_index < num_members; ++member_index)
			{
				T member;
				population_.push_back(member);
			}
		}
		virtual void Mutate() = 0;
		virtual void Select() = 0;
		void NextGeneration()
		{
			//do fitness proportional selection
			Select();
			Mutate();
			EvaluateMembers();
		}
		double GetAverageFitness()
		{
			return std::accumulate(fitness_.begin(), fitness_.end(), 0.0) / fitness_.size();
		}
		double GetMaxFitness()
		{
			return *(std::max_element(fitness_.begin(), fitness_.end()));
		}
		T GetEliteMember()
		{
			std::vector<double>::iterator elite_it = std::max_element(fitness_.begin(), fitness_.end());
			return population_[elite_it - fitness_.begin()];
		}
		double GetMinFitness()
		{
			return *(std::min_element(fitness_.begin(), fitness_.end()));
		}
		T GetWorstMember()
		{
			size_t worst_index = std::min_element(fitness_.begin(), fitness_.end()) - fitness_.begin();
			return population_[worst_index];

		}
		uint32_t GetNumEvals()
		{
			return num_evaluations_;
		}
		virtual void EvaluateMembers() = 0;
	protected:
		std::vector<T> population_;
		std::vector<double> fitness_;
		double mutation_probability_;
		double crossover_probability_;
		uint32_t num_evaluations_;
	};
	template <>
	class GeneticAlgorithm<std::vector<bool>>
	{
	public:
		GeneticAlgorithm(size_t num_members, size_t chromosome_length, double mutation_probability, double crossover_probability)
		{
			if (chromosome_length > RAND_MAX || num_members > RAND_MAX)
			{
				LOGFATAL("Your chromosome or population size is greater than RAND_MAX, so the random number generator will never select some members");
			}
			mutation_probability_ = mutation_probability;
			crossover_probability_ = crossover_probability;
			population_.reserve(num_members);
			fitness_.resize(num_members);
			num_evaluations_ = 0;
			for (uint32_t member_index = 0; member_index < num_members; ++member_index)
			{
				std::vector<bool> member;
				member.reserve(chromosome_length);
				for (uint32_t gene_index = 0; gene_index < chromosome_length; ++gene_index)
				{
					member.push_back(ion::randlf(0.0, 1.0) > 0.5);
				}
				population_.push_back(member);
			}
		}
		virtual void Mutate()
		{
			//we start with the second element because we are doing elite selection
			for (std::vector<std::vector<bool>>::iterator member_it = population_.begin()+1; member_it != population_.end(); ++member_it)
			{
				for (std::vector<bool>::iterator gene_it = member_it->begin(); gene_it != member_it->end(); ++gene_it)
				{
					//generate a random number between 0 and 1
					double random_number = ion::randlf(0.0, 1.0);
					if (random_number < mutation_probability_)
					{
						*gene_it = !*gene_it;
					}
				}
			}
		}
		virtual void Select()
		{
			//note that the fitnesses must already be set
			//get the total fitness
			double fitness_sum = std::accumulate(fitness_.begin(), fitness_.end(), 0.0);
			//create a temporary population
			std::vector<std::vector<bool>> temp_population;
			temp_population.reserve(population_.size());
			//since we are using elite selection, push the elite member
			temp_population.push_back(GetEliteMember());
			//start selecting elements by treating the fitness as cumulative density function
			for (uint32_t member_index = 1; member_index < population_.size(); ++member_index)
			{
				//get a number between 0 and fitness_sum
				double selected_individual = ion::randlf(0.0, fitness_sum);
				//traverse the CDF until selected_individual is found
				std::vector<double>::iterator parent_it;
				uint32_t parent_index = 0;
				for (parent_it = fitness_.begin(); parent_it != fitness_.end(); ++parent_it,++parent_index)
				{
					selected_individual -= *parent_it;
					if (selected_individual < 0.0000000001)
					{
						break;
					}
				}
				LOGASSERT(selected_individual <= 0.0000000001);
				//now parent_it is the member that is getting propogated to the next generation
				temp_population.push_back(*(population_.begin()+parent_index));
				//if this iteration is an odd number (that is, we have pushed an even number of elements onto the queue) attempt crossover on these two members
				if (member_index % 1 == 1)
				{
					double random_number = ion::randlf(0.0, 1.0);
					if (random_number < crossover_probability_)
					{
						//we will do crossover

						//select a point to start the crossover at
						uint32_t crossover_location = (uint32_t)ion::randull(0, (uint32_t)population_.begin()->size() - 1);
						//perform the crossover
						//get the last two members
						std::vector<std::vector<bool>>::reverse_iterator mate1 = temp_population.rbegin();
						std::vector<std::vector<bool>>::reverse_iterator mate2 = mate1 + 1;
						std::swap_ranges(mate1->begin(), mate1->begin() + crossover_location, mate2->begin());
					}
				}
			}
			population_.swap(temp_population);
		}
		void NextGeneration()
		{
			//do fitness proportional selection
			Select();
			Mutate();
			EvaluateMembers();
		}
		double GetAverageFitness()
		{
			return std::accumulate(fitness_.begin(),fitness_.end(),0.0)/fitness_.size();
		}
		double GetMaxFitness()
		{
			return *(std::max_element(fitness_.begin(), fitness_.end()));
		}
		std::vector<bool> GetEliteMember()
		{
			std::vector<double>::iterator elite_it = std::max_element(fitness_.begin(), fitness_.end());
			return population_[elite_it - fitness_.begin()];
		}
		double GetMinFitness()
		{
			return *(std::min_element(fitness_.begin(), fitness_.end()));
		}
		std::vector<bool> GetWorstMember()
		{
			size_t worst_index = std::min_element(fitness_.begin(), fitness_.end()) - fitness_.begin();
			return population_[worst_index];

		}
		uint32_t GetNumEvals()
		{
			return num_evaluations_;
		}
		virtual void EvaluateMembers() = 0;
	protected:
		std::vector<std::vector<bool>> population_;
		std::vector<double> fitness_;
		double mutation_probability_;
		double crossover_probability_;
		uint32_t num_evaluations_;
	};

};
#endif //ION_GENETIC_ALGORITHM_H_