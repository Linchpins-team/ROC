package rbtree

type Tree struct {
	Parent *Tree
	Left   *Tree
	Right  *Tree
	ValueObject
}

type ValueObject interface {
	Value() uint64
}

func New(value ValueObject) *Tree {
	return &Tree{
		ValueObject: value,
	}
}

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
			Parent:      tree,
			ValueObject: value,
		}
	} else {
		(*node).Insert(value)
	}
}

func (tree *Tree) Delete(value ValueObject) {
	if (tree == nil) {
		return
	}
	if value.Value() > tree.Value() {
		tree.Right.Delete(value)
	} else if value.Value() < tree.Value() {
		tree.Left.Delete(value)
	} else {
		tree.delete()
	}
}

// delete this tree
func (tree *Tree) delete() {
	var pNode **Tree = tree.getParentNode()
	if pNode == nil {
		return
	}
	if tree.Right == nil && tree.Left == nil {
		// Set parent's left or right node to nil
		*pNode = nil
	} else if tree.Right != nil && tree.Left != nil {
		// Set left node to this position
		tree.Left.Parent = tree.Parent
		*pNode = tree.Left
		tree.Right.Parent = tree.Left
		rightMost := tree.Left.getRightMost()
		*rightMost = tree.Right
	} else if tree.Right != nil && tree.Left == nil {
		tree.Right.Parent = tree.Parent
		*pNode = tree.Right
	} else if tree.Right == nil && tree.Left != nil {
		tree.Left.Parent = tree.Parent
		*pNode = tree.Left
	}
	// C need to release memory
}

// get this tree in parent's node
func (tree *Tree) getParentNode() **Tree {
	if tree.Parent == nil {
		return nil
	}
	if tree.Parent.Left == tree {
		return &tree.Parent.Left
	}
	return &tree.Parent.Right
}

func (tree *Tree) getRightMost() **Tree {
	if tree.Right == nil {
		return &tree.Right
	} else {
		return tree.Right.getParentNode()
	}
}

func (tree *Tree) Search(key uint64) ValueObject {
	if key < tree.Value() {
		return tree.Left.Search(key)
	} else if key > tree.Value() {
		return tree.Right.Search(key)
	}
	return tree.ValueObject
}
