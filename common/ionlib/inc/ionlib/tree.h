#ifndef ION_TREE_H_
#define ION_TREE_H_
#include <vector>
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

namespace ion
{
	template <class T>
	class TreeNode
	{
	public:
		TreeNode(T);
		T GetData();
		void SetData(T data);
		void AddLeaf(T data);
		TreeNode<T> GetLeaf(uint32_t index);
		TreeNode<T>* GetParent();
		size_t NumLeafs()
		{
			return leafs_.size();
		}
		typename std::vector<TreeNode<T>>::iterator begin();
		typename std::vector<TreeNode<T>>::iterator end();
	private:
		T data_;
		std::vector<TreeNode<T>> leafs_;
		TreeNode<T>* parent_;
	};
}

template <class T>
ion::TreeNode<T>::TreeNode(T data)
{
	this->data_ = data;
	this->parent_ = NULL;
}

template <class T>
T ion::TreeNode<T>::GetData()
{
	return this->data_;
}
template <class T>
void ion::TreeNode<T>::SetData(T data)
{
	this->data_ = data;
}
template <class T>
void ion::TreeNode<T>::AddLeaf(T data)
{
	ion::TreeNode<T> new_leaf(data);
	new_leaf.parent_ = this;
	this->leafs_.push_back(new_leaf);
}
template <class T>
ion::TreeNode<T> ion::TreeNode<T>::GetLeaf(uint32_t index)
{
	LOGASSERT(index < this->leafs_.size());
	return this->leafs_[index];
}
template <class T>
ion::TreeNode<T>* ion::TreeNode<T>::GetParent()
{
	return this->parent_;
}
template <class T>
typename std::vector<ion::TreeNode<T>>::iterator ion::TreeNode<T>::begin()
{
	return this->leafs_.begin();
}
template <class T>
typename std::vector<ion::TreeNode<T>>::iterator ion::TreeNode<T>::end()
{
	return this->leafs_.end();
}
#endif //ION_TREE_H_