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
#ifndef ION_ALGO_TRAVERSE_H_
#define ION_ALGO_TRAVERSE_H_
#include "ionlib\tree.h"
#include "ionlib\log.h"
namespace ion
{
	namespace algo
	{
		enum traversal_type_t
		{
			DEPTH_FIRST,
			BREADTH_FIRST
		};
		template <class T>
		TreeIterator<TreeNode<T>> traverse(traversal_type_t type, ion::TreeNode<T> root);
		template <class T>
		class TreeIterator : public std::iterator<std::input_iterator_tag, ion::TreeNode<T>>
		{
			TreeNode<T>* root_;
			TreeNode<T>* cursor_;
			ion::algo::traversal_type_t traversal_type_;
			uint32_t branch_index_;
			TreeIterator(TreeNode<T>* root, traversal_type_t type) : this->root_(root), this->cursor_(root), this->traversal_type_(type)
			{
				this->branch_index_ = 0;
			}
			TreeIterator(const TreeIterator& rhs) : this->root_(rhs.root_), this->cursor_(rhs.cursor_), this->traversal_type_(rhs.traversal_type_)
			{
				this->branch_index_ = 0;
			}
			TreeIterator& operator++()
			{
				switch (traversal_type_)
				{
					case ion::algo::BREADTH_FIRST:
						LOGASSERT(cursor_);
						LOGASSERT(branch_index_ < 2*cursor_->NumLeafs());
						if (branch_index_ == 0)//at self
						{
							++branch_index_;
							return cursor_;
						}
						if (branch_index_ <= cursor_->NumLeafs())//we are iterating through the leafs the first time and returning them
						{
							//return the next leaf
							return cursor_.GetLeaf((branch_index_++)-1);
						} else if (branch_index_ == (cursor->NumLeafs()+1)) //we just finished exploring all the leafs of this leaf, so go back to the parent to explore our siblings
						{
							//check if the parent has more children to explore
							//figure out which child index I was
							//TODO: Check for NULL parent
							TreeNode<T>* parent = cursor_->GetParent();
							LOGASSERT(parent != NULL);
							for (uint32_t parent_branch_index = 0; parent_branch_index < parent->NumLeafs(); ++parent_branch_index)
							{
								if (parent->GetLeaf(parent_branch_index) == cursor_)
								{
									//so I am branch # parent_branch_index of my parent.
									//if there are more branches then we want to go to them
									if (parent_branch_index < parent->NumLeafs() - 1)
									{
										//go to my sibling
										cursor_ = parent->GetLeaf(parent_branch_index + 1);
										branch_index_ = 1;
										//note that parent didn't change
									} else
									{
										//I was the last child of my parent. So we need to go back up to my grandparent and do the same thing
										cursor_ = parent;
										parent = parent->GetParent();
										//
										parent_branch_index = 0;
									}
								}
							}
						} else if (branch_index_ < cursor_->NumLeafs() * 2)//we are iterating through the children of the leafs
						{
							//go into the next child
							this->root_ = 
							//need to handle stepping into and out of child leafs
						} 
						break;
					case ion::algo:::DEPTH_FIRST

						break;
					default:
						LOGFATAL("Invalid traversal type %d", this->traversal_type_);
						break;
				}
				++p; return *this;
			}
			MyIterator operator++(int)
			{
				MyIterator tmp(*this); operator++(); return tmp;
			}
			bool operator==(const MyIterator& rhs)
			{
				return p == rhs.p;
			}
			bool operator!=(const MyIterator& rhs)
			{
				return p != rhs.p;
			}
			int& operator*()
			{
				return *p;
			}
		};
	}
};
#endif //ION_ALGO_TRAVERSE_H_