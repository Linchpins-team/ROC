package rbtree

import (
	"fmt"
)

func (n *Node) grandparent() *Node {
	if n.Parent == nil {
		return nil
	}
	return n.Parent.Parent
}

func (n *Node) uncle() *Node {
	gp := n.grandparent()
	if n.Parent == gp.Left {
		return gp.Right
	}
	return gp.Left
}

func (n *Node) case1() {
	if n.Parent == nil {
		n.Color = BLACK
		return
	}
	n.case2()
}

func (n *Node) case2() {
	if n.Parent.Color == BLACK {
		return
	}
	n.case3()
}

func (n *Node) case3() {
	if n.uncle() != nil && n.uncle().Color == RED {
		n.Parent.Color = BLACK
		n.uncle().Color = BLACK
		n.grandparent().Color = RED
		n.grandparent().case1()
		return
	}
	n.case4()
}

func (n *Node) case4() {
	if n == n.Parent.Right && n.Parent == n.grandparent().Left {
		n.rotateLeft()
		n = n.Left
	} else if n == n.Parent.Left && n.Parent == n.grandparent().Right {
		n.rotateRight()
		n = n.Right
	}
	n.case5()
}

func (n *Node) rotateLeft() {
	p := n.Parent
	g := p.Parent
	y := n.Left
	n.Parent = g
	if g == nil {
		*n.Root = n
	} else {
		if g.Left == p {
			g.Left = n
		} else {
			g.Right = n
		}
	}
	p.Parent = n
	p.Right = y
	if y != nil {
		y.Parent = p
	}
	n.Left = p
}

func (n *Node) rotateRight() {
	p := n.Parent
	g := p.Parent
	y := n.Right
	n.Parent = g
	if g == nil {
		*n.Root = n
	} else {
		if g.Left == p {
			g.Left = n
		} else {
			g.Right = n
		}
	}
	p.Parent = n
	p.Left = y
	if y != nil {
		y.Parent = p
	}
	n.Right = p
}

func (n *Node) case5() {
	n.Parent.Color = BLACK
	n.grandparent().Color = RED
	if n.Parent == n.grandparent().Left {
		n.Parent.rotateRight()
	} else {
		n.Parent.rotateLeft()
	}
}

func (n *Node) Count() int {
	// if n is a leaf
	if n == nil {
		return 1
	}
	// get node's right and left count
	count := n.Right.Count()
	if l := n.Left.Count(); count != l {
		// Panic because it doesn't the same
		err := fmt.Errorf("Not a rbtree %s:%d %s:%d", n.Left, l, n.Right, count)
		panic(err)
	}
	if n.Color == BLACK {
		count++
	}
	return count
}
