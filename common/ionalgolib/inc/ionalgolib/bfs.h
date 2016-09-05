/*
This file is part of Ionlib.

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ION_ALGO_BFS_H_
#define ION_ALGO_BFS_H_
#include "ionlib\tree.h"
namespace ion
{
	namespace algo
	{
		template <class T>
		std::vector<ion::TreeNode<T>> bfs(ion::TreeNode<T> root, T goal);
	}
};

template <class T>
std::vector<ion::TreeNode<T>> ion::algo::bfs(ion::TreeNode<T> root, T goal)
{
	//check if this node is the goal (could happen if the original root is the goal)
	if (root.GetData() == goal)
	{
		//create a vector with myself in it
		std::vector<ion::TreeNode<T>> vec;
		vec.push_back(root);
		return vec;
	}
	//check if any of the children are the goal
	for (typename std::vector<ion::TreeNode<T>>::iterator it = root.begin(); it != root.end(); ++it)
	{
		if (it->GetData() == goal)
		{
			//create a vector with that node in it
			std::vector<ion::TreeNode<T>> vec;
			vec.push_back(*it);
			//push myself on the queue
			vec.push_back(root);
			return vec;
		}
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
#endif //ION_ALGO_BFS_H_