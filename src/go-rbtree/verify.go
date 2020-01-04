package rbtree

import (
	"fmt"
)

func (n workNode) Count() int {
	// if n is a leaf
	if n.Node == n.NIL {
		return n.left().Count()
	}
	if n.Node == nil {
		return 1
	}
	// get node's right and left count
	r := n.right().Count()
	l := n.left().Count()
	if r != l {
		// Panic because it doesn't the same
		err := fmt.Errorf("Not a rbtree %s:%d %s:%d", n.left(), l, n.right(), r)
		panic(err)
	}
	if n.Color == BLACK {
		r++
	}
	return r
}
