package rbtree

func work(n, NIL *Node) workNode {
	return workNode{
		NIL:  NIL,
		Node: n,
	}
}

func (n workNode) parent() workNode {
	return workNode{
		NIL:  n.NIL,
		Node: n.Parent,
	}
}

func (n workNode) grandparent() workNode {
	if n.Parent == nil {
		work(nil, n.NIL)
	}
	return work(n.Parent.Parent, n.NIL)
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

func (n workNode) left() workNode {
	return work(n.Left, n.NIL)
}

func (n workNode) right() workNode {
	return work(n.Right, n.NIL)
}

func (n workNode) isNil() bool {
	return n.Node == n.NIL
}
