package rbtree

func (n *Node) Insert(value ValueObject) {
	child := n.selectChild(value)
	// if value is self
	if child == nil {
		return
	}

	// if child hasn't created
	if *child == nil {
		// if node not exist, create a new node
		*child = &Node{
			Parent:      n,
			Color: RED,
			ValueObject: value,
			Root: n.Root,
		}
		(*child).case1()
	} else {
		(*child).Insert(value)
	}
}

func (n *Node) selectChild(value ValueObject) **Node {
	if value.Value() > n.Value() {
		return &n.Right
	} else if value.Value() < n.Value() {
		return &n.Left
	} else {
		return nil
	}
}

func (n *Node) Search(key uint64) ValueObject {
	if n == nil {
		return nil
	}
	if key < n.Value() {
		return n.Left.Search(key)
	} else if key > n.Value() {
		return n.Right.Search(key)
	}
	return n.ValueObject
}
