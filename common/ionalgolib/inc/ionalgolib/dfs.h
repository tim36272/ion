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
#ifndef ION_ALGO_DFS_H_
#define ION_ALGO_DFS_H_
#include "ionlib\tree.h"
namespace ion
{
	namespace algo
	{
		template <class T>
		std::vector<ion::TreeNode<T>> dfs(ion::TreeNode<T> root, T goal);
	}
};


template <class T>
std::vector<ion::TreeNode<T>> ion::algo::dfs(ion::TreeNode<T> root, T goal)
{
	//check if this node is the goal (could happen if the actuall root is the goal)
	if (root.GetData() == goal)
	{
		//create a vector with myself in it
		std::vector<ion::TreeNode<T>> vec;
		vec.push_back(root);
		return vec;
	}
	//for each of the leaves, call bfs until the goal is found
	for (typename std::vector<ion::TreeNode<T>>::iterator it = root.begin(); it != root.end(); ++it)
	{
		typename std::vector<ion::TreeNode<T>> vec = ion::algo::bfs(*it, goal);
		if (!vec.empty())
		{
			//we have found the path, add myself to it and return
			vec.push_back(root);
			return vec;
		}
	}
	//if we get this far then we got to a leaf node without finding the goal, thus the goal is not on this branch
	//return an empty vector to indicate failure
	typename std::vector<ion::TreeNode<T>> vec;
	return vec;
}
#endif //ION_ALGO_DFS_H_