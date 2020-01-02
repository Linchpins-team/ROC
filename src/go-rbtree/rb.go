package rbtree

func (n workNode) case1() {
	if n.Parent == n.NIL {
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
	g := n.grandparent()
	if !uncle.isNil() && uncle.Color == RED {
		n.Parent.Color = BLACK
		uncle.Color = BLACK
		g.Color = RED
		g.case1()
		return
	}
	n.case4()
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
