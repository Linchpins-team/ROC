package rbtree

type workNode struct {
	NIL *Node
	*Node
}

func (n workNode) parent() workNode {
	return workNode{
		NIL:  n.NIL,
		Node: n.Parent,
	}
}

func (n workNode) grandparent() workNode {
	if n.Parent == nil {
		return workNode{
			NIL:  n.NIL,
			Node: nil,
		}
	}
	return workNode{
		NIL:  n.NIL,
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

func (n workNode) left() workNode {
	return workNode{
		NIL:  n.NIL,
		Node: n.Left,
	}
}

func (n workNode) right() workNode {
	return workNode{
		NIL:  n.NIL,
		Node: n.Right,
	}
}
