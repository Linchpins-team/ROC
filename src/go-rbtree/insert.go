package rbtree

func (n workNode) Insert(value ValueObject) {
	child := n.selectChild(value)
	// if value is self
	if child == nil {
		return
	}
	// if child hasn't created
	if *child == nil {
		// if node not exist, create a new node
		*child = &Node{
			Parent:      n.Node,
			Color:       RED,
			ValueObject: value,
		}
		w := (*child).Work(n.NIL)
		w.case1()
	} else {
		(*child).Work(n.NIL).Insert(value)
	}
}

func (n workNode) selectChild(value ValueObject) **Node {
	// if n is root's parent, always select left
	if n.Node == n.NIL {
		return &n.Left
	}
	if value.Value() > n.Value() {
		return &n.Right
	}
	if value.Value() < n.Value() {
		return &n.Left
	}
	return nil
}

func (n workNode) Search(key uint64) ValueObject {
	if n.Node == nil {
		return nil
	}
	if key < n.Value() {
		return n.left().Search(key)
	}
	if key > n.Value() {
		return n.right().Search(key)
	}
	return n.ValueObject
}
