package rbtree

import (
	"fmt"
)

func (n *Node) grandparent(tree *Tree) *Node {
	if n.Parent == nil {
		return nil
	}
	return n.Parent.Parent
}

func (n *Node) uncle(tree *Tree) *Node {
	gp := n.grandparent(tree)
	if n.Parent == gp.Left {
		return gp.Right
	}
	return gp.Left
}

func (n *Node) case1(tree *Tree) {
	if n.Parent == tree.NIL {
		n.Color = BLACK
		return
	}
	n.case2(tree)
}

func (n *Node) case2(tree *Tree) {
	if n.Parent.Color == BLACK {
		return
	}
	n.case3(tree)
}

func (n *Node) case3(tree *Tree) {
	if n.uncle(tree) != tree.NIL && n.uncle(tree).Color == RED {
		n.Parent.Color = BLACK
		n.uncle(tree).Color = BLACK
		n.grandparent(tree).Color = RED
		n.grandparent(tree).case1(tree)
		return
	}
	n.case4(tree)
}

func (n *Node) case4(tree *Tree) {
	if n == n.Parent.Right && n.Parent == n.grandparent(tree).Left {
		n.rotateLeft(tree)
		n = n.Left
	} else if n == n.Parent.Left && n.Parent == n.grandparent(tree).Right {
		n.rotateRight(tree)
		n = n.Right
	}
	n.case5(tree)
}

func (n *Node) rotateLeft(tree *Tree) {
	p := n.Parent
	g := p.Parent
	y := n.Left

	n.Parent = g
	g.chChild(p, n)

	p.Parent = n
	p.Right = y
	if y != tree.NIL {
		y.Parent = p
	}
	n.Left = p
}

func (n *Node) chChild(a, b *Node) {
	if n.Left == a {
		n.Left = b
	} else if n.Right == a {
		n.Right = b
	}
}

func (n *Node) rotateRight(tree *Tree) {
	p := n.Parent
	g := p.Parent
	y := n.Right

	n.Parent = g
	g.chChild(p, n)

	p.Parent = n
	p.Left = y
	if y != tree.NIL {
		y.Parent = p
	}
	n.Right = p
}

func (n *Node) case5(tree *Tree) {
	n.Parent.Color = BLACK
	n.grandparent(tree).Color = RED
	if n.Parent == n.grandparent(tree).Left {
		n.Parent.rotateRight(tree)
	} else {
		n.Parent.rotateLeft(tree)
	}
}

func (n *Node) Count(tree *Tree) int {
	// if n is a leaf
	if n == tree.NIL {
		return 1
	}
	// get node's right and left count
	r := n.Right.Count(tree)
	l := n.Left.Count(tree)
	if r != l {
		// Panic because it doesn't the same
		err := fmt.Errorf("Not a rbtree %s:%d %s:%d", n.Left.String(tree), l, n.Right.String(tree), r)
		panic(err)
	}
	if n.Color == BLACK {
		r++
	}
	return r
}
