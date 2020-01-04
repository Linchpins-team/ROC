package rbtree

import (
	"fmt"
)

func (n workNode) Insert(value ValueObject) {
	child := n.selectChild(value)
	// if value is self
	if child == nil {
		return
	}
	// if child hasn't created
	if *child == nil {
		// if node not exist, create a new node
		*child = &Node{
			Parent:      n.Node,
			Color:       RED,
			ValueObject: value,
		}
		w := work(*child, n.NIL)
		w.case1()
	} else {
		work(*child, n.NIL).Insert(value)
	}
}

func (n workNode) selectChild(value ValueObject) **Node {
	// if n is root's parent, always select left
	if n.Node == n.NIL || value.Value() < n.Value() {
		return &n.Left
	}
	if value.Value() > n.Value() {
		return &n.Right
	}
	return nil
}

// Search a ValueObject by a key
func (n workNode) Search(key uint64) ValueObject {
	if n.isNil() {
		return n.left().Search(key)
	}
	if n.Node == nil {
		return nil
	}
	if key < n.Value() {
		return n.left().Search(key)
	}
	if key > n.Value() {
		return n.right().Search(key)
	}
	return n.ValueObject
}

// Delete all the tree
func (n workNode) Delete() {
	if n.Node == nil {
		return
	}
	n.right().Delete()
	n.Right = nil
	n.left().Delete()
	n.Left = nil
	n.ValueObject = nil
}

func (n workNode) String() string {
	if n.isNil() {
		return n.left().String()
	}
	s := ""
	s += fmt.Sprint(n.Value())
	if n.Color == BLACK {
		s = "*" + s + "*"
	}
	if n.Left != nil {
		s = n.left().String() + " " + s
	}
	if n.Right != nil {
		s += " " + n.right().String()
	}
	return "(" + s + ")"
}
