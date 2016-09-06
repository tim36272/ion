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
#ifndef ION_TREE_H_
#define ION_TREE_H_
#include <vector>
#include <deque>
#include "ionlib/log.h"
#include <iostream>
#include <fstream>

namespace ion
{
	//Used to define what order to traverse in for various functions
	enum traversal_type_t
	{
		DEPTH_FIRST,
		BREADTH_FIRST,
		INVALID_TRAVERSAL_TYPE
	};
	/*
		This class defines a node in a tree. There is no distinction between
		the root, an intermediate node, or a leaf: they are all tree nodes.
		There are some things that need to be updated eventually, for example:
			* Not sure if I want all the functions to return pointers or not.
			  Originally they were not but I had trouble with copying
			* The begin/end doesn't really do what you might expect
			* The iterator should (but doesn't) comply with std::iterator
			* Consistify usage of "child" vs "leaf" etc.
	*/
	template <class T>
	class TreeNode
	{
	public:
		//constructors
		TreeNode();
		TreeNode(const TreeNode<T>& rhs);
		TreeNode(T data, TreeNode<T>* parent);
		//Functions that operate on the node
		T GetData();
		void SetData(T data);
		void AddLeaf(T data);
		TreeNode<T>* GetLeaf(size_t index);
		size_t NumLeafs()
		{
			return leafs_.size();
		}
		TreeNode<T>* GetParent()
		{
			return parent_;
		}
		//This function prints in a graphviz-compatible format (dot)
		void print(std::ostream &output);
		//These iterators iterate over just the leafs, not the whole tree.
		//For whole-tree iterators, see TreeNode::iterator
		typename std::vector<TreeNode<T>*>::iterator begin();
		typename std::vector<TreeNode<T>*>::iterator end();
		//get a path from this node to data
		//TODO: I don't think this works if you give it a node in the middle of
		//a tree (it will trace all the way to the root, not the given node)
		std::vector<ion::TreeNode<T>*> GetPath(T data);
		//This iterator class can be set to iterate over the entire tree:
		//	* In depth first order
		//	* In breadth-first order
		class iterator
		{
		public:
			iterator();
			iterator(ion::traversal_type_t type);
			void init(TreeNode<T>* root);
			iterator& operator++();
			iterator& operator++(int);
			bool operator==(const iterator& rhs);
			bool operator!=(const iterator& rhs);
			TreeNode<T>* operator*();
			//Use this instead of an end() function (I'll fix this later)
			bool complete();
		private:
			std::deque<TreeNode<T>*> queue_;
			ion::traversal_type_t traversal_type_;
		};
	private:
		//The data stored in the node
		T data_;
		//The children of this node
		std::vector<TreeNode<T>*> leafs_;
		//Pointer to this node's parent. nullptr for root
		TreeNode<T>* parent_;
	};
} //namespace ion

template <class T>
ion::TreeNode<T>::TreeNode()
{
	this->parent_ = nullptr;
}
template <class T>
ion::TreeNode<T>::TreeNode(const TreeNode<T>& rhs)
{
	this->data_ = rhs.data_;
	this->leafs_ = rhs.leafs_;
	this->parent_ = rhs.parent_;
}
template <class T>
ion::TreeNode<T>::TreeNode(T data, ion::TreeNode<T>* parent)
{
	this->data_ = data;
	this->parent_ = parent;
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
	ion::TreeNode<T>* new_leaf= new ion::TreeNode<T>(data, this);
	this->leafs_.push_back(new_leaf);
}
template <class T>
ion::TreeNode<T>* ion::TreeNode<T>::GetLeaf(size_t index)
{
	LOGASSERT(index < this->leafs_.size());
	return this->leafs_[index];
}
template <class T>
typename std::vector<ion::TreeNode<T>*>::iterator ion::TreeNode<T>::begin()
{
	return this->leafs_.begin();
}
template <class T>
typename std::vector<ion::TreeNode<T>*>::iterator ion::TreeNode<T>::end()
{
	return this->leafs_.end();
}
template <class T>
typename std::vector<ion::TreeNode<T>*> ion::TreeNode<T>::GetPath(T data)
{
	std::vector<ion::TreeNode<T>*> path;
	ion::TreeNode<T>::iterator it(ion::BREADTH_FIRST);
	it.init(this);
	while (!it.complete())
	{
		if ((*it)->GetData() == data) {
			//this is the element we were searching for, return the path to this one
			break;
		}
		++it;
	}
	if (it.complete())
	{
		//return empty path
		return path;
	}
	ion::TreeNode<T>* cursor = *it;
	//push the cursor because the found element always has to be pushed,
	//otherwise we wouldn't be able to distinguish between a missing element
	//and an element at the root
	path.push_back(cursor);
	while ((cursor->GetParent() != nullptr) && (cursor != this))
	{
		cursor = cursor->GetParent();
		path.push_back(cursor);
	} 
	return path;
}

template <class T>
typename void ion::TreeNode<T>::print(std::ostream &output)
{
	ion::TreeNode<T>::iterator it(ion::BREADTH_FIRST);
	it.init(this);
	ion::TreeNode<T>* item;
	output << "digraph G {" << std::endl;
	while (!it.complete())
	{
		item = *it;
		if (item->parent_ == nullptr)
		{
			output << "root -> \"" << (item->GetData()) << "\"" << std::endl;
		} else
		{
			output << "\"" << (item->parent_->GetData()) << "\" -> \"" << (item->GetData()) << "\"" << std::endl;
		}
		++it;
	}
	output << "}";
}

namespace ion
{
	template <class T>
	TreeNode<T>::iterator::iterator(): traversal_type_(ion::INVALID_TRAVERSAL_TYPE)
	{
	}

	template <class T>
	TreeNode<T>::iterator::iterator(traversal_type_t type)
	{
		this->traversal_type_ = type;
	}
	template <class T>
	void TreeNode<T>::iterator::init(TreeNode<T>* root)
	{
		queue_.push_back(root);
	}
	template <class T>
	typename TreeNode<T>::iterator& TreeNode<T>::iterator::operator++()
	{
		//if there is nothing in the queue then we have traversed the whole thing
		if (queue_.size() == 0)
		{
			LOGFATAL("Attempted to increment a tree iterator past the end of the tree");
		}
		TreeNode<T>* cursor;
		switch (traversal_type_)
		{
			case ion::BREADTH_FIRST:
				//dequeue the front of the queue and non-recurisvely (i.e. only
				//to depth=1) add its children to the back of the queue
				cursor = queue_.front();
				queue_.pop_front();
				for (size_t branch_index = 0; branch_index < cursor->NumLeafs(); ++branch_index)
				{
					queue_.push_back(cursor->GetLeaf(branch_index));
				}
				break;
			case ion::DEPTH_FIRST:
				//dequeue the back of the queue and non-recurisvely (i.e. only
				//to depth=1) add its children to the front of the queue

				//notice the queueing is actually done in reverse so that we
				//traverse in the common top-down left-right order (although
				//the other order would still technically be a depth-first
				//search)
				cursor = queue_.back();
				queue_.pop_back();
				for (size_t branch_index = cursor->NumLeafs(); branch_index != 0; --branch_index)
				{
					queue_.push_back(cursor->GetLeaf(branch_index-1));
				}
				break;
			default:
				LOGFATAL("Invalid traversal type %d", this->traversal_type_);
				break;
		}
		return *this;
	}
	template <class T>
	typename TreeNode<T>::iterator& TreeNode<T>::iterator::operator++(int)
	{
		TreeNode<T>::iterator tmp(*this);
		operator++();
		return tmp;
	}
	template <class T>
	typename bool TreeNode<T>::iterator::operator==(typename const TreeNode<T>::iterator& rhs)
	{
		//iterators must be traversal-type equivalent (consider relaxing this in the future)
		if (rhs.traversal_type_ != this->traversal_type_)
		{
			return false;
		}
		//heads must be the same
		switch (this->traversal_type_)
		{
			case ion::BREADTH_FIRST:
				//breadth-first uses the front of the queue as next
				return this->queue_.front() == rhs.queue_.front();
				break
			case ion::DEPTH_FIRST:
				//depth-first uses the back of the queue as next
				return this->queue_.back() == rhs.queue_.back();
				break;
			default:
				LOGFATAL("Invalid traversal type %d", this->traversal_type_);
				break;
		}
	}
	template <class T>
	typename bool TreeNode<T>::iterator::operator!=(typename const TreeNode<T>::iterator& rhs)
	{
		return !this->operator==(rhs);
	}
	template <class T>
	typename TreeNode<T>* TreeNode<T>::iterator::operator*()
	{
		switch (traversal_type_)
		{
			case ion::BREADTH_FIRST:
				return queue_.front();
				break;
			case ion::DEPTH_FIRST:
				return queue_.back();
				break;
			default:
				LOGFATAL("Invalid traversal type %d", this->traversal_type_);
				break;
		}
		LOGFATAL("Shouldn't have ran to this line");
		return nullptr;
	}
	template <class T>
	typename bool TreeNode<T>::iterator::complete()
	{
		return queue_.size() == 0;
	}
}; //namespace ion
#endif //ION_TREE_H_
