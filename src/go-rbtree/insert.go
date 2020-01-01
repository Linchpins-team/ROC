package rbtree

func (n *Node) Insert(value ValueObject, tree *Tree) {
	child := n.selectChild(value, tree)
	// if value is self
	if child == nil {
		return
	}
	// if child hasn't created
	if *child == tree.NIL {
		// if node not exist, create a new node
		*child = &Node{
			Parent:      n,
			Left:        tree.NIL,
			Right:       tree.NIL,
			Color:       RED,
			ValueObject: value,
		}
		w := (*child).Work(tree.NIL)
		w.case1()
	} else {
		(*child).Insert(value, tree)
	}
}

func (n *Node) selectChild(value ValueObject, tree *Tree) **Node {
	// if n is root's parent
	if n == tree.NIL {
		return &n.Left
	}
	if value.Value() > n.Value() {
		return &n.Right
	} else if value.Value() < n.Value() {
		return &n.Left
	} else {
		return nil
	}
}

func (n *Node) Search(key uint64, tree *Tree) ValueObject {
	if n == tree.NIL {
		return nil
	}
	if key < n.Value() {
		return n.Left.Search(key, tree)
	}
	if key > n.Value() {
		return n.Right.Search(key, tree)
	}
	return n.ValueObject
}
