package rbtree

func (n workNode) rotateLeft() {
	p := n.Parent
	g := p.Parent
	y := n.Left

	n.Parent = g
	g.chChild(p, n.Node)

	p.Parent = n.Node
	p.Right = y
	if y != nil {
		y.Parent = p
	}
	n.Left = p
}

func (n workNode) rotateRight() {
	p := n.Parent
	g := p.Parent
	y := n.Right

	n.Parent = g
	g.chChild(p, n.Node)

	p.Parent = n.Node
	p.Left = y
	if y != nil {
		y.Parent = p
	}
	n.Right = p
}

func (n *Node) chChild(a, b *Node) {
	if n.Left == a {
		n.Left = b
	} else if n.Right == a {
		n.Right = b
	}
}
