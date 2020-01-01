package rbtree

import (
	"fmt"
)

type workNode struct {
	NIL *Node
	*Node
}

func (n workNode) parent() workNode {
	return workNode{
		NIL: n.NIL,
		Node: n.Parent,
	}
}

func (n workNode) grandparent() workNode {
	if n.Parent == nil {
		return workNode{
			NIL: n.NIL,
			Node: nil,
		}
	}
	return workNode{
		NIL: n.NIL,
		Node: n.Parent.Parent,
	}
}

func (n workNode) uncle() (w workNode) {
	gp := n.grandparent()
	w = workNode{
		NIL: n.NIL,
	}
	if n.Parent == gp.Left {
		w.Node = gp.Right
		return
	}
	w.Node = gp.Left
	return w
}

func (n workNode) case1() {
	if n.parent().isNil() {
		n.Color = BLACK
		return
	}
	n.case2()
}

func (n workNode) case2() {
	if n.Parent.Color == BLACK {
		return
	}
	n.case3()
}

func (n workNode) case3() {
	uncle := n.uncle()
	gr := n.grandparent()
	if !uncle.isNil() && uncle.Color == RED {
		n.Parent.Color = BLACK
		uncle.Color = BLACK
		gr.Color = RED
		gr.case1()
		return
	}
	n.case4()
}

func (n workNode) isNil() bool {
	return n.Node == n.NIL
}

func (n workNode) case4() {
	p := n.Parent
	g := p.Parent
	if n.Node == p.Right && p == g.Left {
		n.rotateLeft()
		n = n.left()
	} else if n.Node == p.Left && p == g.Right {
		n.rotateRight()
		n = n.right()
	}
	n.case5()
}

func (n workNode) left() workNode {
	return workNode{
		NIL: n.NIL,
		Node: n.Left,
	}
}

func (n workNode) right() workNode {
	return workNode{
		NIL: n.NIL,
		Node: n.Right,
	}
}

func (n workNode) rotateLeft() {
	p := n.Parent
	g := p.Parent
	y := n.Left

	n.Parent = g
	g.chChild(p, n.Node)

	p.Parent = n.Node
	p.Right = y
	if y != n.NIL {
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

func (n workNode) rotateRight() {
	p := n.Parent
	g := p.Parent
	y := n.Right

	n.Parent = g
	g.chChild(p, n.Node)

	p.Parent = n.Node
	p.Left = y
	if y != n.NIL {
		y.Parent = p
	}
	n.Right = p
}

func (n workNode) case5() {
	p := n.parent()
	g := n.grandparent()
	p.Color = BLACK
	
	g.Color = RED
	if p.Node == g.Left {
		p.rotateRight()
	} else {
		p.rotateLeft()
	}
}

func (n workNode) Count() int {
	// if n is a leaf
	if n.Node == n.NIL {
		return 1
	}
	// get node's right and left count
	r := n.right().Count()
	l := n.left().Count()
	if r != l {
		// Panic because it doesn't the same
		err := fmt.Errorf("Not a rbtree %s:%d %s:%d", n.Left.Work(n.NIL).String(), l, n.Right.Work(n.NIL).String(), r)
		panic(err)
	}
	if n.Color == BLACK {
		r++
	}
	return r
}

func (n *Node) Work(NIL *Node) workNode {
	return workNode{
		NIL: NIL,
		Node: n,
	}
}