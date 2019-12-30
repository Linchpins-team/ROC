package rbtree

func (tree *Tree) Insert(value ValueObject) {
	var node **Tree
	if value.Value() > tree.Value() {
		node = &tree.Right
	} else if value.Value() < tree.Value() {
		node = &tree.Left
	} else {
		return
	}
	if *node == nil {
		// if node not exist, create a new tree
		*node = &Tree{
			ValueObject: value,
		}
	} else {
		(*node).Insert(value)
	}
}

func (tree *Tree) Search(key uint64) ValueObject {
	if tree == nil {
		return nil
	}
	if key < tree.Value() {
		return tree.Left.Search(key)
	} else if key > tree.Value() {
		return tree.Right.Search(key)
	}
	return tree.ValueObject
}
